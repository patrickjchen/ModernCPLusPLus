#include <benchmark/benchmark.h>
#include <vector>
#include <sys/mman.h>
#include <cstring>
#include <cstdlib>

// ============================================================================
// Huge Pages vs Regular Pages
//
// Regular pages: 4KB (4096 bytes)
// Huge pages: 2MB (2097152 bytes)
//
// Benefits:
// - Fewer TLB (Translation Lookaside Buffer) misses
// - Better performance for large memory allocations
// - Reduced page table overhead
// ============================================================================

constexpr size_t HUGE_PAGE_SIZE = 2 * 1024 * 1024; // 2MB
constexpr size_t REGULAR_PAGE_SIZE = 4 * 1024;     // 4KB

// ============================================================================
// Helper: Allocate memory with regular pages
// ============================================================================

void* allocate_regular_pages(size_t size) {
    void* ptr = mmap(nullptr, size,
                     PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS,
                     -1, 0);

    if (ptr == MAP_FAILED) {
        return nullptr;
    }

    // Touch all pages to ensure they're allocated
    memset(ptr, 0, size);

    return ptr;
}

// ============================================================================
// Helper: Allocate memory with huge pages hint
// ============================================================================

void* allocate_huge_pages(size_t size) {
    // Align size to huge page boundary
    size_t aligned_size = ((size + HUGE_PAGE_SIZE - 1) / HUGE_PAGE_SIZE) * HUGE_PAGE_SIZE;

    void* ptr = mmap(nullptr, aligned_size,
                     PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS,
                     -1, 0);

    if (ptr == MAP_FAILED) {
        return nullptr;
    }

    // Advise kernel to use huge pages (requires THP enabled)
    madvise(ptr, aligned_size, MADV_HUGEPAGE);

    // Touch all pages to ensure they're allocated
    memset(ptr, 0, aligned_size);

    return ptr;
}

// ============================================================================
// Benchmark: Random access with regular pages
// ============================================================================

static void BM_RegularPages(benchmark::State& state) {
    const size_t size = state.range(0) * 1024 * 1024; // Size in MB
    long* data = static_cast<long*>(allocate_regular_pages(size));

    if (!data) {
        state.SkipWithError("Failed to allocate memory");
        return;
    }

    const size_t num_elements = size / sizeof(long);

    // Initialize with pattern
    for (size_t i = 0; i < num_elements; ++i) {
        data[i] = i;
    }

    for (auto _ : state) {
        long sum = 0;
        // Stride access pattern to cause TLB misses
        for (size_t i = 0; i < num_elements; i += 512) {
            sum += data[i];
        }
        benchmark::DoNotOptimize(sum);
    }

    munmap(data, size);

    state.SetBytesProcessed(state.iterations() * size);
}
BENCHMARK(BM_RegularPages)->Arg(64)->Arg(128)->Arg(256);

// ============================================================================
// Benchmark: Random access with huge pages
// ============================================================================

static void BM_HugePages(benchmark::State& state) {
    const size_t size = state.range(0) * 1024 * 1024; // Size in MB
    long* data = static_cast<long*>(allocate_huge_pages(size));

    if (!data) {
        state.SkipWithError("Failed to allocate memory");
        return;
    }

    const size_t aligned_size = ((size + HUGE_PAGE_SIZE - 1) / HUGE_PAGE_SIZE) * HUGE_PAGE_SIZE;
    const size_t num_elements = size / sizeof(long);

    // Initialize with pattern
    for (size_t i = 0; i < num_elements; ++i) {
        data[i] = i;
    }

    for (auto _ : state) {
        long sum = 0;
        // Stride access pattern to highlight TLB benefit
        for (size_t i = 0; i < num_elements; i += 512) {
            sum += data[i];
        }
        benchmark::DoNotOptimize(sum);
    }

    munmap(data, aligned_size);

    state.SetBytesProcessed(state.iterations() * size);
}
BENCHMARK(BM_HugePages)->Arg(64)->Arg(128)->Arg(256);

// ============================================================================
// Sequential access comparison
// ============================================================================

static void BM_RegularPagesSequential(benchmark::State& state) {
    const size_t size = state.range(0) * 1024 * 1024;
    long* data = static_cast<long*>(allocate_regular_pages(size));

    if (!data) {
        state.SkipWithError("Failed to allocate memory");
        return;
    }

    const size_t num_elements = size / sizeof(long);

    for (size_t i = 0; i < num_elements; ++i) {
        data[i] = i;
    }

    for (auto _ : state) {
        long sum = 0;
        for (size_t i = 0; i < num_elements; ++i) {
            sum += data[i];
        }
        benchmark::DoNotOptimize(sum);
    }

    munmap(data, size);

    state.SetBytesProcessed(state.iterations() * size);
}
BENCHMARK(BM_RegularPagesSequential)->Arg(64)->Arg(128)->Arg(256);

static void BM_HugePagesSequential(benchmark::State& state) {
    const size_t size = state.range(0) * 1024 * 1024;
    long* data = static_cast<long*>(allocate_huge_pages(size));

    if (!data) {
        state.SkipWithError("Failed to allocate memory");
        return;
    }

    const size_t aligned_size = ((size + HUGE_PAGE_SIZE - 1) / HUGE_PAGE_SIZE) * HUGE_PAGE_SIZE;
    const size_t num_elements = size / sizeof(long);

    for (size_t i = 0; i < num_elements; ++i) {
        data[i] = i;
    }

    for (auto _ : state) {
        long sum = 0;
        for (size_t i = 0; i < num_elements; ++i) {
            sum += data[i];
        }
        benchmark::DoNotOptimize(sum);
    }

    munmap(data, aligned_size);

    state.SetBytesProcessed(state.iterations() * size);
}
BENCHMARK(BM_HugePagesSequential)->Arg(64)->Arg(128)->Arg(256);

BENCHMARK_MAIN();
