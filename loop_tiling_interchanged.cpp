#include <benchmark/benchmark.h>
#include <memory>
#include <cstdlib>
#include <random>
#include <algorithm>

void matrixMultiplyLoopInterchangedTiling(const double* A, const double* B, double* C, const int n, const int tile_size) {
    // Perform tiled matrix multiplication with iikkjj and ikj loop orders
    for (int ii = 0; ii < n; ii += tile_size) {
        for (int kk = 0; kk < n; kk += tile_size) {
            for (int jj = 0; jj < n; jj += tile_size) {
                for (int i = ii; i < std::min(ii + tile_size, n); ++i) {
                    for (int k = kk; k < std::min(kk + tile_size, n); ++k) {
                        for (int j = jj; j < std::min(jj + tile_size, n); ++j) {
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

// Benchmark function for tiled with loop reorders matrix multiplication
static void BM_MatrixMultiplyLoopInterchangedTiling(benchmark::State& state) {
    const int n = state.range(0);
    const int tile_size = 64; // Tile size for the blocks
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
        matrixMultiplyLoopInterchangedTiling(A.get(), B.get(), C.get(), n, tile_size);
    }
}

// Register benchmarks for matrix sizes 240, 1200, and 1680
BENCHMARK(BM_MatrixMultiplyLoopInterchangedTiling)
        ->Arg(240)
        ->Arg(1200)
        ->Arg(1680)
        ->Unit(benchmark::kMicrosecond);

BENCHMARK_MAIN();
