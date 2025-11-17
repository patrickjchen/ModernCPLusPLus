#include <benchmark/benchmark.h>
#include <sstream>
#include <fstream>
#include <iostream>

// ============================================================================
// std::endl vs '\n' - The Hidden Performance Killer
//
// std::endl does TWO things:
// 1. Writes a newline character
// 2. FLUSHES the buffer (expensive I/O operation)
//
// '\n' only writes a newline, letting the buffer flush naturally
//
// The performance difference can be HUGE (10-100x slower with endl)
// ============================================================================

// ============================================================================
// Benchmark: Writing to stringstream
// ============================================================================

static void BM_Stringstream_Endl(benchmark::State& state) {
    for (auto _ : state) {
        std::ostringstream oss;
        for (int i = 0; i < state.range(0); ++i) {
            oss << "Line " << i << std::endl;  // Flush on every line!
        }
        benchmark::DoNotOptimize(oss.str());
    }
}
BENCHMARK(BM_Stringstream_Endl)->Arg(100)->Arg(1000)->Arg(10000);

static void BM_Stringstream_Newline(benchmark::State& state) {
    for (auto _ : state) {
        std::ostringstream oss;
        for (int i = 0; i < state.range(0); ++i) {
            oss << "Line " << i << '\n';  // No flush
        }
        benchmark::DoNotOptimize(oss.str());
    }
}
BENCHMARK(BM_Stringstream_Newline)->Arg(100)->Arg(1000)->Arg(10000);

// ============================================================================
// Benchmark: Writing to file
// ============================================================================

static void BM_File_Endl(benchmark::State& state) {
    const char* filename = "/tmp/benchmark_endl.txt";

    for (auto _ : state) {
        std::ofstream file(filename);
        for (int i = 0; i < state.range(0); ++i) {
            file << "Line " << i << std::endl;  // Flush + syscall overhead!
        }
        file.close();
    }

    std::remove(filename);
}
BENCHMARK(BM_File_Endl)->Arg(100)->Arg(1000)->Arg(5000);

static void BM_File_Newline(benchmark::State& state) {
    const char* filename = "/tmp/benchmark_newline.txt";

    for (auto _ : state) {
        std::ofstream file(filename);
        for (int i = 0; i < state.range(0); ++i) {
            file << "Line " << i << '\n';  // Buffered writes
        }
        file.close();
    }

    std::remove(filename);
}
BENCHMARK(BM_File_Newline)->Arg(100)->Arg(1000)->Arg(5000);

static void BM_File_Newline_ManualFlush(benchmark::State& state) {
    const char* filename = "/tmp/benchmark_manual.txt";

    for (auto _ : state) {
        std::ofstream file(filename);
        for (int i = 0; i < state.range(0); ++i) {
            file << "Line " << i << '\n';
        }
        file.flush();  // Flush once at the end
        file.close();
    }

    std::remove(filename);
}
BENCHMARK(BM_File_Newline_ManualFlush)->Arg(100)->Arg(1000)->Arg(5000);

// ============================================================================
// Benchmark: Logging scenario (typical use case)
// ============================================================================

class Logger {
public:
    void log_with_endl(const std::string& message) {
        oss_ << "[INFO] " << message << std::endl;
    }

    void log_with_newline(const std::string& message) {
        oss_ << "[INFO] " << message << '\n';
    }

    std::string get_logs() const { return oss_.str(); }

private:
    std::ostringstream oss_;
};

static void BM_Logging_Endl(benchmark::State& state) {
    for (auto _ : state) {
        Logger logger;
        for (int i = 0; i < state.range(0); ++i) {
            logger.log_with_endl("Processing order " + std::to_string(i));
        }
        benchmark::DoNotOptimize(logger.get_logs());
    }
}
BENCHMARK(BM_Logging_Endl)->Arg(100)->Arg(1000)->Arg(10000);

static void BM_Logging_Newline(benchmark::State& state) {
    for (auto _ : state) {
        Logger logger;
        for (int i = 0; i < state.range(0); ++i) {
            logger.log_with_newline("Processing order " + std::to_string(i));
        }
        benchmark::DoNotOptimize(logger.get_logs());
    }
}
BENCHMARK(BM_Logging_Newline)->Arg(100)->Arg(1000)->Arg(10000);

// ============================================================================
// Benchmark: Multiple fields per line (CSV-like output)
// ============================================================================

static void BM_CSV_Endl(benchmark::State& state) {
    for (auto _ : state) {
        std::ostringstream oss;
        for (int i = 0; i < state.range(0); ++i) {
            oss << i << "," << (i * 2) << "," << (i * 3) << std::endl;
        }
        benchmark::DoNotOptimize(oss.str());
    }
}
BENCHMARK(BM_CSV_Endl)->Arg(100)->Arg(1000)->Arg(10000);

static void BM_CSV_Newline(benchmark::State& state) {
    for (auto _ : state) {
        std::ostringstream oss;
        for (int i = 0; i < state.range(0); ++i) {
            oss << i << "," << (i * 2) << "," << (i * 3) << '\n';
        }
        benchmark::DoNotOptimize(oss.str());
    }
}
BENCHMARK(BM_CSV_Newline)->Arg(100)->Arg(1000)->Arg(10000);

// ============================================================================
// Benchmark: Trade execution reporting
// ============================================================================

struct Trade {
    int order_id;
    double price;
    int quantity;
    long timestamp;
};

static void BM_TradeReport_Endl(benchmark::State& state) {
    std::vector<Trade> trades;
    for (int i = 0; i < state.range(0); ++i) {
        trades.push_back({i, 100.0 + i * 0.5, 100 + i, 1000000 + i});
    }

    for (auto _ : state) {
        std::ostringstream oss;
        for (const auto& trade : trades) {
            oss << "TRADE," << trade.order_id << "," << trade.price
                << "," << trade.quantity << "," << trade.timestamp << std::endl;
        }
        benchmark::DoNotOptimize(oss.str());
    }
}
BENCHMARK(BM_TradeReport_Endl)->Arg(100)->Arg(1000)->Arg(10000);

static void BM_TradeReport_Newline(benchmark::State& state) {
    std::vector<Trade> trades;
    for (int i = 0; i < state.range(0); ++i) {
        trades.push_back({i, 100.0 + i * 0.5, 100 + i, 1000000 + i});
    }

    for (auto _ : state) {
        std::ostringstream oss;
        for (const auto& trade : trades) {
            oss << "TRADE," << trade.order_id << "," << trade.price
                << "," << trade.quantity << "," << trade.timestamp << '\n';
        }
        benchmark::DoNotOptimize(oss.str());
    }
}
BENCHMARK(BM_TradeReport_Newline)->Arg(100)->Arg(1000)->Arg(10000);

BENCHMARK_MAIN();
