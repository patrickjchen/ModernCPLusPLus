#include <benchmark/benchmark.h>
#include <variant>
#include <vector>
#include <memory>

// ============================================================================
// Virtual Function Approach (Runtime Polymorphism with vtable)
// ============================================================================

class MessageVirtual {
public:
    virtual ~MessageVirtual() = default;
    virtual void process() = 0;
    virtual int getValue() const = 0;
};

class TradeMessage : public MessageVirtual {
    int quantity_;
    double price_;
public:
    TradeMessage(int qty, double price) : quantity_(qty), price_(price) {}

    void process() override {
        // Simulate some processing
        quantity_ *= 2;
    }

    int getValue() const override {
        return quantity_;
    }
};

class QuoteMessage : public MessageVirtual {
    double bid_;
    double ask_;
public:
    QuoteMessage(double bid, double ask) : bid_(bid), ask_(ask) {}

    void process() override {
        // Simulate some processing
        bid_ += 0.01;
        ask_ += 0.01;
    }

    int getValue() const override {
        return static_cast<int>(bid_ * 100);
    }
};

class CancelMessage : public MessageVirtual {
    int order_id_;
public:
    explicit CancelMessage(int id) : order_id_(id) {}

    void process() override {
        // Simulate some processing
        order_id_ = -order_id_;
    }

    int getValue() const override {
        return order_id_;
    }
};

// ============================================================================
// std::variant + std::visit Approach (Type-safe, no vtable)
// ============================================================================

struct TradeMsg {
    int quantity;
    double price;

    void process() {
        quantity *= 2;
    }

    int getValue() const {
        return quantity;
    }
};

struct QuoteMsg {
    double bid;
    double ask;

    void process() {
        bid += 0.01;
        ask += 0.01;
    }

    int getValue() const {
        return static_cast<int>(bid * 100);
    }
};

struct CancelMsg {
    int order_id;

    void process() {
        order_id = -order_id;
    }

    int getValue() const {
        return order_id;
    }
};

using Message = std::variant<TradeMsg, QuoteMsg, CancelMsg>;

// Visitor for processing
struct ProcessVisitor {
    void operator()(TradeMsg& msg) { msg.process(); }
    void operator()(QuoteMsg& msg) { msg.process(); }
    void operator()(CancelMsg& msg) { msg.process(); }
};

// Visitor for getting value
struct GetValueVisitor {
    int operator()(const TradeMsg& msg) { return msg.getValue(); }
    int operator()(const QuoteMsg& msg) { return msg.getValue(); }
    int operator()(const CancelMsg& msg) { return msg.getValue(); }
};

// ============================================================================
// Benchmarks
// ============================================================================

static void BM_VirtualFunctions(benchmark::State& state) {
    std::vector<std::unique_ptr<MessageVirtual>> messages;

    // Create mixed messages
    for (int i = 0; i < state.range(0); ++i) {
        switch (i % 3) {
            case 0: messages.push_back(std::make_unique<TradeMessage>(100 + i, 50.0 + i)); break;
            case 1: messages.push_back(std::make_unique<QuoteMessage>(100.0 + i, 101.0 + i)); break;
            case 2: messages.push_back(std::make_unique<CancelMessage>(i)); break;
        }
    }

    for (auto _ : state) {
        int sum = 0;
        for (auto& msg : messages) {
            msg->process();
            sum += msg->getValue();
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_VirtualFunctions)->Arg(100)->Arg(1000)->Arg(10000);

static void BM_VariantVisit(benchmark::State& state) {
    std::vector<Message> messages;

    // Create mixed messages
    for (int i = 0; i < state.range(0); ++i) {
        switch (i % 3) {
            case 0: messages.push_back(TradeMsg{100 + i, 50.0 + static_cast<double>(i)}); break;
            case 1: messages.push_back(QuoteMsg{100.0 + i, 101.0 + i}); break;
            case 2: messages.push_back(CancelMsg{i}); break;
        }
    }

    for (auto _ : state) {
        int sum = 0;
        for (auto& msg : messages) {
            std::visit(ProcessVisitor{}, msg);
            sum += std::visit(GetValueVisitor{}, msg);
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_VariantVisit)->Arg(100)->Arg(1000)->Arg(10000);

BENCHMARK_MAIN();
