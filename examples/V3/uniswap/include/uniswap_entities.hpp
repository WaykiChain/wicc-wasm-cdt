#include <wasm.hpp>
#include <asset.hpp>
#include <inline_transaction.hpp>
#include <db2.hpp>

#include <string>
using namespace wasm;
using namespace wasm::db;

namespace wasm {

#define TABLE_IN_CONTRACT [[wasm::table, wasm::contract("uniswap")]]

struct TABLE_IN_CONTRACT market_t{
    uint128_t id;
    regid     token0;
    regid     token1;
    regid     liquidity_token;
    asset     liquidity_total_supply;
    asset     reserve0;
    asset     reserve1;
    time_point_sec  block_timestamp_last ;//reserved
    time_point_sec  skim_last_time       ;
    time_point_sec  sync_last_time       ;

    uint128_t  price0_cumulative_last = 0;
    uint128_t  price1_cumulative_last = 0;

    bool       closed                 = false;
    market_t(){}
    market_t(const uint128_t& i):id(i){}
    market_t(uint64_t s0, uint64_t s1):
        reserve0(asset(0,symbol(s0))),
        reserve1(asset(0,symbol(s1))){
            id = (uint128_t)reserve0.symbol.code().raw() << 64 | (uint128_t)reserve1.symbol.code().raw();
        }

    uint64_t get_symbol() const { return liquidity_total_supply.symbol.code().raw(); }
    uint64_t scope() const { return uint64_t(id >> 64); }
    uint128_t primary_key() const {  return id ; }
    std::string to_string() const {
        return wasm::to_string("id:% token0:% token1:% liquidity_token:% liquidity_total_supply:% reserve0:% reserve1:% closed:%", 
                                id, token0, token0, liquidity_token, liquidity_total_supply, reserve0, reserve0, closed);
    }   
    void print() const {wasm::print(to_string());}

     
    typedef index_table<"markets"_n, market_t, uint128_t, 
                        indexed_by<"by_symbol"_n, const_mem_fun<market_t, uint64_t, &wasm::market_t::get_symbol>>> table_t;
    WASM_DB_TABLE_TYPE_DEFINITION(table_t)

    WASMLIB_SERIALIZE( market_t, (id)(token0)(token1)(liquidity_token)(liquidity_total_supply)
                                 (reserve0)(reserve1)(block_timestamp_last)(skim_last_time)(sync_last_time)(closed)
                                 (price0_cumulative_last)(price1_cumulative_last))

};

struct TABLE_IN_CONTRACT mint_action_t {
    regid     owner;
    uint64_t  nonce;
    uint128_t market_id;
    regid     token0;
    regid     token1;
    asset     amount0_in;
    asset     amount1_in;

    time_point_sec create_time;
    bool           closed;

    mint_action_t(){}
    mint_action_t(uint64_t o, uint64_t n):owner(regid(o)),nonce(n){}
    uint64_t scope() const { return owner.value; }
    uint128_t primary_key()const { return (uint128_t)owner.value << 64 | (uint128_t)nonce; }
    std::string to_string() const {
        return wasm::to_string("owner:% nonce:% market_id:% token0:% token1:% amount0_in:% amount1_in:% closed:%", 
                        owner, nonce, market_id, token0, token1, amount0_in, amount1_in, closed);
    }
    void print() const {wasm::print(to_string());}

    typedef wasm::db::index_table<"mint_actions"_n, mint_action_t, uint128_t> table_t;
    WASM_DB_TABLE_TYPE_DEFINITION(table_t)

    WASMLIB_SERIALIZE( mint_action_t, (owner)(nonce)(market_id)
                                      (token0)(token1)(amount0_in)(amount1_in)
                                      (create_time)(closed))
};

struct TABLE_IN_CONTRACT account_t {
    regid    owner;
    asset    balance;

    account_t(){}
    account_t(uint64_t o, uint64_t s):owner(regid(o)),balance(asset(0,symbol(s))){}
    uint64_t scope() const { return owner.value; }
    uint128_t primary_key()const { return (uint128_t)owner.value << 64 | (uint128_t)balance.symbol.code().raw(); }

    std::string to_string() const{
        return wasm::to_string("owner:% asset:%", owner, balance);        
    }
    void print() const {wasm::print(to_string());}

    typedef wasm::db::index_table<"accounts"_n, account_t, uint128_t> table_t;
    WASM_DB_TABLE_TYPE_DEFINITION(table_t)

    WASMLIB_SERIALIZE( account_t, (owner)(balance))
};


}
