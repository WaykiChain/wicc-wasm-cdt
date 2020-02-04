### Building your first smart contract

In `hello.hpp`:

```
#include <wasm.hpp>
#include <string>
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

- Navigate to the hello folder in examples (./examples/hello).
- With CMake

```sh
$ mkdir build
$ cd build
$ cmake ..
$ make
```
This will generate two files (./examples/hello/build/hello):
* The compiled binary wasm (hello.wasm)
* The generated ABI file (hello.abi)
