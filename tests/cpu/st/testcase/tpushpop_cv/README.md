# CPU Sim Cube-to-Vector TPUSH/TPOP Examples

This example shows how to use `TPUSH`/`TPOP` as a cube-to-vector FIFO handoff in CPU simulation:

- The producer side uses a `TileType::Mat` tile to model cube-produced data.
- The consumer side uses a `TileType::Vec` tile with the same logical shape to model vector-side consumption.
- The multicore example runs producer and consumer on separate CPU threads and pushes more tiles than the FIFO depth to exercise blocking and wraparound.

Build and run only this example:

```sh
cmake -S tests/cpu/st -B build/cpu-st-tpushpop-cv -DTEST_CASE=tpushpop_cv
cmake --build build/cpu-st-tpushpop-cv --target tpushpop_cv -j4
/Users/zhoubot/github/pto-isa/build/cpu-st-tpushpop-cv/bin/tpushpop_cv
```
