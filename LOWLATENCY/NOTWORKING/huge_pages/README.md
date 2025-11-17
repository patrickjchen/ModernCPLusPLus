# Huge Pages Benchmark

## What are Huge Pages?

- **Regular pages**: 4KB (4096 bytes)
- **Huge pages**: 2MB (2097152 bytes) or 1GB

## Benefits

1. **Fewer TLB misses**: TLB (Translation Lookaside Buffer) caches virtual-to-physical address translations. Larger pages mean fewer entries needed.
2. **Reduced page table overhead**: Fewer page table entries to manage
3. **Better performance for large allocations**: Especially beneficial for databases, in-memory data structures, and HPC applications

## System Setup (Linux)

### Check current huge pages configuration:

```bash
cat /proc/meminfo | grep Huge
```

### Enable Transparent Huge Pages (THP):

```bash
# Check current status
cat /sys/kernel/mm/transparent_hugepage/enabled

# Enable (requires root)
echo always | sudo tee /sys/kernel/mm/transparent_hugepage/enabled

# Or use madvise mode (recommended - allows per-allocation control)
echo madvise | sudo tee /sys/kernel/mm/transparent_hugepage/enabled
```

### Pre-allocate huge pages (alternative approach):

```bash
# Allocate 100 huge pages (200MB)
echo 100 | sudo tee /proc/sys/vm/nr_hugepages

# Check allocation
cat /proc/meminfo | grep HugePages_
```

## Building and Running

```bash
make
./mybenchmark
```

## Expected Results

- **Stride access pattern**: Huge pages should show 10-30% improvement due to fewer TLB misses
- **Sequential access**: Smaller difference, but still measurable improvement
- Larger working sets (256MB+) will show more pronounced benefits

## Notes

- If THP is disabled, both benchmarks will perform similarly
- The benchmark uses `madvise(MADV_HUGEPAGE)` which requires THP in "madvise" or "always" mode
- You can verify huge page usage with: `cat /proc/<pid>/smaps | grep -i huge`
