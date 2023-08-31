## Build
```shell
cmake -S . -B ./build && cmake --build build/
```
## Test
```shell
ctest --test-dir ./build
```