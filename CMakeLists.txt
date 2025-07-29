cmake_minimum_required(VERSION 3.28)
project(matmul_challenge)

set(CMAKE_CXX_STANDARD 20)

add_executable(matmul_challenge naive.cpp
        naive_interchanged.cpp
        loop_tiling.cpp
        loop_tiling_interchanged.cpp
)
