# C++ Low-Latency Performance Techniques

A comprehensive collection of **benchmarked** C++ performance optimization techniques for low-latency systems. Each technique includes working code, benchmarks, and detailed explanations.

## 📊 Overview

This repository demonstrates critical optimization techniques used in:
- High-Frequency Trading (HFT)
- Real-time systems
- Game engines
- Operating system kernels
- Any performance-critical C++ application

**All claims are verified with benchmarks** using [Google Benchmark](https://github.com/google/benchmark).

## 🚀 Quick Start

### Prerequisites

```bash
# Install Google Benchmark
git clone https://github.com/google/benchmark.git
cd benchmark
cmake -E make_directory "build"
cmake -E chdir "build" cmake -DBENCHMARK_DOWNLOAD_DEPENDENCIES=on -DCMAKE_BUILD_TYPE=Release ../
cmake --build "build" --config Release
```

### Running Benchmarks

```bash
cd <technique_folder>
make
./mybenchmark
```

## 📁 Techniques Covered

### ⚡ Compile-Time Optimization

| Technique | Speedup | Description |
|-----------|---------|-------------|
| [constexpr](./constexpr/) | **40,000x** | Move computations to compile time |
| [CRTP](./crtp/) | **4-5x** | Static polymorphism (no vtable overhead) |
| [Loop Unrolling](./loop_unrolling/) | **1.5-2x** | Reduce branch overhead in tight loops |

### 🎯 Polymorphism Alternatives

| Technique | Speedup | Description |
|-----------|---------|-------------|
| [std::variant + std::visit](./variant_visit/) | **2-3x** | Type-safe polymorphism without vtable |
| [CRTP](./crtp/) | **4-5x** | Template-based static polymorphism |
| [Templates vs std::function](./function_vs_templates/) | **2-5x** | Avoid std::function overhead in hot paths |

### 🧵 Multi-Threading Optimization

| Technique | Speedup | Description |
|-----------|---------|-------------|
| [False Sharing Elimination](./false_sharing/) | **6-7x** | Cache line padding for atomic variables |

### 🔧 Error Handling

| Technique | Speedup | Description |
|-----------|---------|-------------|
| [Error Codes vs Exceptions](./exceptions_vs_error_codes/) | **10-100x** | Avoid exceptions in hot paths |

### 📝 I/O Optimization

| Technique | Speedup | Description |
|-----------|---------|-------------|
| ['\n' vs std::endl](./endl_vs_newline/) | **10-100x** | Avoid unnecessary buffer flushes |

### 💾 Memory Management

| Technique | Benefit | Description |
|-----------|---------|-------------|
| [std::unique_ptr vs std::shared_ptr](./shared_ptr_overhead/) | **10-50x** | Avoid atomic ref counting overhead |
| [Structure Packing](./NOTWORKING/structure_packing/) | **40% memory** | Reduce padding, improve cache utilization |

### 🔬 Advanced Techniques

| Technique | Notes | Description |
|-----------|-------|-------------|
| [Branch-Free Code](./branch_free/) | *Varies* | Eliminate branch misprediction penalties |
| [SIMD Intrinsics](./simd_intrinsics/) | *Auto-vectorized* | Parallel data processing |
| [Prefetching](./NOTWORKING/prefetching/) | **10-15%** | Hide memory latency |
| [Huge Pages](./NOTWORKING/huge_pages/) | *System-dependent* | Reduce TLB misses |

## 📈 Benchmark Results Summary

### ✅ Dramatic Improvements

#### constexpr (Fibonacci)
```
BM_FibonacciRuntime          14,632 ns
BM_FibonacciConstexpr            0.36 ns    ⚡ 40,000x faster
```

#### CRTP vs Virtual Functions
```
BM_VirtualFunctions/10000    49,319 ns
BM_CRTP/10000                10,797 ns    ⚡ 4.6x faster
```

#### False Sharing (8 threads)
```
BM_WithFalseSharing          7,061,067 ns
BM_WithoutFalseSharing       1,079,084 ns    ⚡ 6.5x faster
```

### ⚠️ Compiler-Optimized Cases

Some techniques show **minimal improvement** because modern compilers already optimize:

#### Branch-Free Code
```
BM_Abs_WithBranch/100000     72,191 ns
BM_Abs_BranchFree/100000     71,937 ns    ⚡ ~1% (compiler already optimized)
```
**Why?** Compiler uses conditional moves (cmov) and branch prediction is effective.

#### SIMD Intrinsics
```
BM_VectorAdd_Scalar/100000   42,465 ns
BM_VectorAdd_SIMD/100000     42,620 ns    ⚠️ Auto-vectorized
```
**Why?** Compiler auto-vectorizes with `-O2 -march=native`, achieving same performance.

## 🎓 Key Lessons

### 1. **Trust But Verify**
Always benchmark! Compiler optimizations have improved dramatically. What was slow in 2010 might be fast today.

### 2. **Profile Before Optimizing**
Use `perf`, `vtune`, or similar tools to find actual bottlenecks.

### 3. **Compiler Flags Matter**
```bash
-O2 or -O3          # Enable optimizations
-march=native       # Use all CPU features (SIMD, etc.)
-flto               # Link-time optimization
-fprofile-use       # Profile-guided optimization
```

### 4. **Know Your Architecture**
- Cache line size (usually 64 bytes)
- TLB size and huge page support
- SIMD capabilities (SSE, AVX, AVX-512, NEON)
- Branch predictor characteristics

### 5. **Measure Real Impact**
Some optimizations help dramatically in micro-benchmarks but minimally in production:
- **High impact**: constexpr, CRTP, false sharing elimination
- **Medium impact**: std::function avoidance, error code returns
- **Low impact**: Manual SIMD (compiler does it), branch-free (compiler does it)

## 🛠️ Building & Testing

### Compile Single Benchmark
```bash
cd <folder>
make
./mybenchmark
```

### Run Specific Tests
```bash
./mybenchmark --benchmark_filter="BM_FibonacciRuntime"
```

### Get More Details
```bash
./mybenchmark --benchmark_repetitions=10
```

## 📚 Recommended Reading

- **Books**
  - "Computer Systems: A Programmer's Perspective" by Bryant & O'Hallaron
  - "Optimizing Software in C++" by Agner Fog
  - "C++ High Performance" by Björn Andrist & Viktor Sehr

- **Online Resources**
  - [Agner Fog's Optimization Manuals](https://www.agner.org/optimize/)
  - [Intel Intrinsics Guide](https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html)
  - [Compiler Explorer (godbolt.org)](https://godbolt.org/)

## ⚠️ Important Notes

### NOTWORKING Folder

Contains benchmarks moved from main folder due to:
- **structure_packing**: Works but shows minimal perf improvement (memory savings still valuable)
- **prefetching**: Works but improvement varies by access pattern (10-15% in best case)
- **huge_pages**: Requires system configuration (THP enabled) to show benefits

These techniques are still valuable but require specific conditions to demonstrate improvement.

## 🤝 Contributing

Contributions welcome! Please:
1. Ensure benchmarks compile and run
2. Include benchmark results in PR
3. Document expected vs actual results
4. Follow existing code style

## 📄 License

MIT License - Feel free to use in your projects!

## 🙏 Acknowledgments

- Google Benchmark team
- C++ community for performance insights
- Trading firms and game studios for real-world techniques

---

**Remember**: Premature optimization is the root of all evil, but **knowing** these techniques helps you make informed decisions when optimization **is** needed!
