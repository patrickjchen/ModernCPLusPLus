#include <benchmark/benchmark.h>
#include <vector>
#include <numeric>
#include <cstring>

// ============================================================================
// SIMD (Single Instruction, Multiple Data)
//
// Process multiple data elements with a single instruction
// Architecture-specific:
// - x86/x64: SSE (128-bit), AVX (256-bit), AVX-512 (512-bit)
// - ARM: NEON (128-bit)
//
// This benchmark focuses on x86/x64 SIMD
// ============================================================================

// Platform detection
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386) || defined(_M_IX86)
    #define HAS_X86_SIMD 1
    #ifdef __AVX2__
        #include <immintrin.h>
        #define HAS_AVX2 1
    #elif __AVX__
        #include <immintrin.h>
        #define HAS_AVX 1
    #elif __SSE4_2__
        #include <nmmintrin.h>
        #define HAS_SSE42 1
    #elif __SSE2__
        #include <emmintrin.h>
        #define HAS_SSE2 1
    #endif
#elif defined(__ARM_NEON) || defined(__aarch64__)
    #include <arm_neon.h>
    #define HAS_NEON 1
#endif

// ============================================================================
// Example 1: Vector Addition
// ============================================================================

// Scalar version
void vector_add_scalar(const float* a, const float* b, float* result, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        result[i] = a[i] + b[i];
    }
}

// SIMD version
void vector_add_simd(const float* a, const float* b, float* result, size_t size) {
#ifdef HAS_AVX
    size_t simd_size = size - (size % 8);

    for (size_t i = 0; i < simd_size; i += 8) {
        __m256 va = _mm256_loadu_ps(&a[i]);
        __m256 vb = _mm256_loadu_ps(&b[i]);
        __m256 vr = _mm256_add_ps(va, vb);
        _mm256_storeu_ps(&result[i], vr);
    }

    // Handle remainder
    for (size_t i = simd_size; i < size; ++i) {
        result[i] = a[i] + b[i];
    }
#elif defined(HAS_SSE2)
    size_t simd_size = size - (size % 4);

    for (size_t i = 0; i < simd_size; i += 4) {
        __m128 va = _mm_loadu_ps(&a[i]);
        __m128 vb = _mm_loadu_ps(&b[i]);
        __m128 vr = _mm_add_ps(va, vb);
        _mm_storeu_ps(&result[i], vr);
    }

    for (size_t i = simd_size; i < size; ++i) {
        result[i] = a[i] + b[i];
    }
#elif defined(HAS_NEON)
    size_t simd_size = size - (size % 4);

    for (size_t i = 0; i < simd_size; i += 4) {
        float32x4_t va = vld1q_f32(&a[i]);
        float32x4_t vb = vld1q_f32(&b[i]);
        float32x4_t vr = vaddq_f32(va, vb);
        vst1q_f32(&result[i], vr);
    }

    for (size_t i = simd_size; i < size; ++i) {
        result[i] = a[i] + b[i];
    }
#else
    vector_add_scalar(a, b, result, size);
#endif
}

static void BM_VectorAdd_Scalar(benchmark::State& state) {
    std::vector<float> a(state.range(0), 1.0f);
    std::vector<float> b(state.range(0), 2.0f);
    std::vector<float> result(state.range(0));

    for (auto _ : state) {
        vector_add_scalar(a.data(), b.data(), result.data(), a.size());
        benchmark::DoNotOptimize(result);
    }

    state.SetBytesProcessed(state.iterations() * state.range(0) * sizeof(float) * 3);
}
BENCHMARK(BM_VectorAdd_Scalar)->Arg(1000)->Arg(10000)->Arg(100000);

static void BM_VectorAdd_SIMD(benchmark::State& state) {
    std::vector<float> a(state.range(0), 1.0f);
    std::vector<float> b(state.range(0), 2.0f);
    std::vector<float> result(state.range(0));

    for (auto _ : state) {
        vector_add_simd(a.data(), b.data(), result.data(), a.size());
        benchmark::DoNotOptimize(result);
    }

    state.SetBytesProcessed(state.iterations() * state.range(0) * sizeof(float) * 3);
}
BENCHMARK(BM_VectorAdd_SIMD)->Arg(1000)->Arg(10000)->Arg(100000);

// ============================================================================
// Example 2: Dot Product
// ============================================================================

// Scalar version
float dot_product_scalar(const float* a, const float* b, size_t size) {
    float sum = 0.0f;
    for (size_t i = 0; i < size; ++i) {
        sum += a[i] * b[i];
    }
    return sum;
}

// SIMD version
float dot_product_simd(const float* a, const float* b, size_t size) {
#ifdef HAS_AVX
    __m256 sum_vec = _mm256_setzero_ps();
    size_t simd_size = size - (size % 8);

    for (size_t i = 0; i < simd_size; i += 8) {
        __m256 va = _mm256_loadu_ps(&a[i]);
        __m256 vb = _mm256_loadu_ps(&b[i]);
        sum_vec = _mm256_add_ps(sum_vec, _mm256_mul_ps(va, vb));
    }

    // Horizontal sum
    __m128 sum_high = _mm256_extractf128_ps(sum_vec, 1);
    __m128 sum_low = _mm256_castps256_ps128(sum_vec);
    __m128 sum128 = _mm_add_ps(sum_low, sum_high);
    __m128 shuf = _mm_shuffle_ps(sum128, sum128, _MM_SHUFFLE(2, 3, 0, 1));
    __m128 sums = _mm_add_ps(sum128, shuf);
    shuf = _mm_movehl_ps(shuf, sums);
    sums = _mm_add_ss(sums, shuf);
    float sum = _mm_cvtss_f32(sums);

    // Handle remainder
    for (size_t i = simd_size; i < size; ++i) {
        sum += a[i] * b[i];
    }

    return sum;
#elif defined(HAS_SSE2)
    __m128 sum_vec = _mm_setzero_ps();
    size_t simd_size = size - (size % 4);

    for (size_t i = 0; i < simd_size; i += 4) {
        __m128 va = _mm_loadu_ps(&a[i]);
        __m128 vb = _mm_loadu_ps(&b[i]);
        sum_vec = _mm_add_ps(sum_vec, _mm_mul_ps(va, vb));
    }

    // Horizontal sum
    __m128 shuf = _mm_shuffle_ps(sum_vec, sum_vec, _MM_SHUFFLE(2, 3, 0, 1));
    __m128 sums = _mm_add_ps(sum_vec, shuf);
    shuf = _mm_movehl_ps(shuf, sums);
    sums = _mm_add_ss(sums, shuf);
    float sum = _mm_cvtss_f32(sums);

    for (size_t i = simd_size; i < size; ++i) {
        sum += a[i] * b[i];
    }

    return sum;
#else
    return dot_product_scalar(a, b, size);
#endif
}

static void BM_DotProduct_Scalar(benchmark::State& state) {
    std::vector<float> a(state.range(0), 1.0f);
    std::vector<float> b(state.range(0), 2.0f);

    for (auto _ : state) {
        float result = dot_product_scalar(a.data(), b.data(), a.size());
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_DotProduct_Scalar)->Arg(1000)->Arg(10000)->Arg(100000);

static void BM_DotProduct_SIMD(benchmark::State& state) {
    std::vector<float> a(state.range(0), 1.0f);
    std::vector<float> b(state.range(0), 2.0f);

    for (auto _ : state) {
        float result = dot_product_simd(a.data(), b.data(), a.size());
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_DotProduct_SIMD)->Arg(1000)->Arg(10000)->Arg(100000);

// ============================================================================
// Example 3: Finding Maximum Value
// ============================================================================

// Scalar version
float find_max_scalar(const float* data, size_t size) {
    float max_val = data[0];
    for (size_t i = 1; i < size; ++i) {
        if (data[i] > max_val) {
            max_val = data[i];
        }
    }
    return max_val;
}

// SIMD version
float find_max_simd(const float* data, size_t size) {
#ifdef HAS_AVX
    __m256 max_vec = _mm256_loadu_ps(data);
    size_t simd_size = size - (size % 8);

    for (size_t i = 8; i < simd_size; i += 8) {
        __m256 v = _mm256_loadu_ps(&data[i]);
        max_vec = _mm256_max_ps(max_vec, v);
    }

    // Horizontal max
    __m128 max_high = _mm256_extractf128_ps(max_vec, 1);
    __m128 max_low = _mm256_castps256_ps128(max_vec);
    __m128 max128 = _mm_max_ps(max_low, max_high);
    __m128 shuf = _mm_shuffle_ps(max128, max128, _MM_SHUFFLE(2, 3, 0, 1));
    max128 = _mm_max_ps(max128, shuf);
    shuf = _mm_movehl_ps(shuf, max128);
    max128 = _mm_max_ss(max128, shuf);
    float max_val = _mm_cvtss_f32(max128);

    // Handle remainder
    for (size_t i = simd_size; i < size; ++i) {
        if (data[i] > max_val) {
            max_val = data[i];
        }
    }

    return max_val;
#elif defined(HAS_SSE2)
    __m128 max_vec = _mm_loadu_ps(data);
    size_t simd_size = size - (size % 4);

    for (size_t i = 4; i < simd_size; i += 4) {
        __m128 v = _mm_loadu_ps(&data[i]);
        max_vec = _mm_max_ps(max_vec, v);
    }

    __m128 shuf = _mm_shuffle_ps(max_vec, max_vec, _MM_SHUFFLE(2, 3, 0, 1));
    max_vec = _mm_max_ps(max_vec, shuf);
    shuf = _mm_movehl_ps(shuf, max_vec);
    max_vec = _mm_max_ss(max_vec, shuf);
    float max_val = _mm_cvtss_f32(max_vec);

    for (size_t i = simd_size; i < size; ++i) {
        if (data[i] > max_val) {
            max_val = data[i];
        }
    }

    return max_val;
#else
    return find_max_scalar(data, size);
#endif
}

static void BM_FindMax_Scalar(benchmark::State& state) {
    std::vector<float> data(state.range(0));
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<float>(i * 31337 % 10000);
    }

    for (auto _ : state) {
        float result = find_max_scalar(data.data(), data.size());
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_FindMax_Scalar)->Arg(1000)->Arg(10000)->Arg(100000);

static void BM_FindMax_SIMD(benchmark::State& state) {
    std::vector<float> data(state.range(0));
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = static_cast<float>(i * 31337 % 10000);
    }

    for (auto _ : state) {
        float result = find_max_simd(data.data(), data.size());
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_FindMax_SIMD)->Arg(1000)->Arg(10000)->Arg(100000);

// ============================================================================
// Example 4: Array Scaling (multiply all elements by constant)
// ============================================================================

void scale_scalar(float* data, size_t size, float factor) {
    for (size_t i = 0; i < size; ++i) {
        data[i] *= factor;
    }
}

void scale_simd(float* data, size_t size, float factor) {
#ifdef HAS_AVX
    __m256 factor_vec = _mm256_set1_ps(factor);
    size_t simd_size = size - (size % 8);

    for (size_t i = 0; i < simd_size; i += 8) {
        __m256 v = _mm256_loadu_ps(&data[i]);
        v = _mm256_mul_ps(v, factor_vec);
        _mm256_storeu_ps(&data[i], v);
    }

    for (size_t i = simd_size; i < size; ++i) {
        data[i] *= factor;
    }
#elif defined(HAS_SSE2)
    __m128 factor_vec = _mm_set1_ps(factor);
    size_t simd_size = size - (size % 4);

    for (size_t i = 0; i < simd_size; i += 4) {
        __m128 v = _mm_loadu_ps(&data[i]);
        v = _mm_mul_ps(v, factor_vec);
        _mm_storeu_ps(&data[i], v);
    }

    for (size_t i = simd_size; i < size; ++i) {
        data[i] *= factor;
    }
#else
    scale_scalar(data, size, factor);
#endif
}

static void BM_Scale_Scalar(benchmark::State& state) {
    std::vector<float> original(state.range(0), 1.0f);

    for (auto _ : state) {
        auto data = original;
        scale_scalar(data.data(), data.size(), 1.5f);
        benchmark::DoNotOptimize(data);
    }
}
BENCHMARK(BM_Scale_Scalar)->Arg(1000)->Arg(10000)->Arg(100000);

static void BM_Scale_SIMD(benchmark::State& state) {
    std::vector<float> original(state.range(0), 1.0f);

    for (auto _ : state) {
        auto data = original;
        scale_simd(data.data(), data.size(), 1.5f);
        benchmark::DoNotOptimize(data);
    }
}
BENCHMARK(BM_Scale_SIMD)->Arg(1000)->Arg(10000)->Arg(100000);

BENCHMARK_MAIN();
