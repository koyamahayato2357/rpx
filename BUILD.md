# make
Default parallel build and cache mechanism for high speed
- release: `make run OL=3`
- benchmark: `make run T=bench OL=<as you liking>`

# zig
Modern, maintainable and simple
- release: `zig build run --release=fast`
- benchmark: `zig build run -DT=bench --release=<as you liking>`

Binary size is smaller in make than in zig.
Benchmark results are better with zig than make.
Cause is under investigation.
