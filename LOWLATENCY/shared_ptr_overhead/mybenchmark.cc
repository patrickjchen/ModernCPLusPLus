#include <benchmark/benchmark.h>
#include <memory>
#include <vector>
#include <thread>

// ============================================================================
// std::shared_ptr Overhead in Hot Paths
//
// shared_ptr costs:
// 1. Atomic reference counting (expensive on multi-core systems)
// 2. Two allocations: object + control block
// 3. Two pointers: object pointer + control block pointer
// 4. Cache pollution from control block access
// 5. False sharing when multiple threads access different shared_ptrs
//
// Alternatives:
// - unique_ptr: Zero overhead, move-only
// - Raw pointers: For non-owning references
// - Value types: Best performance when possible
// ============================================================================

struct Data {
    long values[8];  // 64 bytes

    Data() {
        for (int i = 0; i < 8; ++i) {
            values[i] = i;
        }
    }

    long sum() const {
        long result = 0;
        for (int i = 0; i < 8; ++i) {
            result += values[i];
        }
        return result;
    }
};

// ============================================================================
// Benchmark: Creation and destruction
// ============================================================================

static void BM_SharedPtr_Create(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<std::shared_ptr<Data>> ptrs;
        for (int i = 0; i < state.range(0); ++i) {
            ptrs.push_back(std::make_shared<Data>());
        }
        benchmark::DoNotOptimize(ptrs);
    }
}
BENCHMARK(BM_SharedPtr_Create)->Arg(100)->Arg(1000)->Arg(10000);

static void BM_UniquePtr_Create(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<std::unique_ptr<Data>> ptrs;
        for (int i = 0; i < state.range(0); ++i) {
            ptrs.push_back(std::make_unique<Data>());
        }
        benchmark::DoNotOptimize(ptrs);
    }
}
BENCHMARK(BM_UniquePtr_Create)->Arg(100)->Arg(1000)->Arg(10000);

static void BM_RawPtr_Create(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<Data*> ptrs;
        for (int i = 0; i < state.range(0); ++i) {
            ptrs.push_back(new Data());
        }
        for (auto ptr : ptrs) {
            delete ptr;
        }
        benchmark::DoNotOptimize(ptrs);
    }
}
BENCHMARK(BM_RawPtr_Create)->Arg(100)->Arg(1000)->Arg(10000);

static void BM_Value_Create(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<Data> values;
        for (int i = 0; i < state.range(0); ++i) {
            values.push_back(Data());
        }
        benchmark::DoNotOptimize(values);
    }
}
BENCHMARK(BM_Value_Create)->Arg(100)->Arg(1000)->Arg(10000);

// ============================================================================
// Benchmark: Copying (reference counting overhead)
// ============================================================================

static void BM_SharedPtr_Copy(benchmark::State& state) {
    auto original = std::make_shared<Data>();

    for (auto _ : state) {
        std::vector<std::shared_ptr<Data>> copies;
        for (int i = 0; i < state.range(0); ++i) {
            copies.push_back(original);  // Atomic increment on each copy
        }
        benchmark::DoNotOptimize(copies);
    }
    // Atomic decrements happen here when copies go out of scope
}
BENCHMARK(BM_SharedPtr_Copy)->Arg(100)->Arg(1000)->Arg(10000);

static void BM_RawPtr_Copy(benchmark::State& state) {
    Data* original = new Data();

    for (auto _ : state) {
        std::vector<Data*> copies;
        for (int i = 0; i < state.range(0); ++i) {
            copies.push_back(original);  // Just pointer copy, no atomic ops
        }
        benchmark::DoNotOptimize(copies);
    }

    delete original;
}
BENCHMARK(BM_RawPtr_Copy)->Arg(100)->Arg(1000)->Arg(10000);

// ============================================================================
// Benchmark: Access patterns
// ============================================================================

static void BM_SharedPtr_Access(benchmark::State& state) {
    std::vector<std::shared_ptr<Data>> ptrs;
    for (int i = 0; i < state.range(0); ++i) {
        ptrs.push_back(std::make_shared<Data>());
    }

    for (auto _ : state) {
        long sum = 0;
        for (const auto& ptr : ptrs) {
            sum += ptr->sum();
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_SharedPtr_Access)->Arg(100)->Arg(1000)->Arg(10000);

static void BM_UniquePtr_Access(benchmark::State& state) {
    std::vector<std::unique_ptr<Data>> ptrs;
    for (int i = 0; i < state.range(0); ++i) {
        ptrs.push_back(std::make_unique<Data>());
    }

    for (auto _ : state) {
        long sum = 0;
        for (const auto& ptr : ptrs) {
            sum += ptr->sum();
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_UniquePtr_Access)->Arg(100)->Arg(1000)->Arg(10000);

static void BM_RawPtr_Access(benchmark::State& state) {
    std::vector<Data*> ptrs;
    for (int i = 0; i < state.range(0); ++i) {
        ptrs.push_back(new Data());
    }

    for (auto _ : state) {
        long sum = 0;
        for (const auto& ptr : ptrs) {
            sum += ptr->sum();
        }
        benchmark::DoNotOptimize(sum);
    }

    for (auto ptr : ptrs) {
        delete ptr;
    }
}
BENCHMARK(BM_RawPtr_Access)->Arg(100)->Arg(1000)->Arg(10000);

static void BM_Value_Access(benchmark::State& state) {
    std::vector<Data> values;
    for (int i = 0; i < state.range(0); ++i) {
        values.push_back(Data());
    }

    for (auto _ : state) {
        long sum = 0;
        for (const auto& value : values) {
            sum += value.sum();
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_Value_Access)->Arg(100)->Arg(1000)->Arg(10000);

// ============================================================================
// Benchmark: Multi-threaded reference counting (worst case for shared_ptr)
// ============================================================================

static void BM_SharedPtr_MultiThread(benchmark::State& state) {
    auto shared = std::make_shared<Data>();

    for (auto _ : state) {
        std::vector<std::thread> threads;

        for (int t = 0; t < 4; ++t) {
            threads.emplace_back([&shared, &state]() {
                for (int i = 0; i < state.range(0) / 4; ++i) {
                    auto copy = shared;  // Atomic increment
                    benchmark::DoNotOptimize(copy->sum());
                    // Atomic decrement when copy goes out of scope
                }
            });
        }

        for (auto& thread : threads) {
            thread.join();
        }
    }
}
BENCHMARK(BM_SharedPtr_MultiThread)->Arg(1000)->Arg(10000)->Arg(100000)->UseRealTime();

static void BM_RawPtr_MultiThread(benchmark::State& state) {
    Data* raw = new Data();

    for (auto _ : state) {
        std::vector<std::thread> threads;

        for (int t = 0; t < 4; ++t) {
            threads.emplace_back([raw, &state]() {
                for (int i = 0; i < state.range(0) / 4; ++i) {
                    Data* copy = raw;  // No atomic ops
                    benchmark::DoNotOptimize(copy->sum());
                }
            });
        }

        for (auto& thread : threads) {
            thread.join();
        }
    }

    delete raw;
}
BENCHMARK(BM_RawPtr_MultiThread)->Arg(1000)->Arg(10000)->Arg(100000)->UseRealTime();

// ============================================================================
// Benchmark: Passing to functions
// ============================================================================

long process_shared(std::shared_ptr<Data> ptr) {  // Copy = atomic ops
    return ptr->sum();
}

long process_shared_ref(const std::shared_ptr<Data>& ptr) {  // No copy
    return ptr->sum();
}

long process_raw(Data* ptr) {  // No overhead
    return ptr->sum();
}

long process_ref(const Data& data) {  // Best performance
    return data.sum();
}

static void BM_SharedPtr_PassByValue(benchmark::State& state) {
    auto ptr = std::make_shared<Data>();

    for (auto _ : state) {
        long sum = 0;
        for (int i = 0; i < state.range(0); ++i) {
            sum += process_shared(ptr);  // Atomic inc/dec each call
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_SharedPtr_PassByValue)->Arg(1000)->Arg(10000)->Arg(100000);

static void BM_SharedPtr_PassByRef(benchmark::State& state) {
    auto ptr = std::make_shared<Data>();

    for (auto _ : state) {
        long sum = 0;
        for (int i = 0; i < state.range(0); ++i) {
            sum += process_shared_ref(ptr);  // No atomic ops
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_SharedPtr_PassByRef)->Arg(1000)->Arg(10000)->Arg(100000);

static void BM_RawPtr_Pass(benchmark::State& state) {
    Data* ptr = new Data();

    for (auto _ : state) {
        long sum = 0;
        for (int i = 0; i < state.range(0); ++i) {
            sum += process_raw(ptr);
        }
        benchmark::DoNotOptimize(sum);
    }

    delete ptr;
}
BENCHMARK(BM_RawPtr_Pass)->Arg(1000)->Arg(10000)->Arg(100000);

static void BM_Reference_Pass(benchmark::State& state) {
    Data data;

    for (auto _ : state) {
        long sum = 0;
        for (int i = 0; i < state.range(0); ++i) {
            sum += process_ref(data);
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_Reference_Pass)->Arg(1000)->Arg(10000)->Arg(100000);

BENCHMARK_MAIN();
