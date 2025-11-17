#include <benchmark/benchmark.h>
#include <functional>
#include <vector>
#include <algorithm>
#include <numeric>

// ============================================================================
// std::function vs Templates for callbacks
//
// std::function overhead:
// - Type erasure wrapper
// - Indirect function call (cannot be inlined)
// - Potential heap allocation for large captures
// - Virtual-like dispatch overhead
//
// Template approach:
// - Zero overhead abstraction
// - Can be fully inlined
// - No indirection
// - Compile-time polymorphism
// ============================================================================

// ============================================================================
// Approach 1: Using std::function (FLEXIBLE but SLOWER)
// ============================================================================

void process_with_function(const std::vector<int>& data,
                           std::function<int(int)> transform,
                           std::function<bool(int)> predicate) {
    int sum = 0;
    for (int value : data) {
        int transformed = transform(value);
        if (predicate(transformed)) {
            sum += transformed;
        }
    }
    benchmark::DoNotOptimize(sum);
}

// ============================================================================
// Approach 2: Using Templates (FAST, fully inlinable)
// ============================================================================

template<typename TransformFunc, typename PredicateFunc>
void process_with_template(const std::vector<int>& data,
                           TransformFunc transform,
                           PredicateFunc predicate) {
    int sum = 0;
    for (int value : data) {
        int transformed = transform(value);
        if (predicate(transformed)) {
            sum += transformed;
        }
    }
    benchmark::DoNotOptimize(sum);
}

// ============================================================================
// Approach 3: Using function pointers (Middle ground)
// ============================================================================

void process_with_function_ptr(const std::vector<int>& data,
                               int (*transform)(int),
                               bool (*predicate)(int)) {
    int sum = 0;
    for (int value : data) {
        int transformed = transform(value);
        if (predicate(transformed)) {
            sum += transformed;
        }
    }
    benchmark::DoNotOptimize(sum);
}

// ============================================================================
// Simple callback functions
// ============================================================================

int double_value(int x) { return x * 2; }
bool is_even(int x) { return x % 2 == 0; }

int triple_value(int x) { return x * 3; }
bool is_positive(int x) { return x > 0; }

// ============================================================================
// Benchmarks - Simple callbacks
// ============================================================================

static void BM_StdFunction_Simple(benchmark::State& state) {
    std::vector<int> data(state.range(0));
    std::iota(data.begin(), data.end(), 1);

    std::function<int(int)> transform = double_value;
    std::function<bool(int)> predicate = is_even;

    for (auto _ : state) {
        process_with_function(data, transform, predicate);
    }
}
BENCHMARK(BM_StdFunction_Simple)->Arg(1000)->Arg(10000)->Arg(100000);

static void BM_Template_Simple(benchmark::State& state) {
    std::vector<int> data(state.range(0));
    std::iota(data.begin(), data.end(), 1);

    for (auto _ : state) {
        process_with_template(data, double_value, is_even);
    }
}
BENCHMARK(BM_Template_Simple)->Arg(1000)->Arg(10000)->Arg(100000);

static void BM_FunctionPtr_Simple(benchmark::State& state) {
    std::vector<int> data(state.range(0));
    std::iota(data.begin(), data.end(), 1);

    for (auto _ : state) {
        process_with_function_ptr(data, double_value, is_even);
    }
}
BENCHMARK(BM_FunctionPtr_Simple)->Arg(1000)->Arg(10000)->Arg(100000);

// ============================================================================
// Benchmarks - Lambda callbacks
// ============================================================================

static void BM_StdFunction_Lambda(benchmark::State& state) {
    std::vector<int> data(state.range(0));
    std::iota(data.begin(), data.end(), 1);

    int multiplier = 2;
    std::function<int(int)> transform = [multiplier](int x) { return x * multiplier; };
    std::function<bool(int)> predicate = [](int x) { return x % 2 == 0; };

    for (auto _ : state) {
        process_with_function(data, transform, predicate);
    }
}
BENCHMARK(BM_StdFunction_Lambda)->Arg(1000)->Arg(10000)->Arg(100000);

static void BM_Template_Lambda(benchmark::State& state) {
    std::vector<int> data(state.range(0));
    std::iota(data.begin(), data.end(), 1);

    int multiplier = 2;
    auto transform = [multiplier](int x) { return x * multiplier; };
    auto predicate = [](int x) { return x % 2 == 0; };

    for (auto _ : state) {
        process_with_template(data, transform, predicate);
    }
}
BENCHMARK(BM_Template_Lambda)->Arg(1000)->Arg(10000)->Arg(100000);

// ============================================================================
// Real-world scenario: Sorting with custom comparator
// ============================================================================

static void BM_StdFunction_Sort(benchmark::State& state) {
    std::vector<int> original(state.range(0));
    std::iota(original.begin(), original.end(), 1);
    std::reverse(original.begin(), original.end());

    std::function<bool(int, int)> comparator = [](int a, int b) { return a < b; };

    for (auto _ : state) {
        auto data = original;
        std::sort(data.begin(), data.end(),
                 [&comparator](int a, int b) { return comparator(a, b); });
        benchmark::DoNotOptimize(data);
    }
}
BENCHMARK(BM_StdFunction_Sort)->Arg(1000)->Arg(10000)->Arg(100000);

static void BM_Template_Sort(benchmark::State& state) {
    std::vector<int> original(state.range(0));
    std::iota(original.begin(), original.end(), 1);
    std::reverse(original.begin(), original.end());

    auto comparator = [](int a, int b) { return a < b; };

    for (auto _ : state) {
        auto data = original;
        std::sort(data.begin(), data.end(), comparator);
        benchmark::DoNotOptimize(data);
    }
}
BENCHMARK(BM_Template_Sort)->Arg(1000)->Arg(10000)->Arg(100000);

// ============================================================================
// Hot path scenario: Order book processing with price filter
// ============================================================================

struct Order {
    int price;
    int quantity;
};

void process_orders_function(const std::vector<Order>& orders,
                             std::function<bool(const Order&)> filter) {
    int total_quantity = 0;
    for (const auto& order : orders) {
        if (filter(order)) {
            total_quantity += order.quantity;
        }
    }
    benchmark::DoNotOptimize(total_quantity);
}

template<typename FilterFunc>
void process_orders_template(const std::vector<Order>& orders,
                            FilterFunc filter) {
    int total_quantity = 0;
    for (const auto& order : orders) {
        if (filter(order)) {
            total_quantity += order.quantity;
        }
    }
    benchmark::DoNotOptimize(total_quantity);
}

static void BM_StdFunction_OrderBook(benchmark::State& state) {
    std::vector<Order> orders;
    for (int i = 0; i < state.range(0); ++i) {
        orders.push_back({100 + (i % 50), 10 + (i % 20)});
    }

    int min_price = 110;
    std::function<bool(const Order&)> filter = [min_price](const Order& o) {
        return o.price >= min_price && o.quantity > 15;
    };

    for (auto _ : state) {
        process_orders_function(orders, filter);
    }
}
BENCHMARK(BM_StdFunction_OrderBook)->Arg(1000)->Arg(10000)->Arg(100000);

static void BM_Template_OrderBook(benchmark::State& state) {
    std::vector<Order> orders;
    for (int i = 0; i < state.range(0); ++i) {
        orders.push_back({100 + (i % 50), 10 + (i % 20)});
    }

    int min_price = 110;
    auto filter = [min_price](const Order& o) {
        return o.price >= min_price && o.quantity > 15;
    };

    for (auto _ : state) {
        process_orders_template(orders, filter);
    }
}
BENCHMARK(BM_Template_OrderBook)->Arg(1000)->Arg(10000)->Arg(100000);

BENCHMARK_MAIN();
