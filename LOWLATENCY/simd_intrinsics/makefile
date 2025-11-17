mybenchmark: mybenchmark.cc
	g++ mybenchmark.cc -O2 -std=c++20 -march=native -isystem benchmark/include -Lbenchmark/build/src -lbenchmark -lpthread -o mybenchmark

# -march=native enables the best SIMD instructions for your CPU
# For specific targets:
# -msse4.2    : SSE 4.2
# -mavx       : AVX
# -mavx2      : AVX2
# -mavx512f   : AVX-512
