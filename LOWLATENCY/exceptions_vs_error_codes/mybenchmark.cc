#include <benchmark/benchmark.h>
#include <optional>
#include <stdexcept>
#include <string>
#include <variant>

// ============================================================================
// Error handling approaches for low-latency systems
// ============================================================================

enum class ErrorCode {
    Success,
    InvalidInput,
    OutOfRange,
    ParseError,
    NotFound
};

// ============================================================================
// Simple Result type (similar to std::expected from C++23)
// ============================================================================

template<typename T>
class Result {
public:
    Result(T value) : data_(std::move(value)), success_(true) {}
    Result(ErrorCode error) : error_(error), success_(false) {}

    bool is_ok() const { return success_; }
    bool is_error() const { return !success_; }

    const T& value() const { return data_; }
    ErrorCode error() const { return error_; }

private:
    T data_{};
    ErrorCode error_{ErrorCode::Success};
    bool success_;
};

// ============================================================================
// Approach 1: Using Exceptions (SLOW in hot paths)
// ============================================================================

int parse_number_with_exception(const std::string& str) {
    if (str.empty()) {
        throw std::invalid_argument("Empty string");
    }
    if (str[0] == 'x') {
        throw std::runtime_error("Invalid format");
    }
    // Simulate parsing
    return static_cast<int>(str.length());
}

int process_with_exceptions(const std::string& input) {
    try {
        return parse_number_with_exception(input) * 2;
    } catch (const std::exception& e) {
        return -1; // Error indicator
    }
}

// ============================================================================
// Approach 2: Using Error Codes (FAST)
// ============================================================================

int parse_number_with_error_code(const std::string& str, ErrorCode& error) {
    if (str.empty()) {
        error = ErrorCode::InvalidInput;
        return 0;
    }
    if (str[0] == 'x') {
        error = ErrorCode::ParseError;
        return 0;
    }
    error = ErrorCode::Success;
    return static_cast<int>(str.length());
}

int process_with_error_codes(const std::string& input) {
    ErrorCode error;
    int result = parse_number_with_error_code(input, error);
    if (error != ErrorCode::Success) {
        return -1;
    }
    return result * 2;
}

// ============================================================================
// Approach 3: Using Result/Expected type (FAST, type-safe)
// ============================================================================

Result<int> parse_number_with_result(const std::string& str) {
    if (str.empty()) {
        return Result<int>(ErrorCode::InvalidInput);
    }
    if (str[0] == 'x') {
        return Result<int>(ErrorCode::ParseError);
    }
    return Result<int>(static_cast<int>(str.length()));
}

int process_with_result(const std::string& input) {
    auto result = parse_number_with_result(input);
    if (result.is_error()) {
        return -1;
    }
    return result.value() * 2;
}

// ============================================================================
// Approach 4: Using std::optional (FAST, simpler)
// ============================================================================

std::optional<int> parse_number_with_optional(const std::string& str) {
    if (str.empty() || str[0] == 'x') {
        return std::nullopt;
    }
    return static_cast<int>(str.length());
}

int process_with_optional(const std::string& input) {
    auto result = parse_number_with_optional(input);
    if (!result.has_value()) {
        return -1;
    }
    return result.value() * 2;
}

// ============================================================================
// Benchmarks - Success Path (no errors)
// ============================================================================

static void BM_Exceptions_SuccessPath(benchmark::State& state) {
    std::vector<std::string> inputs;
    for (int i = 0; i < state.range(0); ++i) {
        inputs.push_back("valid_" + std::to_string(i));
    }

    for (auto _ : state) {
        int sum = 0;
        for (const auto& input : inputs) {
            sum += process_with_exceptions(input);
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_Exceptions_SuccessPath)->Arg(100)->Arg(1000)->Arg(10000);

static void BM_ErrorCodes_SuccessPath(benchmark::State& state) {
    std::vector<std::string> inputs;
    for (int i = 0; i < state.range(0); ++i) {
        inputs.push_back("valid_" + std::to_string(i));
    }

    for (auto _ : state) {
        int sum = 0;
        for (const auto& input : inputs) {
            sum += process_with_error_codes(input);
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_ErrorCodes_SuccessPath)->Arg(100)->Arg(1000)->Arg(10000);

static void BM_Result_SuccessPath(benchmark::State& state) {
    std::vector<std::string> inputs;
    for (int i = 0; i < state.range(0); ++i) {
        inputs.push_back("valid_" + std::to_string(i));
    }

    for (auto _ : state) {
        int sum = 0;
        for (const auto& input : inputs) {
            sum += process_with_result(input);
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_Result_SuccessPath)->Arg(100)->Arg(1000)->Arg(10000);

static void BM_Optional_SuccessPath(benchmark::State& state) {
    std::vector<std::string> inputs;
    for (int i = 0; i < state.range(0); ++i) {
        inputs.push_back("valid_" + std::to_string(i));
    }

    for (auto _ : state) {
        int sum = 0;
        for (const auto& input : inputs) {
            sum += process_with_optional(input);
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_Optional_SuccessPath)->Arg(100)->Arg(1000)->Arg(10000);

// ============================================================================
// Benchmarks - Error Path (with errors)
// ============================================================================

static void BM_Exceptions_ErrorPath(benchmark::State& state) {
    std::vector<std::string> inputs;
    for (int i = 0; i < state.range(0); ++i) {
        // Every 10th input is invalid
        if (i % 10 == 0) {
            inputs.push_back("x_invalid");
        } else {
            inputs.push_back("valid_" + std::to_string(i));
        }
    }

    for (auto _ : state) {
        int sum = 0;
        for (const auto& input : inputs) {
            sum += process_with_exceptions(input);
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_Exceptions_ErrorPath)->Arg(100)->Arg(1000)->Arg(10000);

static void BM_ErrorCodes_ErrorPath(benchmark::State& state) {
    std::vector<std::string> inputs;
    for (int i = 0; i < state.range(0); ++i) {
        if (i % 10 == 0) {
            inputs.push_back("x_invalid");
        } else {
            inputs.push_back("valid_" + std::to_string(i));
        }
    }

    for (auto _ : state) {
        int sum = 0;
        for (const auto& input : inputs) {
            sum += process_with_error_codes(input);
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_ErrorCodes_ErrorPath)->Arg(100)->Arg(1000)->Arg(10000);

static void BM_Result_ErrorPath(benchmark::State& state) {
    std::vector<std::string> inputs;
    for (int i = 0; i < state.range(0); ++i) {
        if (i % 10 == 0) {
            inputs.push_back("x_invalid");
        } else {
            inputs.push_back("valid_" + std::to_string(i));
        }
    }

    for (auto _ : state) {
        int sum = 0;
        for (const auto& input : inputs) {
            sum += process_with_result(input);
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_Result_ErrorPath)->Arg(100)->Arg(1000)->Arg(10000);

static void BM_Optional_ErrorPath(benchmark::State& state) {
    std::vector<std::string> inputs;
    for (int i = 0; i < state.range(0); ++i) {
        if (i % 10 == 0) {
            inputs.push_back("x_invalid");
        } else {
            inputs.push_back("valid_" + std::to_string(i));
        }
    }

    for (auto _ : state) {
        int sum = 0;
        for (const auto& input : inputs) {
            sum += process_with_optional(input);
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_Optional_ErrorPath)->Arg(100)->Arg(1000)->Arg(10000);

BENCHMARK_MAIN();
