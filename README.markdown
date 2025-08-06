# Coding Challenge Solution

## Table of Contents

- [Development and Testing Environment](#development-and-testing-environment)
- [Project Setup](#project-setup)
- [Cache-Aware Methods Implemented](#cache-aware-methods-implemented)
- [Benchmark Results and Assembly Codes](#benchmark-results-and-assembly-codes)
- [Observations](#observations)

## Development and Testing Environment

The results in this submission were collected on my laptop running `Ubuntu 24.04.2 LTS on WSL2`(GNU/Linux 6.6.87.2-microsoft-standard-WSL2 x86_64) with AMD Ryzen 7 5800HS processor(`Zen 3 Architecture`, `8 cores`, `16 threads`).

I have used the `g++` compiler. I used `-O2 -march=znver3` flag for non-vectorized optimization and occasionally added `-ftree-vectorize` flag for vectorized optimization. I have used -march=znver3 because of my processor's architecture. `-march=native` can be used alternatively if the user's processor architecture differs.

The benchmarks in this repository were written using `Google Benchmark` library. For simplicity, all benchmarks assume square matrix of dimension N x N, where N is 240, 1200, and 1680.

I have used the `perf` tool to gain insights on the compiled C++ program's underlying Assembly code.

## Project Setup

- ```bash
  git clone https://github.com/Aadarsh-Keshri/MatMul_Challenge.git
  cd MatMul_Challenge
  ```
- I have used g++ 13.3.0(gcc 13.3.0). GCC 10 or later should work.
- perf is typically included in packages like `linux-tools-common` or `linux-tools-generic`. If you are using a WSL2 distribution like me, this article is helpful: [Install perf in WSL2](https://www.arong-xu.com/en/posts/wsl2-install-perf-with-manual-compile/)
- Follow the [Google Benchmark](https://github.com/google/benchmark) README.md to install it.
- **Compilation Example**:
  ```bash
  g++ -O2 -march=native loop_tiling.cpp -lbenchmark -pthread -o loop_tiling_O2
  g++ -O2 -march=native -ftree-vectorize loop_tiling.cpp -lbenchmark -pthread -o loop_tiling_O2_vec
  ```
- **Running the programs:**
  ```bash
  ./loop_tiling_O2
  ./loop_tiling_O2_vec
  ```
- The perf tool has many profiling options. I have only used `record`/`report` for this challenge:
  ```bash
  perf record ./loop_tiling_O2 #Captures performance data while running the program.
  perf report #Analyzes and displays the recorded performance data
  ```

## Cache-Aware Methods Implemented

This section describes the matrix multiplication implementations and their cache-aware optimizations:

1. `naive.cpp`:
    - **Method**: Standard `ijk` loop order.
    - **Description**: Baseline implementation with no cache optimizations.

2. `naive_interchanged.cpp`:
    - **Method**: `ikj` loop order.
    - **Description**: Reorders loops to access `B` and `C` contiguously.

3. `loop_tiling.cpp`:
    - **Method**: `ijk` loop order with tiling (tile size 64).
    - **Description**: Divides matrices into 64x64 tiles to fit in L1 cache.

4. `loop_tiling_interchanged.cpp`:
    - **Method**: `ikj` loop order with tiling (tile size 64).
    - **Description**: Combines tiling with contiguous `B` and `C` access.

**Note**:
- All implementations use 1D arrays with 16-byte aligned memory (`aligned_alloc`) for cache and vectorization efficiency.  I chose this approach in every implementation because flattening 2D matrices into 1D arrays is more about how they are represented in memory than about the logic of matrix multiplication.

## Benchmark Results and Assembly Codes

This section presents benchmark results and assembly codes of the respective programs.

1. `naive.cpp`:
    - **Benchmark Result**:
        - Non-vectorized Optimization:
          ```bash
          ----------------------------------------------------------------------
          Benchmark                            Time             CPU   Iterations
          ----------------------------------------------------------------------
          BM_NaiveMatrixMultiply/240       23952 us        25377 us           28                                                                                                              
          BM_NaiveMatrixMultiply/1200    3499797 us      3685234 us            1
          BM_NaiveMatrixMultiply/1680   38709784 us     40778839 us            1
          ```
        - Vectorized Optimization:
          ```bash
          ----------------------------------------------------------------------
          Benchmark                            Time             CPU   Iterations
          ----------------------------------------------------------------------
          BM_NaiveMatrixMultiply/240       22826 us        23911 us           29                                                                                                              
          BM_NaiveMatrixMultiply/1200    3373193 us      3545894 us            1                                                                                                              
          BM_NaiveMatrixMultiply/1680   35434404 us     37306267 us            1
          ```
    - **Assembly Code**:
        - For Non-vectorized  Code:
          ```asm
           Samples: 165K of event 'cycles:Pu', 4000 Hz, Event count (approx.): 65182091607                                                                                                     
           matrixMultiply(double const*, double const*, double*, int)  /mnt/d/open_source/matmul_challenge/naive_O2 [Percent: local period]                                                    
             0.00 │      mov     %r9,%r14
             0.01 │      mov     %r10,%rax
                  │      nop
                  │60:   vmovsd  (%rax),%xmm0
             0.15 │      add     $0x8,%rax
             0.05 │      vmulsd  (%r14),%xmm0,%xmm0
            70.66 │      add     %rsi,%r14                                                                                                                                                     
             0.09 │      vaddsd  %xmm0,%xmm1,%xmm1
            19.12 │      vmovsd  %xmm1,(%r8)                                                                                                                                                   
             9.84 │      cmp     %rax,%rdi                                                                                                                                                     
             0.06 │    ↑ jne     60
                  │      lea     0x1(%rdx),%eax
          ```
        - For Vectorized Code:
          ```asm
           Samples: 61K of event 'cycles:Pu', 1500 Hz, Event count (approx.): 70090148966                                                                                                      
           matrixMultiply(double const*, double const*, double*, int)  /mnt/d/open_source/matmul_challenge/naive_O2_vec [Percent: local period]                                                
             0.01 │      mov     %r9,%r14
             0.01 │      mov     %r10,%rax
                  │      nop
                  │60:   vmovsd  (%rax),%xmm0
             0.16 │      add     $0x8,%rax
             0.04 │      vmulsd  (%r14),%xmm0,%xmm0
            70.84 │      add     %rsi,%r14                                                                                                                                                     
             0.06 │      vaddsd  %xmm0,%xmm1,%xmm1
            18.89 │      vmovsd  %xmm1,(%r8)                                                                                                                                                   
             9.94 │      cmp     %rax,%rdi                                                                                                                                                     
             0.04 │    ↑ jne     60
                  │      lea     0x1(%rdx),%eax
          ```

2. `naive_interchanged.cpp`:
    - **Benchmark Result**:
        - Non-vectorized Optimization:
          ```bash
          ----------------------------------------------------------------------------------
          Benchmark                                        Time             CPU   Iterations
          ----------------------------------------------------------------------------------
          BM_NaiveMatrixMultiplyInterchanged/240       13391 us        14058 us           49                                                                                                  
          BM_NaiveMatrixMultiplyInterchanged/1200    1716592 us      1801518 us            1
          BM_NaiveMatrixMultiplyInterchanged/1680    5085938 us      5338501 us            1
          ```
        - Vectorized Optimization:
          ```bash
          ----------------------------------------------------------------------------------
          Benchmark                                        Time             CPU   Iterations
          ----------------------------------------------------------------------------------
          BM_NaiveMatrixMultiplyInterchanged/240        4124 us         4306 us          185                                                                                                  
          BM_NaiveMatrixMultiplyInterchanged/1200     602828 us       629376 us            1
          BM_NaiveMatrixMultiplyInterchanged/1680    2279934 us      2379931 us            1
          ```
    - **Assembly Code**:
        - For Non-vectorized  Code:
          ```asm
           Samples: 12K of event 'cycles:Pu', 1500 Hz, Event count (approx.): 13296216903                                                                                                      
           matrixMultiplyInterchangedLoops(double const*, double const*, double*, int)  /mnt/d/open_source/matmul_challenge/naive_interchanged_O2 [Percent: local period]                      
           Percent│      cs          nopw 0x0(%rax,%rax,1)
             0.01 │50:   vmovsd      (%rax),%xmm1
             5.12 │      vmovsd      (%rcx),%xmm0                                                                                                                                              
             3.17 │      add         $0x8,%rax                                                                                                                                                 
             1.63 │      add         $0x8,%rdx                                                                                                                                                 
             1.26 │      vfmadd132sd -0x8(%rdx),%xmm1,%xmm0                                                                                                                                    
            40.96 │      vmovsd      %xmm0,-0x8(%rax)                                                                                                                                          
            39.90 │      cmp         %rsi,%rax                                                                                                                                                 
             7.81 │    ↑ jne         50                                                                                                                                                        
                  │      add         $0x8,%rcx
             0.11 │      add         %r8,%rdi
                  │      cmp         %rcx,%r10
          ```
        - For Vectorized Code:
          ```asm
           Samples: 6K of event 'cycles:Pu', 1500 Hz, Event count (approx.): 7119675800                                                                                                        
           matrixMultiplyInterchangedLoops(double const*, double const*, double*, int)  /mnt/d/open_source/matmul_challenge/naive_interchanged_O2_vec [Percent: local period]                  
             0.05 │       vbroadcastsd (%rdx),%ymm1
             0.05 │       mov          -0x40(%rsp),%r8
                  │       xor          %eax,%eax
                  │       data16       cs nopw 0x0(%rax,%rax,1)
             0.02 │       xchg         %ax,%ax
             0.03 │110:   vmovupd      (%rsi,%rax,1),%ymm0
            39.60 │       vfmadd213pd  (%rcx,%rax,1),%ymm1,%ymm0                                                                                                                               
            31.55 │       vmovupd      %ymm0,(%rcx,%rax,1)                                                                                                                                     
            21.36 │       add          $0x20,%rax                                                                                                                                              
             3.71 │       cmp          %rax,%r8                                                                                                                                                
             2.26 │     ↑ jne          110                                                                                                                                                     
                  │       mov          -0x34(%rsp),%r14d
          ```

3. `loop_tiling.cpp`:
    - **Benchmark Result**:
        - Non-vectorized Optimization:
          ```bash
          ---------------------------------------------------------------------------
          Benchmark                                 Time             CPU   Iterations
          ---------------------------------------------------------------------------
          BM_MatrixMultiplyLoopTiling/240        6947 us         7071 us           99                                                                                                         
          BM_MatrixMultiplyLoopTiling/1200     930696 us       947384 us            1
          BM_MatrixMultiplyLoopTiling/1680    2567038 us      2613017 us            1
          ```
        - Vectorized Optimization:
          ```bash
          ---------------------------------------------------------------------------
          Benchmark                                 Time             CPU   Iterations
          ---------------------------------------------------------------------------
          BM_MatrixMultiplyLoopTiling/240        6728 us         7079 us           98                                                                                                         
          BM_MatrixMultiplyLoopTiling/1200     899539 us       946426 us            1
          BM_MatrixMultiplyLoopTiling/1680    2408271 us      2605673 us            1
          ```
    - **Assembly Code**:
        - For Non-vectorized  Code:
          ```asm
           Samples: 6K of event 'cycles:Pu', 1500 Hz, Event count (approx.): 17063624376                                                                                                       
           matrixMultiplyLoopTiling(double const*, double const*, double*, int, int)  /mnt/d/open_source/matmul_challenge/loop_tiling_O2 [Percent: local period]                               
             0.16 │       nop
                  │150:   vmovsd  (%rax),%xmm0
             2.88 │       add     $0x8,%rax                                                                                                                                                    
             2.70 │       vmulsd  (%rdx),%xmm0,%xmm0                                                                                                                                           
             6.64 │       add     %r15,%rdx                                                                                                                                                    
             2.78 │       vaddsd  %xmm0,%xmm1,%xmm1                                                                                                                                            
            52.20 │       vmovsd  %xmm1,(%rcx)                                                                                                                                                 
            29.21 │       cmp     %rdi,%rax                                                                                                                                                    
             2.61 │     ↑ jne     150                                                                                                                                                          
                  │16c:   inc     %r8d
                  │       add     $0x8,%rbp
                  │       cmp     %r10d,%r8d
          ```
        - For Vectorized Code:
          ```asm
           Samples: 6K of event 'cycles:Pu', 1500 Hz, Event count (approx.): 16732036370                                                                                                       
           matrixMultiplyLoopTiling(double const*, double const*, double*, int, int)  /mnt/d/open_source/matmul_challenge/loop_tiling_O2_vec [Percent: local period]                           
             0.11 │       nop
                  │150:   vmovsd  (%rax),%xmm0
             2.97 │       add     $0x8,%rax                                                                                                                                                    
             2.44 │       vmulsd  (%rdx),%xmm0,%xmm0                                                                                                                                           
             6.55 │       add     %r15,%rdx                                                                                                                                                    
             3.31 │       vaddsd  %xmm0,%xmm1,%xmm1                                                                                                                                            
            52.95 │       vmovsd  %xmm1,(%rcx)                                                                                                                                                 
            28.41 │       cmp     %rdi,%rax                                                                                                                                                    
             2.55 │     ↑ jne     150                                                                                                                                                          
                  │16c:   inc     %r8d
                  │       add     $0x8,%rbp
                  │       cmp     %r10d,%r8d
          ```

4. `loop_tiling_interchanged.cpp`:
    - **Benchmark Result**:
        - Non-vectorized Optimization:
          ```bash
          ---------------------------------------------------------------------------------------
          Benchmark                                             Time             CPU   Iterations
          ---------------------------------------------------------------------------------------
          BM_MatrixMultiplyLoopInterchangedTiling/240        5400 us         5663 us          123                                                                                             
          BM_MatrixMultiplyLoopInterchangedTiling/1200     713246 us       748021 us            1
          BM_MatrixMultiplyLoopInterchangedTiling/1680    1962846 us      2058364 us            1
          ```
        - Vectorized Optimization:
          ```bash
          ---------------------------------------------------------------------------------------
          Benchmark                                             Time             CPU   Iterations
          ---------------------------------------------------------------------------------------
          BM_MatrixMultiplyLoopInterchangedTiling/240        2167 us         2286 us          307                                                                                             
          BM_MatrixMultiplyLoopInterchangedTiling/1200     310389 us       327376 us            2
          BM_MatrixMultiplyLoopInterchangedTiling/1680     965589 us      1018443 us            1
          ```
    - **Assembly Code**:
        - For Non-vectorized  Code:
          ```asm
           Samples: 5K of event 'cycles:Pu', 1500 Hz, Event count (approx.): 14034455489                                                                                                       
           matrixMultiplyLoopInterchangedTiling(double const*, double const*, double*, int, int)  /mnt/d/open_source/matmul_challenge/loop_tiling_interchanged_O2 [Percent: local period]      
           Percent│       xor         %eax,%eax
                  │       nop
                  │120:   vmovsd      (%rdx,%rax,1),%xmm1
             0.84 │       vmovsd      (%rsi),%xmm0                                                                                                                                             
             8.18 │       vfmadd132sd (%rcx,%rax,1),%xmm1,%xmm0                                                                                                                                
            33.72 │       vmovsd      %xmm0,(%rdx,%rax,1)                                                                                                                                      
            39.87 │       add         $0x8,%rax                                                                                                                                                
             0.02 │       cmp         %rax,%rdi
            16.03 │     ↑ jne         120                                                                                                                                                      
                  │13d:   inc         %r9d
                  │       add         %r15d,%r10d
             0.06 │       cmp         %r9d,%r14d
          ```
        - For Vectorized Code:
          ```asm
           Samples: 4K of event 'cycles:Pu', 1500 Hz, Event count (approx.): 11793887329                                                                                                       
           matrixMultiplyLoopInterchangedTiling(double const*, double const*, double*, int, int)  /mnt/d/open_source/matmul_challenge/loop_tiling_interchanged_O2_vec [Percent: local period]  
             0.05 │       vbroadcastsd (%rcx),%ymm1
             1.69 │       mov          -0x18(%rsp),%r11                                                                                                                                        
             0.05 │       lea          -0x8(%r14,%r10,1),%r10
             1.08 │       xor          %edx,%edx                                                                                                                                               
                  │       nop
             0.11 │230:   vmovupd      (%r10,%rdx,1),%ymm0
            33.58 │       vfmadd213pd  (%rax,%rdx,1),%ymm1,%ymm0                                                                                                                               
            32.67 │       vmovupd      %ymm0,(%rax,%rdx,1)                                                                                                                                     
            16.04 │       add          $0x20,%rdx                                                                                                                                              
             3.16 │       cmp          %rdx,%r11                                                                                                                                               
             1.63 │     ↑ jne          230                                                                                                                                                     
                  │       mov          -0x1c(%rsp),%edx
          ```

## Observations

- The table below presents the percentage decrease in CPU time(or performance gain) taking naive.cpp with non-vectorized compiler optimization as the baseline:

  | Program                        | Compiler Optimization | Performance Gain |
  |--------------------------------|-----------------------|------------------|
  | `naive.cpp`                    | `Non-vectorized`      | _                |
  | `naive.cpp`                    | `Vectorized`          | 4%  to 8.5%      |
  | `naive_interchanged.cpp`       | `Non-vectorized`      | 45% to 87%       |
  | `naive_interchanged.cpp`       | `Vectorized`          | 83% to 94%       |
  | `loop_tiling.cpp`              | `Non-vectorized`      | 72% to 93.6%     |
  | `loop_tiling.cpp`              | `Vectorized`          | 72% to 93.6%     |
  | `loop_tiling_interchanged.cpp` | `Non-vectorized`      | 78% to 95%       |
  | `loop_tiling_interchanged.cpp` | `Vectorized`          | 91% to 97.5%     |
- We observe different assembly code for non-vectorized and vectorized programs when the `ikj` loop order is used. However, both produce the same assembly code when using the `ijk` loop order.
- As observed from the assembly code of each program, the majority of the time is spent in the instructions within the loop.
- The perf tool helped me identify an inefficient matrix C initialization in my programs which was adding unnecessary noise in the benchmark results. I fixed this in Commit [560b558](https://github.com/Aadarsh-Keshri/MatMul_Challenge/commit/560b5585904655427e8e570ce5124e4df2caf546).

