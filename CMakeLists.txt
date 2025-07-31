#Google's benchmark library is required to be included in this file but....
#I am directly compiling the programs using g++ commands in the terminal.
#Hence, this file is just for the sake of Clion's tracking.
cmake_minimum_required(VERSION 3.28)
project(matmul_challenge)

set(CMAKE_CXX_STANDARD 20)

add_executable(matmul_challenge naive.cpp
        naive_interchanged.cpp
        loop_tiling.cpp
        loop_tiling_interchanged.cpp
)
