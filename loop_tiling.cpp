#include <benchmark/benchmark.h>
#include <memory>
#include <cstdlib>
#include <random>
#include <algorithm>

// Function to perform tiled matrix multiplication using 1D arrays
void matrixMultiplyLoopTiling(const double* A, const double* B, double* C, int n, int tile_size) {
    // Initialize result matrix C to zero
    for (int i = 0; i < n * n; ++i) {
        C[i] = 0.0;
    }
    // Perform tiled matrix multiplication
    for (int ii = 0; ii < n; ii += tile_size) {
        for (int jj = 0; jj < n; jj += tile_size) {
            for (int kk = 0; kk < n; kk += tile_size) {
                for (int i = ii; i < std::min(ii + tile_size, n); ++i) {
                    for (int j = jj; j < std::min(jj + tile_size, n); ++j) {
                        for (int k = kk; k < std::min(kk + tile_size, n); ++k) {
                            C[i * n + j] += A[i * n + k] * B[k * n + j];
                        }
                    }
                }
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

// Benchmark function for tiled matrix multiplication
static void BM_MatrixMultiplyLoopTiling(benchmark::State& state) {
    const int n = state.range(0);
    const int tile_size = 64; // Fixed tile size, consistent with original
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
        matrixMultiplyLoopTiling(A.get(), B.get(), C.get(), n, tile_size);
    }

    // Report operations (2 * n^3 for multiply-add pairs)
    state.SetItemsProcessed(static_cast<int64_t>(2) * n * n * n * state.iterations());
    // Report memory usage in kilobytes (3 matrices * n * n * sizeof(double) / 1024)
    state.counters["MemoryKB"] = (3.0 * n * n * sizeof(double)) / 1024.0;
}

// Register benchmarks for matrix sizes 240, 1200, and 1680
BENCHMARK(BM_MatrixMultiplyLoopTiling)
        ->Arg(240)
        ->Arg(1200)
        ->Arg(1680)
        ->Unit(benchmark::kMicrosecond);

// Use Google Benchmark's main function
BENCHMARK_MAIN();