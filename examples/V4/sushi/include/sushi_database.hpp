#include <wasm.hpp>
#include <asset.hpp>
#include <db.hpp>

#include <string>
using namespace wasm;
using namespace wasm::db;

namespace wasm {

#define TABLE_IN_CONTRACT [[wasm::table, wasm::contract("sushi")]]

struct TABLE_IN_CONTRACT pool_info {
    //uint64_t pid;
    regid    lp_bank;
    uint64_t alloc_point;
    uint64_t last_reward_block;
    uint64_t acc_sushi_per_share;
    asset    total_liquidity;
    bool     closed = false;
}

struct TABLE_IN_CONTRACT sushi_t {
    regid id;

    regid migrator;

    std::vector<pool_info> pools;

    uint64_t bonus_end_block;
    uint64_t sushi_per_block;

    uint64_t total_alloc_point = 0;
    uint64_t start_block;

    regid  sushi_bank;
    symbol sushi_symbol;

    sushi_t(){}
    sushi_t(uint64_t r):id(regid(r)){}
    uint64_t scope() const { return id.value; }
    uint64_t primary_key() const { return id.value; }

    void   print() const {wasm::print(to_string());}
    string to_string() const {
        return wasm::to_string("id:% sushi:% gons_per_fragment:%", 
                                id, sushi, migrator);
    }   

    typedef index_table<"sushi"_n, sushi_t, uint64_t> table_t;
    WASM_DB_TABLE_TYPE_DEFINITION(table_t)

    WASMLIB_SERIALIZE( sushi_t, (id)(migrator)(pools)(bonus_end_block)(sushi_per_block)(total_alloc_point)(start_block)(sushi_bank)(sushi_total_supply))

};

struct TABLE_IN_CONTRACT account_t {
    regid    id;
    uint64_t pid;

    uint64_t amount;
    uint64_t reward_debt;

    account_t(){}
    account_t(uint64_t r, uint64_t p):id(regid(r)),pid(p){}
    uint64_t scope() const { return id.value; }
    uint128_t primary_key() const { return (uint128_t)id.value << 64 | pid; }

    void   print() const {wasm::print(to_string());}
    string to_string() const {
          return wasm::to_string("id:% pid:% amount:% reward_debt:%", id, amount, reward_debt);
    } 

    typedef index_table<"accounts"_n, account_t, uint128_t> table_t;
    WASM_DB_TABLE_TYPE_DEFINITION(table_t)

    WASMLIB_SERIALIZE( account_t, (id)(amount)(reward_debt))    

}

}