#include <index.hpp>

using namespace wasm;

#define TO_ASSET( amount, code ) \
    asset(amount, symbol(code, 8))


ACTION index::add(uint64_t id, regid owner, asset balance)
{
    account a{id, owner, balance};
    wasm::db::set(a);
}

ACTION index::get( uint64_t id, regid owner, symbol_code code)
{
    account account1;
    account1.id = id;
    wasm::db::get(account1);
    WASM_LOG_PRINT(true,
      " id:",      account1.id,
      " owner:",   account1.owner,
      " balance:", account1.balance, "\n")

    account account2;
    account2.owner = owner;
    wasm::db::get(account2, wasm::name("by_owner").value);
    WASM_LOG_PRINT(true,
      " id:",      account2.id,
      " owner:",   account2.owner,
      " balance:", account2.balance, "\n")

    account account3;
    account3.balance = TO_ASSET(0, code);
    wasm::db::get(account3, wasm::name("by_code").value);
    WASM_LOG_PRINT(true,
      " id:",      account3.id,
      " owner:",   account3.owner,
      " balance:", account3.balance, "\n")
}

WASM_DISPATCH( index, (add)(get))
