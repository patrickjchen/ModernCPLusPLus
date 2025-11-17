#include <benchmark/benchmark.h>
#include <vector>
#include <random>
#include <algorithm>

// ============================================================================
// Linked List Node for pointer chasing
// ============================================================================

struct Node {
    int data;
    Node* next;
};

// ============================================================================
// Helper function to create a shuffled linked list
// ============================================================================

std::vector<Node> createShuffledList(size_t size) {
    std::vector<Node> nodes(size);
    std::vector<size_t> indices(size);

    // Initialize indices
    for (size_t i = 0; i < size; ++i) {
        indices[i] = i;
        nodes[i].data = i;
    }

    // Shuffle indices to create random access pattern
    std::random_device rd;
    std::mt19937 gen(42); // Fixed seed for reproducibility
    std::shuffle(indices.begin(), indices.end(), gen);

    // Link nodes in shuffled order
    for (size_t i = 0; i < size - 1; ++i) {
        nodes[indices[i]].next = &nodes[indices[i + 1]];
    }
    nodes[indices[size - 1]].next = nullptr;

    return nodes;
}

// ============================================================================
// Benchmark: Traverse linked list WITHOUT prefetching
// ============================================================================

static void BM_WithoutPrefetching(benchmark::State& state) {
    auto nodes = createShuffledList(state.range(0));
    Node* head = &nodes[0];

    for (auto _ : state) {
        long sum = 0;
        Node* current = head;

        while (current != nullptr) {
            sum += current->data;
            current = current->next;
        }

        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_WithoutPrefetching)->Arg(1000)->Arg(10000)->Arg(100000);

// ============================================================================
// Benchmark: Traverse linked list WITH prefetching
// ============================================================================

static void BM_WithPrefetching(benchmark::State& state) {
    auto nodes = createShuffledList(state.range(0));
    Node* head = &nodes[0];

    for (auto _ : state) {
        long sum = 0;
        Node* current = head;

        while (current != nullptr) {
            // Prefetch the next node while processing current
            if (current->next != nullptr) {
                __builtin_prefetch(current->next, 0, 3);
                // Parameters: (address, rw, locality)
                // rw: 0 = read, 1 = write
                // locality: 0-3, where 3 = high temporal locality
            }

            sum += current->data;
            current = current->next;
        }

        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_WithPrefetching)->Arg(1000)->Arg(10000)->Arg(100000);

// ============================================================================
// Array-based benchmark with stride access
// ============================================================================

struct DataBlock {
    long data[8];  // 64 bytes
};

static void BM_ArrayWithoutPrefetch(benchmark::State& state) {
    std::vector<DataBlock> blocks(state.range(0));
    std::vector<size_t> indices(state.range(0));

    // Initialize with random access pattern
    for (size_t i = 0; i < indices.size(); ++i) {
        indices[i] = i;
        for (int j = 0; j < 8; ++j) {
            blocks[i].data[j] = i * 8 + j;
        }
    }

    std::random_device rd;
    std::mt19937 gen(42);
    std::shuffle(indices.begin(), indices.end(), gen);

    for (auto _ : state) {
        long sum = 0;
        for (size_t i = 0; i < indices.size(); ++i) {
            const auto& block = blocks[indices[i]];
            for (int j = 0; j < 8; ++j) {
                sum += block.data[j];
            }
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_ArrayWithoutPrefetch)->Arg(1000)->Arg(10000)->Arg(50000);

static void BM_ArrayWithPrefetch(benchmark::State& state) {
    std::vector<DataBlock> blocks(state.range(0));
    std::vector<size_t> indices(state.range(0));

    // Initialize with random access pattern
    for (size_t i = 0; i < indices.size(); ++i) {
        indices[i] = i;
        for (int j = 0; j < 8; ++j) {
            blocks[i].data[j] = i * 8 + j;
        }
    }

    std::random_device rd;
    std::mt19937 gen(42);
    std::shuffle(indices.begin(), indices.end(), gen);

    for (auto _ : state) {
        long sum = 0;
        for (size_t i = 0; i < indices.size(); ++i) {
            // Prefetch next block while processing current
            if (i + 1 < indices.size()) {
                __builtin_prefetch(&blocks[indices[i + 1]], 0, 3);
            }

            const auto& block = blocks[indices[i]];
            for (int j = 0; j < 8; ++j) {
                sum += block.data[j];
            }
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_ArrayWithPrefetch)->Arg(1000)->Arg(10000)->Arg(50000);

BENCHMARK_MAIN();
