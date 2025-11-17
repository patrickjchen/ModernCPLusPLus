#include <benchmark/benchmark.h>
#include <thread>
#include <vector>
#include <atomic>

// Typical cache line size on modern processors
constexpr size_t CACHE_LINE_SIZE = 64;

// ============================================================================
// Problem: False Sharing
// Multiple threads writing to different variables on the same cache line
// ============================================================================

struct CountersWithFalseSharing {
    std::atomic<long> counter1{0};
    std::atomic<long> counter2{0};
    std::atomic<long> counter3{0};
    std::atomic<long> counter4{0};
};

// ============================================================================
// Solution: Cache Line Padding
// Ensure each counter is on its own cache line
// ============================================================================

struct alignas(CACHE_LINE_SIZE) PaddedCounter {
    std::atomic<long> counter{0};
    // Padding to fill the rest of the cache line
    char padding[CACHE_LINE_SIZE - sizeof(std::atomic<long>)];
};

struct CountersWithoutFalseSharing {
    PaddedCounter counter1;
    PaddedCounter counter2;
    PaddedCounter counter3;
    PaddedCounter counter4;
};

// ============================================================================
// Benchmark: False Sharing (Poor Performance)
// ============================================================================

static void BM_WithFalseSharing(benchmark::State& state) {
    CountersWithFalseSharing counters;
    const int num_threads = state.range(0);
    const int iterations_per_thread = 100000;

    for (auto _ : state) {
        std::vector<std::thread> threads;

        // Spawn threads, each incrementing a different counter
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([&counters, i, iterations_per_thread]() {
                std::atomic<long>* counter;
                switch (i % 4) {
                    case 0: counter = &counters.counter1; break;
                    case 1: counter = &counters.counter2; break;
                    case 2: counter = &counters.counter3; break;
                    case 3: counter = &counters.counter4; break;
                }

                for (int j = 0; j < iterations_per_thread; ++j) {
                    counter->fetch_add(1, std::memory_order_relaxed);
                }
            });
        }

        for (auto& thread : threads) {
            thread.join();
        }

        benchmark::DoNotOptimize(counters);
    }
}
BENCHMARK(BM_WithFalseSharing)->Arg(2)->Arg(4)->Arg(8)->UseRealTime();

// ============================================================================
// Benchmark: No False Sharing (Good Performance)
// ============================================================================

static void BM_WithoutFalseSharing(benchmark::State& state) {
    CountersWithoutFalseSharing counters;
    const int num_threads = state.range(0);
    const int iterations_per_thread = 100000;

    for (auto _ : state) {
        std::vector<std::thread> threads;

        // Spawn threads, each incrementing a different counter
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([&counters, i, iterations_per_thread]() {
                std::atomic<long>* counter;
                switch (i % 4) {
                    case 0: counter = &counters.counter1.counter; break;
                    case 1: counter = &counters.counter2.counter; break;
                    case 2: counter = &counters.counter3.counter; break;
                    case 3: counter = &counters.counter4.counter; break;
                }

                for (int j = 0; j < iterations_per_thread; ++j) {
                    counter->fetch_add(1, std::memory_order_relaxed);
                }
            });
        }

        for (auto& thread : threads) {
            thread.join();
        }

        benchmark::DoNotOptimize(counters);
    }
}
BENCHMARK(BM_WithoutFalseSharing)->Arg(2)->Arg(4)->Arg(8)->UseRealTime();

BENCHMARK_MAIN();
