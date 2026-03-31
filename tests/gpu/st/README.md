# tests/gpu/st/

Standalone CUDA smoke / correctness tests for the PTO NVIDIA GPU backend.

## What it covers today

Current executable:

- `pto_gpu_core`

Current checks:

- `TLOAD` ND row-major path
- `TLOAD` DN col-major path
- `TSTORE` ND row-major path
- `TSTORE` DN col-major path
- `TADD` correctness against a host reference
- `sm121` `TMATMUL` fast-path smoke test against a host reference

## Build

```bash
cmake -S tests/gpu/st -B build/tests/gpu-st   -DCMAKE_CUDA_COMPILER=/usr/local/cuda/bin/nvcc   -DCMAKE_CUDA_ARCHITECTURES=121
cmake --build build/tests/gpu-st -j
```

## Run

```bash
cd build/tests/gpu-st
ctest --output-on-failure
```

## Notes

- This lane is intentionally lightweight and self-contained.
- It uses CTest directly instead of the repo's CPU/NPU test harnesses.
- The current `sm121` matmul fast path is an initial inline-PTX FMA specialization, not the final tensor-core path.
