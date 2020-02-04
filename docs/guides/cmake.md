# CMake

## CMake Configuration
The template `CMakeLists.txt` in the examples folder is a good boilerplate for manual usage.

For example:

In `CMakeLists.txt`:
```
include(ExternalProject)
# if no cdt root is given use default path
if(WASM_CDT_ROOT STREQUAL "" OR NOT WASM_CDT_ROOT)
   find_package(wasm.cdt)
endif()

ExternalProject_Add(
   hello_project
   SOURCE_DIR ${CMAKE_SOURCE_DIR}/src
   BINARY_DIR ${CMAKE_BINARY_DIR}/hello
   CMAKE_ARGS -DCMAKE_TOOLCHAIN_FILE=${WASM_CDT_ROOT}/lib/cmake/wasm.cdt/WasmWasmToolchain.cmake
   UPDATE_COMMAND ""
   PATCH_COMMAND ""
   TEST_COMMAND ""
   INSTALL_COMMAND ""
   BUILD_ALWAYS 1
)
```


In `hello.hpp`:

```
#include <wasm.hpp>
using namespace wasm;
using namespace std;

CONTRACT hello : public contract {
   public:
      using contract::contract;

      ACTION hi( name nm );

};

```

In `hello.cpp`:

```
#include <hello.hpp>

ACTION hello::hi( name nm ) {
   print_f("Name : %\n", nm);
}

WASM_DISPATCH( hello, (hi))
```

## source code files structure
- check folder examples/hello
