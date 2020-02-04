## CMake Configuration
The template `CMakeLists.txt` in the examples/hello folder is a good boilerplate for manual usage.

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

In `src/CMakeLists.txt`:
```
project(hello)

set(WASM_WASM_OLD_BEHAVIOR "Off")
find_package(wasm.cdt)

add_contract( hello hello hello.cpp )
target_include_directories( hello PUBLIC ${CMAKE_SOURCE_DIR}/../include )
target_ricardian_directory( hello ${CMAKE_SOURCE_DIR}/../ricardian )
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
