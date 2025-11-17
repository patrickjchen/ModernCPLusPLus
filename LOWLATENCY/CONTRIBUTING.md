# Contributing to C++ Low-Latency Performance Techniques

Thank you for your interest in contributing! This project aims to provide **honest, benchmarked** examples of C++ performance techniques.

## 📝 Contribution Guidelines

### Adding New Techniques

When adding a new performance technique:

1. **Create a new folder** following the naming convention (lowercase with underscores)
2. **Include these files:**
   - `mybenchmark.cc` - Complete, runnable benchmark
   - `makefile` - Following the existing pattern
   - `README.md` - (Optional) If the technique needs explanation

3. **Benchmark Requirements:**
   - Use Google Benchmark framework
   - Compare at least 2 approaches (baseline vs optimized)
   - Test with multiple input sizes (e.g., 100, 1000, 10000)
   - Include `benchmark::DoNotOptimize()` to prevent optimization
   - Compile with `-O2 -std=c++20`

4. **Be Honest About Results:**
   - If your technique shows minimal improvement, **that's valuable information!**
   - Document why (compiler optimization, architecture, etc.)
   - Consider adding to README explaining the conditions where it helps

### Example Structure

```
new_technique/
├── makefile
├── mybenchmark.cc
└── README.md (optional)
```

### Makefile Template

```makefile
mybenchmark: mybenchmark.cc
	g++ mybenchmark.cc -O2 -std=c++20 -isystem benchmark/include -Lbenchmark/build/src -lbenchmark -lpthread -o mybenchmark
```

### Benchmark Code Template

```cpp
#include <benchmark/benchmark.h>

// Baseline implementation
static void BM_Baseline(benchmark::State& state) {
    for (auto _ : state) {
        // Your baseline code
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Baseline)->Arg(1000)->Arg(10000);

// Optimized implementation
static void BM_Optimized(benchmark::State& state) {
    for (auto _ : state) {
        // Your optimized code
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Optimized)->Arg(1000)->Arg(10000);

BENCHMARK_MAIN();
```

## 🔍 Code Review Checklist

Before submitting a PR, ensure:

- [ ] Code compiles with `make`
- [ ] Benchmark runs without errors
- [ ] Results are reproducible
- [ ] Code is well-commented
- [ ] Includes actual benchmark output in PR description
- [ ] README updated if adding major technique
- [ ] Honest about performance results (even if minimal)

## 📊 Running Your Benchmarks

```bash
cd your_technique/
make
./mybenchmark

# Run specific tests
./mybenchmark --benchmark_filter="BM_Baseline.*"

# Get statistics
./mybenchmark --benchmark_repetitions=10
```

## 🎯 Good PR Examples

### Example 1: Technique Shows Improvement
```markdown
## Lock-Free Queue vs Mutex Queue

Benchmark results:
```
BM_MutexQueue/1000     45,231 ns
BM_LockFreeQueue/1000   8,942 ns  (5x faster)
```

Testing environment:
- CPU: Intel i7-9700K
- Threads: 4
- Compiler: g++ 11.4 with -O2
```

### Example 2: Technique Shows No Improvement
```markdown
## Manual Memory Alignment

Benchmark results:
```
BM_Unaligned/10000     12,345 ns
BM_Aligned/10000       12,298 ns  (~1% difference)
```

**Analysis:** Compiler already aligns data with -O2. This technique
may help on older compilers or with specific data structures.

Moved to NOTWORKING/ with README explaining conditions.
```

## 🚫 What NOT to Submit

- Techniques without benchmarks
- Code that doesn't compile
- Micro-optimizations without measurable impact
- Platform-specific code without fallbacks (unless documented)
- Copied code without attribution

## 💡 Ideas for Contributions

### Wanted Techniques:
- Memory allocator comparisons
- Lock-free data structures
- Cache-aware algorithms
- Profile-guided optimization examples
- Compiler flag comparisons
- Different STL container comparisons

### Improvements Needed:
- Platform testing (ARM, RISC-V)
- Compiler comparisons (GCC vs Clang vs MSVC)
- More real-world scenarios
- Multi-threading patterns

## 🐛 Bug Reports

When reporting issues:
1. Include your system info (`uname -a`, `g++ --version`)
2. Copy the error message
3. Describe what you expected vs what happened
4. Include steps to reproduce

## 📖 Documentation

Help improve documentation:
- Fix typos and clarity
- Add explanations for complex techniques
- Improve README structure
- Add diagrams or visual aids

## 🤝 Code of Conduct

- Be respectful and constructive
- Focus on technical merit
- Welcome newcomers
- Assume good intentions

## ❓ Questions?

- Open an issue for discussion
- Check existing issues first
- Use clear, descriptive titles

## 📄 License

By contributing, you agree that your contributions will be licensed under the MIT License.

---

**Remember:** Honest benchmarks that show "no improvement" are just as valuable as ones showing huge speedups. Both teach us about modern compilers and CPU architecture!
