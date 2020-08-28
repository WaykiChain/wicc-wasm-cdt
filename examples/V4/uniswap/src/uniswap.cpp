#include <uniswap.hpp>
#include <uniswap_utils.hpp>
#include <inline_transaction.hpp>
#include <uniswap_database.hpp>
#include <return.hpp>
#include <make_log.hpp>

using namespace wasm;

std::optional<market_t> global_market;

void uniswap::_update(asset balance0, asset balance1)
{
    WASM_LOG_FPRINT(UNISWAP_DEBUG, "balance0:% balance1:%", balance0, balance1)

    check(global_market.has_value(), "market does not exist");

    time_point_sec block_timestamp = time_point_sec(current_block_time()) ;
    uint64_t       time_elapsed    = (block_timestamp - global_market->block_timestamp_last).count();

    if(time_elapsed > 0 && global_market->reserve0.amount != 0 && global_market->reserve1.amount != 0){
        global_market->price0_cumulative_last += divide_decimal(multiply_decimal(global_market->reserve0.amount, PRECISION_1), global_market->reserve1.amount) * time_elapsed;
        global_market->price1_cumulative_last += divide_decimal(multiply_decimal(global_market->reserve1.amount, PRECISION_1), global_market->reserve0.amount) * time_elapsed;
    }

    global_market->reserve0             = balance0;
    global_market->reserve1             = balance1;   
    global_market->block_timestamp_last = block_timestamp;

}

ACTION uniswap::init(regid token0, regid token1, symbol symbol0, symbol symbol1, regid liquidity_token)
{
    require_auth( get_maintainer(get_self()));
    check( symbol0.raw() < symbol1.raw(), "symbol1 must be > symbol0" );

    check(!global_market.has_value(), "market already exist");

    market_t market(get_self().value);
    market.token0               = token0;
    market.token1               = token1;

    market.reserve0             = asset(0, symbol0);
    market.reserve1             = asset(0, symbol1);
    market.block_timestamp_last = 0;

    market.liquidity_token         = liquidity_token;
    market.liquidity_total_supply  = TO_ASSET(0, add_symbol(symbol0, symbol1));
    market.closed                  = false;

    global_market = market;

}

ACTION uniswap::mint(regid to)
{
    require_auth( to );

    check(global_market.has_value(), "market does not exist");    
    check(!global_market->closed, "market has been closed");  

    asset balance0 = BALANCE_OF(global_market->token0, get_self(), global_market->reserve0.symbol);
    asset balance1 = BALANCE_OF(global_market->token1, get_self(), global_market->reserve1.symbol);

    asset amount0 = balance0 - global_market->reserve0;
    asset amount1 = balance1 - global_market->reserve1;

    WASM_LOG_FPRINT(UNISWAP_DEBUG, "balance0:% balance1:% amount0:% amount1:%", balance0, balance1, amount0, amount1)

    int64_t liquidity;
    if(global_market->liquidity_total_supply.amount == 0){

        liquidity = sqrt(multiply_decimal(amount0.amount, amount1.amount)) - MINIMUM_LIQUIDITY * PRECISION_1;

        MINT(global_market->liquidity_token, get_self(), asset(MINIMUM_LIQUIDITY * PRECISION_1, global_market->liquidity_total_supply.symbol));     
        global_market->liquidity_total_supply = asset(MINIMUM_LIQUIDITY * PRECISION_1, global_market->liquidity_total_supply.symbol);   

    }else{

        WASM_LOG_FPRINT(UNISWAP_DEBUG,"liquidity0:% liquidity1:%", 
            divide_decimal(multiply_decimal(amount0.amount, global_market->liquidity_total_supply.amount), global_market->reserve0.amount),
            divide_decimal(multiply_decimal(amount1.amount, global_market->liquidity_total_supply.amount), global_market->reserve1.amount))

        liquidity = min(divide_decimal(multiply_decimal(amount0.amount, global_market->liquidity_total_supply.amount), global_market->reserve0.amount), divide_decimal(multiply_decimal(amount1.amount, global_market->liquidity_total_supply.amount), global_market->reserve1.amount));
    }

    check( liquidity > 0, "insufficient liquidity minted" );
    MINT(global_market->liquidity_token, to, asset(liquidity, global_market->liquidity_total_supply.symbol));

    global_market->liquidity_total_supply += asset(liquidity, global_market->liquidity_total_supply.symbol);

    _update(balance0, balance1);

    WASM_LOG_FPRINT(UNISWAP_DEBUG,"to:% liquidity:% liquidity_total_supply:% balance0:% balance1:%", 
        to, asset(liquidity, global_market->liquidity_total_supply.symbol), 
        global_market->liquidity_total_supply, balance0, balance1)

    set_return(wasm::pack<asset>(asset(liquidity, global_market->liquidity_total_supply.symbol)));
}

ACTION uniswap::burn(regid to)
{
    require_auth( to );

    check(global_market.has_value(), "market does not exist");    
    check(!global_market->closed, "market has been closed");     

    asset balance0 = BALANCE_OF(global_market->token0, get_self(), global_market->reserve0.symbol);
    asset balance1 = BALANCE_OF(global_market->token1, get_self(), global_market->reserve1.symbol);

    asset liquidity;
    liquidity = BALANCE_OF(global_market->liquidity_token, to, global_market->liquidity_total_supply.symbol);

    asset amount0 = asset(divide_decimal(multiply_decimal(balance0.amount, divide_decimal(multiply_decimal(liquidity.amount, PRECISION_1), global_market->liquidity_total_supply.amount)), PRECISION_1), balance0.symbol);
    asset amount1 = asset(divide_decimal(multiply_decimal(balance1.amount, divide_decimal(multiply_decimal(liquidity.amount, PRECISION_1), global_market->liquidity_total_supply.amount)), PRECISION_1), balance1.symbol);

    WASM_LOG_FPRINT(UNISWAP_DEBUG,"to:% liquidity:% liquidity_total_supply:% balance0:% balance1:% amount0:% amount1:%", 
        to, liquidity, global_market->liquidity_total_supply, balance0, balance1, amount0, amount1)     

    BURN(global_market->liquidity_token, to, liquidity)

    TRANSFER(global_market->token0, get_self(), to, amount0);
    TRANSFER(global_market->token1, get_self(), to, amount1);

    balance0 = BALANCE_OF(global_market->token0, get_self(), global_market->reserve0.symbol);
    balance1 = BALANCE_OF(global_market->token1, get_self(), global_market->reserve1.symbol);

    global_market->liquidity_total_supply = global_market->liquidity_total_supply - liquidity;

    _update(balance0, balance1);

    WASM_LOG_FPRINT(UNISWAP_DEBUG,"to:% liquidity:% liquidity_total_supply:% balance0:% balance1:% amount0:% amount1:%", 
        to, liquidity, global_market->liquidity_total_supply, balance0, balance1) 

    set_return(wasm::pack<asset>(liquidity));
}

ACTION uniswap::swap(asset amount0_out, asset amount1_out, regid to)
{
    require_auth( to );
    check(amount0_out > 0 || amount1_out > 0, "insufficient output amount");

    check(global_market.has_value(), "market does not exist");    
    check(!global_market->closed, "market has been closed");     

    check(amount0_out < global_market->reserve0  || amount1_out < global_market->reserve1, "insufficient liquidity");

    WASM_LOG_FPRINT(UNISWAP_DEBUG,"to:% reserve0:% reserve1:%", to, global_market->reserve0, global_market->reserve1) 


    asset balance0, balance1;
    {
        check(global_market->token0 != to  && global_market->token1 != to, "invalid to");
        if(amount0_out > 0) TRANSFER(global_market->token0, get_self(), to, amount0_out);
        if(amount1_out > 0) TRANSFER(global_market->token1, get_self(), to, amount1_out);
        balance0 = BALANCE_OF(global_market->token0, get_self(), global_market->reserve0.symbol);
        balance1 = BALANCE_OF(global_market->token1, get_self(), global_market->reserve1.symbol);
    }

    asset amount0_in = balance0 > global_market->reserve0 - amount0_out ? balance0 - global_market->reserve0 - amount0_out : asset(0, global_market->reserve0.symbol);
    asset amount1_in = balance1 > global_market->reserve1 - amount1_out ? balance1 - global_market->reserve1 - amount1_out : asset(0, global_market->reserve1.symbol);    
    check(amount0_in > 0 || amount0_in > 0, "insufficient input amount");
    {
        asset balance0_adjusted = balance0 * 1000 - amount0_in * 3;
        asset balance1_adjusted = balance1 * 1000 - amount1_in * 3;

        check(multiply_decimal(balance0_adjusted.amount, balance1_adjusted.amount) >= multiply_decimal(global_market->reserve0.amount, global_market->reserve1.amount) * 1000 * 1000, "invalid amount in");
    }

    WASM_LOG_FPRINT(UNISWAP_DEBUG,"to:% amount0_in:% amount1_in:% amount0_out:% amount1_out:% balance0:% balance1:%", 
        to, amount0_in, amount1_in, amount0_out, amount1_out, balance0, balance1) 

    _update(balance0, balance1);
}

ACTION uniswap::skim(regid to)
{
    require_auth( get_maintainer(get_self()));

    check(global_market.has_value(), "market does not exist");    

    TRANSFER(global_market->token0, get_self(), to, BALANCE_OF(global_market->token0, to, global_market->reserve0.symbol) - global_market->reserve0);
    TRANSFER(global_market->token1, get_self(), to, BALANCE_OF(global_market->token1, to, global_market->reserve1.symbol) - global_market->reserve1);
}

ACTION uniswap::sync()
{
    require_auth( get_maintainer(get_self()));

    check(global_market.has_value(), "market does not exist"); 
    _update(BALANCE_OF(global_market->token0, get_self(), global_market->reserve0.symbol), BALANCE_OF(global_market->token1, get_self(), global_market->reserve1.symbol));
}

ACTION uniswap::get_market(){

    check(global_market.has_value(), "market does not exist"); 

    WASM_LOG_FPRINT(true, "market:%", global_market.value());
    set_return(wasm::pack<market_t>(global_market.value()));
}

extern "C" bool pre_dispatch(regid self, regid original_receiver, name action) {
   market_t market(self.value);
   if(wasm::db::get(market)) global_market = market;

   return true;
}

extern "C" void post_dispatch(regid self, regid original_receiver, name action) {
  if(global_market.has_value()) wasm::db::set(global_market.value());

}


