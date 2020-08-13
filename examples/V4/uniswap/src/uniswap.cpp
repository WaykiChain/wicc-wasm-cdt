#include <uniswap.hpp>

using namespace wasm;

static constexpr uint64_t MINIMUM_LIQUIDITY = 10*10*10;
static constexpr int128_t PRECISION         = 100000000;           


#define __INLINE_TRX_NAME(LBL, SUF) LBL##SUF
#define _INLINE_TRX_NAME(LBL, SUF) __INLINE_TRX_NAME(LBL, SUF)

#define TO_ASSET( amount, code ) \
    asset(amount, symbol(code, 8))

#define BALANCE_OF(token, owner, sym) \
    get_return_wasm<asset>(wasm::transaction{token, name("balance_of"), std::vector<permission>{{get_self(), name("wasmio_code")}}, std::tuple<regid, class symbol>(owner, sym)}.call())

#define MINT(token, to, quantity) \
    wasm::transaction inline_trx(token, name("mint"), std::vector<permission>{{get_self(), name("wasmio_code")}}, std::tuple<regid, asset, string>(to, quantity, "mint"));\
    inline_trx.call();

#define BURN(token, from, quantity) \
    wasm::transaction inline_trx(token, name("burn"), std::vector<permission>{{get_self(), name("wasmio_code")}}, std::tuple<regid, asset, string>(from, quantity, "BURN"));\
    inline_trx.call();


uint128_t divide_decimal(uint128_t a, uint128_t b){
    return a / b;
}

uint128_t multiply_decimal(uint128_t a, uint128_t b){
    return a * b;
}

inline uint128_t sqrt(uint128_t y) {
    if(y > 3) {
       uint128_t z = y;
       uint128_t x = y / 2 + 1;
       while(x < z){
          z = x;
          x = (y / x + x) / 2;
       }
       return z;
    } else if(y != 0) {
       return 1;
    }

    return 0;
}

void transfer(regid token, regid from, regid to, asset quantity){
    wasm::transaction inline_trx(token, name("transfer"), std::vector<permission>{{from, name("wasmio_code")}}, std::tuple<regid, regid, asset, string>(from, to, quantity, "from"));
    inline_trx.call();
}

template<typename T>
T get_return_wasm(uint64_t size){
    T return_value = wasm::unpack<T>(get_return(size));
    return return_value;
}

void uniswap::update(asset balance0, asset balance1)
{
    WASM_LOG_PRINT(true,
      " balance0:", balance0,
      " balance1:", balance1,
      "\n")

    market_t market(get_self().value);
    check(wasm::db::get(market), "market already exist");

    uint64_t block_timestamp = current_block_time() / 3;
    uint64_t time_elapsed    = block_timestamp - market.block_timestamp_last;

    if(time_elapsed > 0 && market.reserve0.amount != 0 && market.reserve1.amount != 0){
        market.price0_cumulative_last += divide_decimal(multiply_decimal(market.reserve0.amount, PRECISION), market.reserve1.amount) * time_elapsed;
        market.price1_cumulative_last += divide_decimal(multiply_decimal(market.reserve1.amount, PRECISION), market.reserve0.amount) * time_elapsed;
    }

    market.reserve0             = balance0;
    market.reserve1             = balance1;   
    market.block_timestamp_last = block_timestamp;

    wasm::db::set(market);
}

ACTION uniswap::init(regid token0, regid token1, symbol symbol0, symbol symbol1, symbol supply_symbol, regid token_contract)
{
    require_auth( get_maintainer(get_self()));
    check( symbol0.raw() < symbol1.raw(), "symbol1 must be > symbol0" );

    market_t market(get_self().value);
    check(!wasm::db::get(market), "market already exist");

    market.token0               = token0;
    market.token1               = token1;

    market.reserve0             = asset(0, symbol0);
    market.reserve1             = asset(0, symbol1);
    market.block_timestamp_last = 0;

    market.token_contract       = token_contract;
    market.total_supply         = asset(0, supply_symbol);
    market.closed               = false;

    wasm::db::set(market);
}

ACTION uniswap::mint(regid to)
{
    require_auth( to );
    market_t market(get_self().value);
    check(wasm::db::get(market), "market does not exist");
    check(!market.closed, "market has been closed");  

    asset balance0 = BALANCE_OF(market.token0, get_self(), market.reserve0.symbol);
    asset balance1 = BALANCE_OF(market.token1, get_self(), market.reserve1.symbol);

    asset amount0 = balance0 - market.reserve0;
    asset amount1 = balance1 - market.reserve1;

    WASM_LOG_PRINT(true,
      " balance0:", balance0,
      " balance1:", balance1,     
      " amount0:", amount0,
      " amount1:", amount1,
      "\n")

    int64_t liquidity;
    if(market.total_supply.amount == 0){

        //uint128_t total = multiply_decimal(amount0.amount, amount1.amount);
        liquidity = sqrt(multiply_decimal(amount0.amount, amount1.amount)) - MINIMUM_LIQUIDITY * PRECISION;

        MINT(market.token_contract, get_self(), asset(MINIMUM_LIQUIDITY * PRECISION, market.total_supply.symbol));     
        market.total_supply = asset(MINIMUM_LIQUIDITY * PRECISION, market.total_supply.symbol);   

    }else{

        WASM_LOG_PRINT(true,
          " liquidity0:", divide_decimal(multiply_decimal(amount0.amount, market.total_supply.amount), market.reserve0.amount),
          " liquidity1:", divide_decimal(multiply_decimal(amount1.amount, market.total_supply.amount), market.reserve1.amount),     
          "\n")

        liquidity = min(divide_decimal(multiply_decimal(amount0.amount, market.total_supply.amount), market.reserve0.amount), divide_decimal(multiply_decimal(amount1.amount, market.total_supply.amount), market.reserve1.amount));
    }

    check( liquidity > 0, "insufficient liquidity minted" );
    MINT(market.token_contract, to, asset(liquidity, market.total_supply.symbol));

    market.total_supply += asset(liquidity, market.total_supply.symbol);
    wasm::db::set(market);

    update(balance0, balance1);

    WASM_LOG_PRINT(true,
      " to:", to,
      " liquidity:", asset(liquidity, market.total_supply.symbol),
      " total_supply:", market.total_supply,
      " balance0:", balance0,
      " balance1:", balance1,
      "\n")

    set_return(wasm::pack<asset>(asset(liquidity, market.total_supply.symbol)));
}

ACTION uniswap::burn(regid to)
{
    require_auth( to );
    market_t market(get_self().value);
    check(wasm::db::get(market), "market does not exist");
    check(!market.closed, "market has been closed");    

    asset balance0 = BALANCE_OF(market.token0, get_self(), market.reserve0.symbol);
    asset balance1 = BALANCE_OF(market.token1, get_self(), market.reserve1.symbol);

    asset liquidity;
    liquidity = BALANCE_OF(market.token_contract, to, market.total_supply.symbol);

    // asset amount0 = balance0 * (liquidity * PRECISION / market.total_supply) / PRECISION;
    // asset amount1 = balance1 * (liquidity * PRECISION / market.total_supply) / PRECISION;
    asset amount0 = asset(divide_decimal(multiply_decimal(balance0.amount, divide_decimal(multiply_decimal(liquidity.amount, PRECISION), market.total_supply.amount)), PRECISION), balance0.symbol);
    asset amount1 = asset(divide_decimal(multiply_decimal(balance1.amount, divide_decimal(multiply_decimal(liquidity.amount, PRECISION), market.total_supply.amount)), PRECISION), balance1.symbol);

     WASM_LOG_PRINT(true,
      " to:", to,
      " liquidity:", liquidity,
      " balance0:", balance0,
      " balance1:", balance1,
      " total_supply:", market.total_supply,
      " amount0:", amount0,
      " amount1:", amount1,
      "\n")      

    BURN(market.token_contract, to, liquidity)

    transfer(market.token0, get_self(), to, amount0);
    transfer(market.token1, get_self(), to, amount1);

    balance0 = BALANCE_OF(market.token0, get_self(), market.reserve0.symbol);
    balance1 = BALANCE_OF(market.token1, get_self(), market.reserve1.symbol);

    market.total_supply = market.total_supply - liquidity;
    wasm::db::set(market);

    update(balance0, balance1);

     WASM_LOG_PRINT(true,
      " to:", to,
      " liquidity:", liquidity,
      " balance0:", balance0,
      " balance1:", balance1,
      " total_supply:", market.total_supply,
      "\n") 

    set_return(wasm::pack<asset>(liquidity));
}

ACTION uniswap::swap(asset amount0_out, asset amount1_out, regid to)
{
    require_auth( to );
    check(amount0_out > 0 || amount1_out > 0, "insufficient output amount");

    market_t market(get_self().value);
    check(wasm::db::get(market), "market does not exist");
    check(!market.closed , "market has been closed");  

    check(amount0_out < market.reserve0  || amount1_out < market.reserve1, "insufficient liquidity");

    WASM_LOG_PRINT(true,
      " to:", to,
      " market.reserve0 :", market.reserve0,
      " market.reserve1:", market.reserve1,
      "\n") 

    asset balance0, balance1;
    {
        check(market.token0 != to  && market.token1 != to, "invalid to");
        if(amount0_out > 0) transfer(market.token0, get_self(), to, amount0_out);
        if(amount1_out > 0) transfer(market.token1, get_self(), to, amount1_out);
        balance0 = BALANCE_OF(market.token0, get_self(), market.reserve0.symbol);
        balance1 = BALANCE_OF(market.token1, get_self(), market.reserve1.symbol);
    }

    asset amount0_in = balance0 > market.reserve0 - amount0_out ? balance0 - market.reserve0 - amount0_out : asset(0, market.reserve0.symbol);
    asset amount1_in = balance1 > market.reserve1 - amount1_out ? balance1 - market.reserve1 - amount1_out : asset(0, market.reserve1.symbol);    
    check(amount0_in > 0 || amount0_in > 0, "insufficient input amount");
    {
        asset balance0_adjusted = balance0 * 1000 - amount0_in * 3;
        asset balance1_adjusted = balance1 * 1000 - amount1_in * 3;

        check(multiply_decimal(balance0_adjusted.amount, balance1_adjusted.amount) >= multiply_decimal(market.reserve0.amount, market.reserve1.amount) * 1000 * 1000, "invalid amount in");
    }

     WASM_LOG_PRINT(true,
      " to:", to,
      " amount0_in:", amount0_in,
      " amount1_in:", amount1_in,
      " amount0_out:", amount0_out,
      " amount1_out:", amount1_out,
      " balance0:", balance0,
      " balance1:", balance1,
      "\n") 

    update(balance0, balance1);
}

ACTION uniswap::skim(regid to)
{
    require_auth( get_maintainer(get_self()));

    market_t market(get_self().value);
    check(wasm::db::get(market), "market does not exist");

    transfer(market.token0, get_self(), to, BALANCE_OF(market.token0, to, market.reserve0.symbol) - market.reserve0);
    transfer(market.token1, get_self(), to, BALANCE_OF(market.token1, to, market.reserve1.symbol) - market.reserve1);
}

ACTION uniswap::sync()
{
    require_auth( get_maintainer(get_self()));
    market_t market(get_self().value);
    check(wasm::db::get(market), "market does not exist");

    update(BALANCE_OF(market.token0, get_self(), market.reserve0.symbol), BALANCE_OF(market.token1, get_self(), market.reserve1.symbol));
}

ACTION uniswap::get_market(){

    market_t market(get_self().value);
    check(wasm::db::get(market), "market does not exist");

    set_return(wasm::pack<market_t>(market));
}

