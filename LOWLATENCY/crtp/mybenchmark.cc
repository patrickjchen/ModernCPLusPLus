#include <benchmark/benchmark.h>
#include <vector>
#include <memory>

// ============================================================================
// Virtual Function Approach (Runtime Polymorphism)
// ============================================================================

class ShapeVirtual {
public:
    virtual ~ShapeVirtual() = default;
    virtual double area() const = 0;
    virtual double perimeter() const = 0;
};

class CircleVirtual : public ShapeVirtual {
private:
    double radius_;
public:
    explicit CircleVirtual(double r) : radius_(r) {}

    double area() const override {
        return 3.14159 * radius_ * radius_;
    }

    double perimeter() const override {
        return 2.0 * 3.14159 * radius_;
    }
};

class RectangleVirtual : public ShapeVirtual {
private:
    double width_, height_;
public:
    RectangleVirtual(double w, double h) : width_(w), height_(h) {}

    double area() const override {
        return width_ * height_;
    }

    double perimeter() const override {
        return 2.0 * (width_ + height_);
    }
};

// ============================================================================
// CRTP Approach (Compile-time Polymorphism)
// ============================================================================

template<typename Derived>
class ShapeCRTP {
public:
    double area() const {
        return static_cast<const Derived*>(this)->area_impl();
    }

    double perimeter() const {
        return static_cast<const Derived*>(this)->perimeter_impl();
    }
};

class CircleCRTP : public ShapeCRTP<CircleCRTP> {
private:
    double radius_;
public:
    explicit CircleCRTP(double r) : radius_(r) {}

    double area_impl() const {
        return 3.14159 * radius_ * radius_;
    }

    double perimeter_impl() const {
        return 2.0 * 3.14159 * radius_;
    }
};

class RectangleCRTP : public ShapeCRTP<RectangleCRTP> {
private:
    double width_, height_;
public:
    RectangleCRTP(double w, double h) : width_(w), height_(h) {}

    double area_impl() const {
        return width_ * height_;
    }

    double perimeter_impl() const {
        return 2.0 * (width_ + height_);
    }
};

// ============================================================================
// Benchmarks
// ============================================================================

// Benchmark Virtual Function approach
static void BM_VirtualFunctions(benchmark::State& state) {
    std::vector<std::unique_ptr<ShapeVirtual>> shapes;

    // Create mixed shapes
    for (int i = 0; i < state.range(0); ++i) {
        if (i % 2 == 0) {
            shapes.push_back(std::make_unique<CircleVirtual>(5.0 + i));
        } else {
            shapes.push_back(std::make_unique<RectangleVirtual>(4.0 + i, 3.0 + i));
        }
    }

    for (auto _ : state) {
        double total_area = 0.0;
        double total_perimeter = 0.0;

        for (const auto& shape : shapes) {
            total_area += shape->area();
            total_perimeter += shape->perimeter();
        }

        benchmark::DoNotOptimize(total_area);
        benchmark::DoNotOptimize(total_perimeter);
    }
}
BENCHMARK(BM_VirtualFunctions)->Arg(100)->Arg(1000)->Arg(10000);

// Benchmark CRTP approach with separate vectors (homogeneous containers)
static void BM_CRTP(benchmark::State& state) {
    std::vector<CircleCRTP> circles;
    std::vector<RectangleCRTP> rectangles;

    // Create shapes in separate containers
    int half = state.range(0) / 2;
    for (int i = 0; i < half; ++i) {
        circles.emplace_back(5.0 + i);
        rectangles.emplace_back(4.0 + i, 3.0 + i);
    }

    for (auto _ : state) {
        double total_area = 0.0;
        double total_perimeter = 0.0;

        for (const auto& circle : circles) {
            total_area += circle.area();
            total_perimeter += circle.perimeter();
        }

        for (const auto& rectangle : rectangles) {
            total_area += rectangle.area();
            total_perimeter += rectangle.perimeter();
        }

        benchmark::DoNotOptimize(total_area);
        benchmark::DoNotOptimize(total_perimeter);
    }
}
BENCHMARK(BM_CRTP)->Arg(100)->Arg(1000)->Arg(10000);

BENCHMARK_MAIN();
