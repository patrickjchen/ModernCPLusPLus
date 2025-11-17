#include <benchmark/benchmark.h>
#include <vector>
#include <cstdint>

// ============================================================================
// Poorly Packed Structure (with padding)
// ============================================================================

struct PoorlyPacked {
    char a;        // 1 byte
                   // 7 bytes padding
    double b;      // 8 bytes
    char c;        // 1 byte
                   // 3 bytes padding
    int d;         // 4 bytes
    char e;        // 1 byte
                   // 7 bytes padding
    long f;        // 8 bytes
    // Total: 40 bytes (with padding)
};

// ============================================================================
// Well Packed Structure (minimal padding)
// ============================================================================

struct WellPacked {
    double b;      // 8 bytes
    long f;        // 8 bytes
    int d;         // 4 bytes
    char a;        // 1 byte
    char c;        // 1 byte
    char e;        // 1 byte
                   // 1 byte padding
    // Total: 24 bytes (with minimal padding)
};

// ============================================================================
// Packed with attribute (compiler-specific, no padding)
// ============================================================================

struct __attribute__((packed)) TightlyPacked {
    char a;        // 1 byte
    double b;      // 8 bytes
    char c;        // 1 byte
    int d;         // 4 bytes
    char e;        // 1 byte
    long f;        // 8 bytes
    // Total: 23 bytes (no padding, but potentially slower access)
};

// ============================================================================
// Benchmarks
// ============================================================================

static void BM_PoorlyPackedAccess(benchmark::State& state) {
    std::vector<PoorlyPacked> data;
    for (int i = 0; i < state.range(0); ++i) {
        data.push_back({
            static_cast<char>(i),
            static_cast<double>(i) * 1.5,
            static_cast<char>(i + 1),
            i * 2,
            static_cast<char>(i + 2),
            static_cast<long>(i) * 3
        });
    }

    for (auto _ : state) {
        long sum = 0;
        for (const auto& item : data) {
            sum += item.a + static_cast<long>(item.b) + item.c + item.d + item.e + item.f;
        }
        benchmark::DoNotOptimize(sum);
    }

    state.SetBytesProcessed(state.iterations() * data.size() * sizeof(PoorlyPacked));
}
BENCHMARK(BM_PoorlyPackedAccess)->Arg(1000)->Arg(10000)->Arg(100000);

static void BM_WellPackedAccess(benchmark::State& state) {
    std::vector<WellPacked> data;
    for (int i = 0; i < state.range(0); ++i) {
        data.push_back({
            static_cast<double>(i) * 1.5,
            static_cast<long>(i) * 3,
            i * 2,
            static_cast<char>(i),
            static_cast<char>(i + 1),
            static_cast<char>(i + 2)
        });
    }

    for (auto _ : state) {
        long sum = 0;
        for (const auto& item : data) {
            sum += static_cast<long>(item.b) + item.f + item.d + item.a + item.c + item.e;
        }
        benchmark::DoNotOptimize(sum);
    }

    state.SetBytesProcessed(state.iterations() * data.size() * sizeof(WellPacked));
}
BENCHMARK(BM_WellPackedAccess)->Arg(1000)->Arg(10000)->Arg(100000);

static void BM_TightlyPackedAccess(benchmark::State& state) {
    std::vector<TightlyPacked> data;
    for (int i = 0; i < state.range(0); ++i) {
        data.push_back({
            static_cast<char>(i),
            static_cast<double>(i) * 1.5,
            static_cast<char>(i + 1),
            i * 2,
            static_cast<char>(i + 2),
            static_cast<long>(i) * 3
        });
    }

    for (auto _ : state) {
        long sum = 0;
        for (const auto& item : data) {
            sum += item.a + static_cast<long>(item.b) + item.c + item.d + item.e + item.f;
        }
        benchmark::DoNotOptimize(sum);
    }

    state.SetBytesProcessed(state.iterations() * data.size() * sizeof(TightlyPacked));
}
BENCHMARK(BM_TightlyPackedAccess)->Arg(1000)->Arg(10000)->Arg(100000);

BENCHMARK_MAIN();

// ============================================================================
// Structure sizes (compile-time info):
// PoorlyPacked:   40 bytes (with padding)
// WellPacked:     24 bytes (40% memory savings!)
// TightlyPacked:  23 bytes (42.5% memory savings, but slower unaligned access)
//
// To verify sizes at runtime, add: static_assert(sizeof(PoorlyPacked) == 40);
// ============================================================================
