#include <benchmark/benchmark.h>

// ============================================================================
// Runtime Fibonacci Calculation
// ============================================================================

long fibonacci_runtime(int n) {
    if (n <= 1) return n;
    return fibonacci_runtime(n - 1) + fibonacci_runtime(n - 2);
}

// ============================================================================
// Compile-time Fibonacci Calculation using constexpr
// ============================================================================

constexpr long fibonacci_constexpr(int n) {
    if (n <= 1) return n;
    return fibonacci_constexpr(n - 1) + fibonacci_constexpr(n - 2);
}

// ============================================================================
// Benchmarks
// ============================================================================

// Benchmark: Runtime Fibonacci calculation
static void BM_FibonacciRuntime(benchmark::State& state) {
    for (auto _ : state) {
        long result = fibonacci_runtime(20);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_FibonacciRuntime);

// Benchmark: Compile-time Fibonacci calculation
// The value is computed at compile time and just loaded at runtime
static void BM_FibonacciConstexpr(benchmark::State& state) {
    for (auto _ : state) {
        constexpr long result = fibonacci_constexpr(20);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_FibonacciConstexpr);

// ============================================================================
// Additional benchmark with multiple calls to show the difference more clearly
// ============================================================================

static void BM_FibonacciRuntimeMultiple(benchmark::State& state) {
    for (auto _ : state) {
        long sum = 0;
        for (int i = 0; i < 100; ++i) {
            sum += fibonacci_runtime(20);
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_FibonacciRuntimeMultiple);

static void BM_FibonacciConstexprMultiple(benchmark::State& state) {
    for (auto _ : state) {
        long sum = 0;
        constexpr long fib20 = fibonacci_constexpr(20);
        for (int i = 0; i < 100; ++i) {
            sum += fib20;
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_FibonacciConstexprMultiple);

BENCHMARK_MAIN();
