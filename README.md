# WASM.CDT (Contract Development Toolkit)
## Version : 1.0.0

WASM.CDT is a toolchain for WebAssembly (WASM) and set of tools to facilitate contract writing for the WASM platform.  In addition to being a general purpose WebAssembly toolchain, WASM.cdt specific optimizations are available to support building WASM smart contracts.  This new toolchain is built around Clang 7, which means that WASM.CDT has the most currently available optimizations and analyses from LLVM, but as the WASM target is still considered experimental, some optimizations are not available or incomplete.


### Guided Installation (Building from source code)
```sh
$ git clone --recursive git@github.com:WaykiChain/wicc-wasm-cdt.git
$ cd wasm.cdt
$ ./build.sh
$ sudo ./install.sh
```
### Build wasm from c++ project 
```sh
$ cd examples/hello
$ mkdir build
$ cd build
$ cmake ..
$ make
```

### Installed Tools
---
* wasm-cpp
* wasm-cc
* wasm-lld
* wasm2wast
* wast2wasm
* wasm-ranlib
* wasm-ar
