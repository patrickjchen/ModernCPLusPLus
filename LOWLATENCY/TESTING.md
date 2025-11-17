# Testing Guide

This document explains how to build and run all benchmarks in this repository.

## Prerequisites

### 1. Install Google Benchmark

```bash
git clone https://github.com/google/benchmark.git
cd benchmark
cmake -E make_directory "build"
cmake -E chdir "build" cmake -DBENCHMARK_DOWNLOAD_DEPENDENCIES=on -DCMAKE_BUILD_TYPE=Release ../
cmake --build "build" --config Release
sudo cmake --build "build" --target install
```

### 2. Verify Installation

```bash
# Check if benchmark library is installed
ls /usr/local/lib/libbenchmark*
# or
ls benchmark/build/src/libbenchmark*
```

## Running Individual Benchmarks

### Basic Usage

```bash
cd <technique_folder>
make
./mybenchmark
```

### Example

```bash
cd constexpr
make
./mybenchmark
```

Expected output:
```
Run on (8 X 3600 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x4)
  L1 Instruction 32 KiB (x4)
  L2 Unified 256 KiB (x4)
  L3 Unified 8192 KiB (x1)
-----------------------------------------------------------------
Benchmark                       Time             CPU   Iterations
-----------------------------------------------------------------
BM_FibonacciRuntime         14632 ns        14632 ns        47367
BM_FibonacciConstexpr        0.36 ns         0.36 ns   1962314419
```

## Running Specific Tests

### Filter by Pattern

```bash
./mybenchmark --benchmark_filter="BM_Fibonacci.*"
```

### Run with Repetitions

```bash
./mybenchmark --benchmark_repetitions=10
```

### Show Statistics

```bash
./mybenchmark --benchmark_repetitions=10 --benchmark_report_aggregates_only=true
```

## Testing All Benchmarks

### Quick Test Script

```bash
#!/bin/bash

FOLDERS=(
    "constexpr"
    "crtp"
    "false_sharing"
    "exceptions_vs_error_codes"
    "function_vs_templates"
    "endl_vs_newline"
    "shared_ptr_overhead"
    "variant_visit"
    "loop_unrolling"
    "branch_free"
    "simd_intrinsics"
)

for folder in "${FOLDERS[@]}"; do
    echo "===== Testing $folder ====="
    cd "$folder" || continue
    make && ./mybenchmark --benchmark_filter=".*" --benchmark_min_time=0.1
    cd ..
    echo ""
done
```

### NOTWORKING Folder

These benchmarks work but have specific requirements:

```bash
# Structure packing
cd NOTWORKING/structure_packing
make && timeout 10 ./mybenchmark

# Prefetching
cd NOTWORKING/prefetching
make && ./mybenchmark

# Huge pages (requires THP enabled)
cd NOTWORKING/huge_pages
make && ./mybenchmark
```

## Troubleshooting

### Compilation Errors

#### "benchmark/benchmark.h: No such file or directory"

**Solution:** Update makefile to point to your benchmark installation:

```makefile
# If installed system-wide:
mybenchmark: mybenchmark.cc
	g++ mybenchmark.cc -O2 -std=c++20 -lbenchmark -lpthread -o mybenchmark

# If local installation:
mybenchmark: mybenchmark.cc
	g++ mybenchmark.cc -O2 -std=c++20 -isystem benchmark/include -Lbenchmark/build/src -lbenchmark -lpthread -o mybenchmark
```

#### "cannot find -lbenchmark"

**Solution:** Set library path:

```bash
export LD_LIBRARY_PATH=/path/to/benchmark/build/src:$LD_LIBRARY_PATH
```

### Runtime Issues

#### "Infinite loop" in structure_packing

**Fixed.** If you see this on an old version, update to latest code.

#### Huge pages showing no benefit

**Expected.** Check THP status:

```bash
cat /sys/kernel/mm/transparent_hugepage/enabled
# Should show: always [madvise] never

# Enable if needed:
echo always | sudo tee /sys/kernel/mm/transparent_hugepage/enabled
```

#### SIMD showing no improvement

**Expected.** Compiler auto-vectorizes with `-march=native`. Check:

```bash
g++ -O2 -march=native -fopt-info-vec-optimized mybenchmark.cc 2>&1 | grep vectorized
```

## Performance Analysis

### Using perf

```bash
# Count events
perf stat ./mybenchmark

# Focus on cache misses
perf stat -e cache-references,cache-misses ./mybenchmark

# Branch prediction
perf stat -e branches,branch-misses ./mybenchmark
```

### Check Assembly

```bash
# Generate assembly
g++ -O2 -march=native -S -masm=intel mybenchmark.cc -o mybenchmark.s

# View specific function
objdump -d mybenchmark -M intel | grep -A 20 "my_function"
```

### Profile with gprof

```bash
g++ mybenchmark.cc -O2 -pg -isystem benchmark/include -Lbenchmark/build/src -lbenchmark -lpthread -o mybenchmark
./mybenchmark
gprof mybenchmark gmon.out > analysis.txt
```

## Expected Results Summary

| Benchmark | Expected Speedup | Status |
|-----------|------------------|--------|
| constexpr | 40,000x | ✅ |
| CRTP | 4-5x | ✅ |
| False sharing | 6-7x | ✅ |
| Exceptions (error path) | 180x | ✅ |
| function→template | 10x | ✅ |
| endl→'\n' (file) | 10x | ✅ |
| shared_ptr→raw | 6x | ✅ |
| variant vs virtual | 2-3x | ✅ |
| Loop unrolling | 1.5-2x | ✅ |
| Branch-free | ~1% | ⚠️ Compiler optimized |
| SIMD | ~1% | ⚠️ Auto-vectorized |
| Structure packing | Memory only | 📊 40% savings |
| Prefetching | 10-15% | 📊 Conditional |
| Huge pages | Varies | 🔧 System-dependent |

## System Requirements

### Minimum
- g++ 9.0+ or clang 10.0+
- C++20 support
- Google Benchmark library

### Recommended
- g++ 11.0+ or clang 14.0+
- CPU with AVX2 support (for SIMD)
- Linux/WSL2 (for huge pages, perf tools)

### Tested On
- Ubuntu 20.04/22.04
- WSL2 (Windows Subsystem for Linux)
- Arch Linux
- macOS (most benchmarks)

## Continuous Integration

### GitHub Actions Example

```yaml
name: Benchmarks

on: [push, pull_request]

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v3

    - name: Install dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y cmake build-essential

    - name: Build Google Benchmark
      run: |
        git clone https://github.com/google/benchmark.git
        cd benchmark
        cmake -E make_directory "build"
        cmake -E chdir "build" cmake -DBENCHMARK_DOWNLOAD_DEPENDENCIES=on -DCMAKE_BUILD_TYPE=Release ../
        cmake --build "build" --config Release
        sudo cmake --build "build" --target install

    - name: Test benchmarks
      run: |
        for dir in constexpr crtp false_sharing; do
          cd $dir && make && timeout 30 ./mybenchmark && cd ..
        done
```

## Contributing Tests

When adding new benchmarks:

1. Ensure `make` builds without warnings
2. Run benchmark at least 3 times
3. Include results in PR
4. Document expected performance range
5. Note any system-specific requirements

See [CONTRIBUTING.md](CONTRIBUTING.md) for details.
