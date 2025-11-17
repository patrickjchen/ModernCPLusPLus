# SIMD Intrinsics Benchmark

## What is SIMD?

**Single Instruction, Multiple Data** - Process multiple data elements simultaneously with one instruction.

## Architecture Support

### x86/x64 (Intel/AMD)
- **SSE** (128-bit): 4 floats or 2 doubles at once
- **AVX** (256-bit): 8 floats or 4 doubles at once
- **AVX2**: AVX + integer operations
- **AVX-512** (512-bit): 16 floats or 8 doubles at once

### ARM/ARM64
- **NEON** (128-bit): 4 floats at once
- **SVE**: Scalable vector extension (variable width)

### Other
- **PowerPC**: AltiVec/VMX
- **RISC-V**: Vector extension

## This Benchmark

Demonstrates x86/x64 SIMD with fallback to scalar code. Automatically detects and uses the best available instruction set on your CPU.

**Examples included:**
1. Vector addition
2. Dot product (with horizontal reduction)
3. Finding maximum value
4. Array scaling

## Building

```bash
make
./mybenchmark
```

The `-march=native` flag enables the best SIMD for your CPU.

## Benchmark Results

```
BM_VectorAdd_Scalar/100000   42,465 ns
BM_VectorAdd_SIMD/100000     42,620 ns    (No difference!)
```

## Why No Improvement?

### Compiler Auto-Vectorization

Modern compilers **automatically** vectorize simple loops with `-O2 -march=native`:

```bash
g++ -O2 -march=native -fopt-info-vec-optimized mybenchmark.cc

# Output:
# mybenchmark.cc:44:26: optimized: loop vectorized using 32 byte vectors
```

**The scalar version is already using SIMD!**

### Check Generated Assembly

```bash
g++ -O2 -march=native -S -masm=intel mybenchmark.cc
objdump -d mybenchmark -M intel | grep vmovups

# You'll see AVX instructions (vmovups, vaddps) in BOTH versions
```

## Expected Speedups (When Manual SIMD Helps)

When compiler **can't** auto-vectorize:
- **SSE (4-wide)**: ~3-4x faster
- **AVX (8-wide)**: ~6-8x faster
- **AVX-512 (16-wide)**: ~10-15x faster

Actual speedup depends on memory bandwidth and data alignment.

## Portable SIMD Alternatives

For cross-platform code:
1. **Compiler auto-vectorization** - Let the compiler do it (use `-O3 -march=native`)
2. **Google Highway** - Write once, runs on all SIMD architectures
3. **xsimd** - Header-only SIMD wrapper
4. **Eigen** - Linear algebra library with SIMD support

## When Manual SIMD Is Still Needed

✅ **Compiler can't auto-vectorize:**
- Complex access patterns (gather/scatter)
- Mixed data types
- Horizontal operations (reductions, shuffles)
- AoS → SoA conversions
- Bit manipulation
- Special functions (trigonometry, etc.)

✅ **Fine-grained control needed:**
- Explicit memory alignment
- Prefetching hints
- Cache control
- Specific instruction selection

✅ **Performance-critical kernels:**
- Image/video codecs
- Audio DSP
- Cryptography
- Physics engines
- ML inference

❌ **Bad use cases (let compiler handle it):**
- Simple loops (add, multiply, min, max)
- Small arrays (overhead > benefit)
- Non-contiguous memory
- Portable code requirements

## Debugging SIMD

Check if compiler is auto-vectorizing:
```bash
g++ -O3 -march=native -fopt-info-vec-optimized file.cc
```

View generated assembly:
```bash
g++ -O3 -march=native -S -masm=intel file.cc
```
