#include <benchmark/benchmark.h>
#include <vector>
#include <algorithm>
#include <cmath>

// ============================================================================
// Branch-Free Programming
//
// Modern CPUs use branch prediction. Mispredicted branches cause pipeline
// stalls (10-20 cycles penalty). Branch-free code uses arithmetic/logical
// operations instead of conditional branches.
//
// When to use:
// - Unpredictable branches (random data)
// - Hot paths with tight loops
// - Avoid when branches are highly predictable
// ============================================================================

// ============================================================================
// Example 1: Absolute Value
// ============================================================================

// With branch (misprediction penalty for random data)
int abs_with_branch(int x) {
    if (x < 0) {
        return -x;
    }
    return x;
}

// Branch-free using bit manipulation
int abs_branch_free(int x) {
    int mask = x >> 31;  // All 1s if negative, all 0s if positive
    return (x + mask) ^ mask;  // Equivalent to (x ^ mask) - mask
}

static void BM_Abs_WithBranch(benchmark::State& state) {
    std::vector<int> data(state.range(0));
    // Random data causes branch mispredictions
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = (i * 31337) % 10000 - 5000;
    }

    for (auto _ : state) {
        int sum = 0;
        for (int val : data) {
            sum += abs_with_branch(val);
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_Abs_WithBranch)->Arg(1000)->Arg(10000)->Arg(100000);

static void BM_Abs_BranchFree(benchmark::State& state) {
    std::vector<int> data(state.range(0));
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = (i * 31337) % 10000 - 5000;
    }

    for (auto _ : state) {
        int sum = 0;
        for (int val : data) {
            sum += abs_branch_free(val);
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_Abs_BranchFree)->Arg(1000)->Arg(10000)->Arg(100000);

// ============================================================================
// Example 2: Min/Max
// ============================================================================

// With branch
int min_with_branch(int a, int b) {
    return (a < b) ? a : b;
}

// Branch-free
int min_branch_free(int a, int b) {
    return b + ((a - b) & ((a - b) >> 31));
}

// Alternative: compiler might optimize ternary to cmov (conditional move)
int min_ternary(int a, int b) {
    return (a < b) ? a : b;
}

static void BM_Min_WithBranch(benchmark::State& state) {
    std::vector<int> data1(state.range(0));
    std::vector<int> data2(state.range(0));
    for (size_t i = 0; i < data1.size(); ++i) {
        data1[i] = (i * 31337) % 10000;
        data2[i] = (i * 27183) % 10000;
    }

    for (auto _ : state) {
        int sum = 0;
        for (size_t i = 0; i < data1.size(); ++i) {
            sum += min_with_branch(data1[i], data2[i]);
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_Min_WithBranch)->Arg(1000)->Arg(10000)->Arg(100000);

static void BM_Min_BranchFree(benchmark::State& state) {
    std::vector<int> data1(state.range(0));
    std::vector<int> data2(state.range(0));
    for (size_t i = 0; i < data1.size(); ++i) {
        data1[i] = (i * 31337) % 10000;
        data2[i] = (i * 27183) % 10000;
    }

    for (auto _ : state) {
        int sum = 0;
        for (size_t i = 0; i < data1.size(); ++i) {
            sum += min_branch_free(data1[i], data2[i]);
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_Min_BranchFree)->Arg(1000)->Arg(10000)->Arg(100000);

// ============================================================================
// Example 3: Conditional Assignment
// ============================================================================

// With branch
void update_with_branch(int& value, int threshold, int new_value) {
    if (value > threshold) {
        value = new_value;
    }
}

// Branch-free using mask
void update_branch_free(int& value, int threshold, int new_value) {
    int mask = -(value > threshold);  // 0xFFFFFFFF if true, 0 if false
    value = (mask & new_value) | (~mask & value);
}

static void BM_ConditionalUpdate_WithBranch(benchmark::State& state) {
    std::vector<int> data(state.range(0));
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = (i * 31337) % 100;
    }

    for (auto _ : state) {
        auto copy = data;
        for (auto& val : copy) {
            update_with_branch(val, 50, 0);
        }
        benchmark::DoNotOptimize(copy);
    }
}
BENCHMARK(BM_ConditionalUpdate_WithBranch)->Arg(1000)->Arg(10000)->Arg(100000);

static void BM_ConditionalUpdate_BranchFree(benchmark::State& state) {
    std::vector<int> data(state.range(0));
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = (i * 31337) % 100;
    }

    for (auto _ : state) {
        auto copy = data;
        for (auto& val : copy) {
            update_branch_free(val, 50, 0);
        }
        benchmark::DoNotOptimize(copy);
    }
}
BENCHMARK(BM_ConditionalUpdate_BranchFree)->Arg(1000)->Arg(10000)->Arg(100000);

// ============================================================================
// Example 4: Clamping (min/max combined)
// ============================================================================

// With branches
int clamp_with_branch(int value, int min_val, int max_val) {
    if (value < min_val) return min_val;
    if (value > max_val) return max_val;
    return value;
}

// Branch-free
int clamp_branch_free(int value, int min_val, int max_val) {
    // First clamp to min
    int diff = min_val - value;
    value += diff & (diff >> 31);
    // Then clamp to max
    diff = value - max_val;
    value -= diff & (diff >> 31);
    return value;
}

static void BM_Clamp_WithBranch(benchmark::State& state) {
    std::vector<int> data(state.range(0));
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = (i * 31337) % 200 - 50;  // Range: -50 to 149
    }

    for (auto _ : state) {
        int sum = 0;
        for (int val : data) {
            sum += clamp_with_branch(val, 0, 100);
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_Clamp_WithBranch)->Arg(1000)->Arg(10000)->Arg(100000);

static void BM_Clamp_BranchFree(benchmark::State& state) {
    std::vector<int> data(state.range(0));
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = (i * 31337) % 200 - 50;
    }

    for (auto _ : state) {
        int sum = 0;
        for (int val : data) {
            sum += clamp_branch_free(val, 0, 100);
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_Clamp_BranchFree)->Arg(1000)->Arg(10000)->Arg(100000);

// ============================================================================
// Example 5: Sign function (-1, 0, or 1)
// ============================================================================

// With branches
int sign_with_branch(int x) {
    if (x > 0) return 1;
    if (x < 0) return -1;
    return 0;
}

// Branch-free
int sign_branch_free(int x) {
    return (x > 0) - (x < 0);  // Boolean to int conversion
}

static void BM_Sign_WithBranch(benchmark::State& state) {
    std::vector<int> data(state.range(0));
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = (i * 31337) % 10000 - 5000;
    }

    for (auto _ : state) {
        int sum = 0;
        for (int val : data) {
            sum += sign_with_branch(val);
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_Sign_WithBranch)->Arg(1000)->Arg(10000)->Arg(100000);

static void BM_Sign_BranchFree(benchmark::State& state) {
    std::vector<int> data(state.range(0));
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = (i * 31337) % 10000 - 5000;
    }

    for (auto _ : state) {
        int sum = 0;
        for (int val : data) {
            sum += sign_branch_free(val);
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_Sign_BranchFree)->Arg(1000)->Arg(10000)->Arg(100000);

// ============================================================================
// Example 6: Counting (e.g., count positive numbers)
// ============================================================================

// With branch
int count_positive_with_branch(const std::vector<int>& data) {
    int count = 0;
    for (int val : data) {
        if (val > 0) {
            count++;
        }
    }
    return count;
}

// Branch-free
int count_positive_branch_free(const std::vector<int>& data) {
    int count = 0;
    for (int val : data) {
        count += (val > 0);  // Boolean converts to 0 or 1
    }
    return count;
}

static void BM_CountPositive_WithBranch(benchmark::State& state) {
    std::vector<int> data(state.range(0));
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = (i * 31337) % 10000 - 5000;
    }

    for (auto _ : state) {
        int result = count_positive_with_branch(data);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_CountPositive_WithBranch)->Arg(1000)->Arg(10000)->Arg(100000);

static void BM_CountPositive_BranchFree(benchmark::State& state) {
    std::vector<int> data(state.range(0));
    for (size_t i = 0; i < data.size(); ++i) {
        data[i] = (i * 31337) % 10000 - 5000;
    }

    for (auto _ : state) {
        int result = count_positive_branch_free(data);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_CountPositive_BranchFree)->Arg(1000)->Arg(10000)->Arg(100000);

BENCHMARK_MAIN();
