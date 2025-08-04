#include <benchmark/benchmark.h>
#include <memory>
#include <cstdlib>
#include <random>
#include <algorithm>

void matrixMultiplyInterchangedLoops(const double* A, const double* B, double* C, const int n) {
    // Perform matrix multiplication with ikj loop order
    for (int i = 0; i < n; ++i) {
        for (int k = 0; k < n; ++k) {
            for (int j = 0; j < n; ++j) {
                C[i * n + j] += A[i * n + k] * B[k * n + j];
            }
        }
    }
}

// Function to initialize a 1D matrix with random values
void initializeMatrix(double* matrix, int n) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 10.0);
    for (int i = 0; i < n * n; ++i) {
        matrix[i] = dis(gen);
    }
}

// Benchmark function for loop reordered matrix multiplication
static void BM_NaiveMatrixMultiplyInterchanged(benchmark::State& state) {
    const int n = state.range(0);
    // Allocate aligned memory for matrices A, B, and C (16-byte alignment for vectorization/cache)
    auto A = std::unique_ptr<double[]>(static_cast<double*>(aligned_alloc(16, n * n * sizeof(double))));
    auto B = std::unique_ptr<double[]>(static_cast<double*>(aligned_alloc(16, n * n * sizeof(double))));
    auto C = std::unique_ptr<double[]>(static_cast<double*>(aligned_alloc(16, n * n * sizeof(double))));

    // Check for allocation failure
    if (!A || !B || !C) {
        state.SkipWithError("Memory allocation failed");
        return;
    }

    // Initialize matrices with random values
    initializeMatrix(A.get(), n);
    initializeMatrix(B.get(), n);

    // Benchmark loop
    for (auto _ : state) {
        std::fill_n(C.get(), n * n, 0.0); // Zero C before multiplication
        matrixMultiplyInterchangedLoops(A.get(), B.get(), C.get(), n);
    }
}

// Register benchmarks for matrix sizes 240, 1200, and 1680
BENCHMARK(BM_NaiveMatrixMultiplyInterchanged)
        ->Arg(240)
        ->Arg(1200)
        ->Arg(1680)
        ->Unit(benchmark::kMicrosecond);

BENCHMARK_MAIN();