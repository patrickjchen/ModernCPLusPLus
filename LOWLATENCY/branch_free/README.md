# Branch-Free Programming

## What is Branch-Free Code?

Code that eliminates conditional branches (if/else) using arithmetic and bitwise operations instead.

## Why Care About Branches?

Modern CPUs use **branch prediction** to speculatively execute code. When prediction is **wrong**:
- **10-20 cycle penalty** from pipeline flush
- Critical in tight loops with unpredictable data

## Examples in This Benchmark

### 1. Absolute Value
```cpp
// With branch
int abs_with_branch(int x) {
    if (x < 0) return -x;
    return x;
}

// Branch-free
int abs_branch_free(int x) {
    int mask = x >> 31;  // All 1s if negative, all 0s if positive
    return (x + mask) ^ mask;
}
```

### 2. Min/Max
```cpp
// Branch-free min
int min_branch_free(int a, int b) {
    return b + ((a - b) & ((a - b) >> 31));
}
```

### 3. Conditional Assignment
```cpp
// Branch-free
void update_branch_free(int& value, int threshold, int new_value) {
    int mask = -(value > threshold);
    value = (mask & new_value) | (~mask & value);
}
```

## Benchmark Results

```
BM_Abs_WithBranch/100000      72,191 ns
BM_Abs_BranchFree/100000      71,937 ns    (~1% difference)
```

## Why No Improvement?

### Modern Compiler Optimizations

1. **Conditional Moves (cmov)**
   ```bash
   # Check assembly
   g++ -O2 -S -masm=intel code.cc
   # Look for: cmov instructions instead of branches
   ```

2. **Branch Prediction**
   - Modern CPUs have sophisticated branch predictors
   - Pattern-based predictions work well for many cases

3. **Compiler Intelligence**
   - With `-O2`, compiler already converts simple branches to cmov
   - Automatically applies branch-free techniques

### When Branch-Free Still Helps

1. **Truly Random Data**
   - Crypto operations
   - Hash functions
   - Random number processing

2. **Security-Critical Code**
   - Prevent timing attacks
   - Constant-time operations

3. **GPU/SIMD Code**
   - Branches cause divergence
   - All lanes must execute both paths

4. **Embedded Systems**
   - Weaker branch predictors
   - Smaller pipelines

## How to Verify Impact

### Check Branch Mispredictions
```bash
perf stat -e branches,branch-misses ./mybenchmark
```

### Compare Assembly
```bash
# Compile both versions
g++ -O2 -S -masm=intel -o with_branch.s with_branch.cc
g++ -O2 -S -masm=intel -o branch_free.s branch_free.cc

# Compare
diff with_branch.s branch_free.s
```

### Disable Optimizations
```bash
# Try with -O0 to see raw difference
g++ -O0 mybenchmark.cc ...
```

## Key Takeaway

**Branch-free programming is a valuable technique** but modern compilers already apply it in many cases.

**When to manually apply:**
- Cryptographic code (constant-time requirement)
- GPU kernels
- After profiling shows branch mispredictions
- Embedded systems with weak predictors

**Don't manually optimize** unless you've:
1. Profiled and found branch mispredictions
2. Checked compiler isn't already doing it
3. Verified improvement with benchmarks
