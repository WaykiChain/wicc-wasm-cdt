#include <mem.hpp>

using namespace wasm;

ACTION mem::prints( uint32_t b1, uint32_t b2)
{
    balance1 = b1;
    balance2 = b2;
    print(balance1);
    print(balance2);
}

WASM_DISPATCH( mem, (prints))

