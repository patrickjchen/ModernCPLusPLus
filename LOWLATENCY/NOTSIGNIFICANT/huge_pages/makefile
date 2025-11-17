mybenchmark: mybenchmark.cc
	g++ mybenchmark.cc -O2 -std=c++20 -isystem benchmark/include -Lbenchmark/build/src -lbenchmark -lpthread -o mybenchmark

# Note: For transparent huge pages, you may need:
# echo always > /sys/kernel/mm/transparent_hugepage/enabled
# or use madvise with MADV_HUGEPAGE
