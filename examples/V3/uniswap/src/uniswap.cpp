#include <uniswap.hpp>
#include <wasm.hpp>
#include <wasm_log.hpp>

using namespace wasm;  


ACTION uniswap::create(regid token0, regid token1, symbol symbol0, symbol symbol1, regid liquidity_token){

    require_auth( get_maintainer(get_self()));
    check( symbol0.raw() < symbol1.raw(), uniswap_failed{}, "symbol0:% must be < symbol1:%", symbol0, symbol1);

    market_t market(symbol0.raw(), symbol1.raw());  

    string liquidity_symbol       = add_symbol(symbol0, symbol1);
    market.liquidity_total_supply = TO_ASSET(0, liquidity_symbol); 

    WASM_LOG_FPRINT(true, "token0:% token1:% symbol0:% symbol1:% liquidity_token:%", 
                          token0, token1, symbol0, symbol1, liquidity_token)

    check(!wasm::db::get(N(by_symbol), market), uniswap_failed{}, "market % with liquidity token symbol % already exist", market, liquidity_symbol );    
    check(!wasm::db::get(market), uniswap_failed{}, "market % with symbol pair %-% already exist", market, symbol0.code(), symbol1.code() );

    market.token0                 = token0;
    market.token1                 = token1;

    market.reserve0               = asset(0, symbol0);
    market.reserve1               = asset(0, symbol1);
    market.block_timestamp_last   = current_block_time();

    market.skim_last_time         = current_block_time();
    market.sync_last_time         = current_block_time();

    market.liquidity_token        = liquidity_token;
    market.liquidity_total_supply = TO_ASSET(0, add_symbol(symbol0, symbol1)); 
    market.closed                 = false;

    wasm::db::set(market);

    WASM_LOG_FPRINT(true, "market:%", market)

}

[[wasm::on_notify("*::transfer")]] 
void uniswap::on_transfer(regid from, regid to, asset quant, std::string memo) {

    if(to != get_self()) return;

    std:vector <string> transfer_memo = string_split(memo, ':');
    check(transfer_memo.size() > 3, uniswap_failed{}, "params size % must > 3", transfer_memo.size());
    uint64_t  action     = name(transfer_memo[0]).value;
    uint128_t market_id  = string_to_market_id(transfer_memo[1]);
    switch(action)
    {
        case N(mint)://mint:btc-eth:1:100(mint:btc-eth:2:100)
            _mint(market_id, atoi(transfer_memo[2].data()), from, quant, atoi(transfer_memo[3].data()));
            return;
        case N(swap)://swap:btc-eth:amount_out:to
            _swap(market_id, quant, asset::from_string(transfer_memo[2]), regid(transfer_memo[3]));
            return;
    }

}

void uniswap::_mint_liquidity(regid liquidity_token, regid to, asset quant){
    account_t owner(to.value, quant.symbol.raw());
    wasm::db::get(owner);
    owner.balance += quant;
    wasm::db::set(owner);
    MINT(liquidity_token, to, quant);    
}

void uniswap::_burn_liquidity(regid liquidity_token, regid from, asset quant){
    account_t owner(from.value, quant.symbol.raw());
    wasm::db::get(owner);
    owner.balance -= quant;
    wasm::db::set(owner);
    BURN(liquidity_token, from, quant);   
}

void uniswap::_mint(const uint128_t& market_id, uint64_t step, regid to, asset quant, uint64_t nonce)
{

    WASM_LOG_FPRINT(true, "market:% step:% to:% quant:% nonce:%", market_id, step, to ,quant, nonce)

    market_t market(market_id);
    check(wasm::db::get(market), uniswap_failed{}, "market % does not exist", market_id_to_string(market_id)); 
    check(!market.closed, uniswap_failed{}, "market % already closed", market_id_to_string(market_id));

    mint_action_t mint_action(to.value, nonce);
    if(step == UNISWAP_MINT_STEP1){
         check(quant.symbol         == market.reserve0.symbol, uniswap_failed{}, "step1 token0 symbol % and reserve0 symbol % mismatch", quant.symbol, market.reserve0.symbol);  
         check(get_first_receiver() == market.token0 , uniswap_failed{}, "step1 bank % and token0 % mismatch",regid(get_first_receiver()), regid(market.token0));

         check(!wasm::db::get(mint_action), uniswap_failed{}, "mint action %:% step1 already exist", to, nonce);  
         mint_action.nonce       = nonce;
         mint_action.market_id   = market_id;
         mint_action.token0      = get_first_receiver();
         mint_action.amount0_in  = quant;
         mint_action.closed      = false; 
         mint_action.create_time = current_block_time();
         wasm::db::set(mint_action); 
                     
    }else if (step == UNISWAP_MINT_STEP2){
        check(wasm::db::get(mint_action), uniswap_failed{}, "step2 mint action %:% does not exist", to, nonce);  
        check(mint_action.market_id == market_id, uniswap_failed{}, "market_id % of mint action is mismatch market_id % from param", mint_action.market_id, market_id); 
        //check(mint_action.create_time == time_point_sec(current_block_time()), uniswap_failed{}, "2 steps must be in one tx");  
        check(!mint_action.closed, uniswap_failed{}, "mint action %:% already closed", to, nonce); 
        check(get_first_receiver() == market.token1 , uniswap_failed{}, "step2 bankd % and token1 % mismatch",regid(get_first_receiver()), regid(market.token0));
        check(quant.symbol         == market.reserve1.symbol, uniswap_failed{}, "step2 token1 symbol % and reserve1 symbol % mismatch", quant.symbol, market.reserve1.symbol);

        mint_action.token1     = get_first_receiver();
        mint_action.amount1_in = quant;
        mint_action.closed     = true; 
        wasm::db::set(mint_action);
 
        WASM_LOG_FPRINT(true, "amount0:% amount1:% reserve0:% reserve1:%", 
                        mint_action.amount0_in, mint_action.amount1_in, market.reserve0 ,market.reserve1)

        int64_t liquidity;
        symbol liquidity_symbol = market.liquidity_total_supply.symbol;
        if(market.liquidity_total_supply.amount == 0){

            liquidity = sqrt(multiply_decimal(mint_action.amount0_in.amount, mint_action.amount1_in.amount)) - MINIMUM_LIQUIDITY * PRECISION_1;

            _mint_liquidity(market.liquidity_token, get_self(), asset(MINIMUM_LIQUIDITY * PRECISION_1, liquidity_symbol));
            market.liquidity_total_supply = asset(MINIMUM_LIQUIDITY * PRECISION_1, liquidity_symbol);   

        }else{
            liquidity = min(divide_decimal(multiply_decimal(mint_action.amount0_in.amount, market.liquidity_total_supply.amount), market.reserve0.amount), divide_decimal(multiply_decimal(mint_action.amount1_in.amount, market.liquidity_total_supply.amount), market.reserve1.amount));
        }

        WASM_LOG_FPRINT(true," liquidity:%", liquidity)  

        check( liquidity > 0, uniswap_failed{}, "insufficient liquidity % minted", liquidity );

        market.liquidity_total_supply += asset(liquidity, liquidity_symbol);
        market.reserve0                = market.reserve0 + mint_action.amount0_in;
        market.reserve1                = market.reserve1 + mint_action.amount1_in;   
        market.block_timestamp_last    = current_block_time();   
        wasm::db::set(market);

        _mint_liquidity(market.liquidity_token, to, asset(liquidity, liquidity_symbol));

    }

    WASM_LOG_FPRINT(true,"market:%", market)  
    WASM_LOG_FPRINT(true,"mint_action:%", mint_action)  

}

ACTION uniswap::burn(std::string market_id, regid to)
{
    require_auth( to );
    //check( symbol0.raw() < symbol1.raw(), "symbol1 must be > symbol0" );

    market_t market(string_to_market_id(market_id));
    check(wasm::db::get(market), uniswap_failed{}, "market does not exist");
    check(!market.closed, uniswap_failed{}, "market already closed");  

    asset balance0 = market.reserve0;
    asset balance1 = market.reserve1;   

    account_t owner(to.value, market.liquidity_total_supply.symbol.raw());
    check(wasm::db::get(owner), uniswap_failed{}, "owner does not exist");  
    
    asset liquidity =  owner.balance;

    asset amount0 = asset(divide_decimal(multiply_decimal(balance0.amount, divide_decimal(multiply_decimal(liquidity.amount, PRECISION_1), market.liquidity_total_supply.amount)), PRECISION_1), balance0.symbol);
    asset amount1 = asset(divide_decimal(multiply_decimal(balance1.amount, divide_decimal(multiply_decimal(liquidity.amount, PRECISION_1), market.liquidity_total_supply.amount)), PRECISION_1), balance1.symbol);

    _burn_liquidity(market.liquidity_token, to, liquidity);

    TRANSFER(market.token0, get_self(), to, amount0);
    TRANSFER(market.token1, get_self(), to, amount1);

    WASM_LOG_FPRINT(true,"amount0:% amount1:%", amount0, amount1)  

    market.liquidity_total_supply = market.liquidity_total_supply - liquidity;  
    
    market.reserve0 = balance0 - amount0;
    market.reserve1 = balance1 - amount1;  
    market.block_timestamp_last = current_block_time();   
    wasm::db::set(market);        
}

void uniswap::_swap(const uint128_t& market_id, asset quant, asset amount_out, regid to)
{

    //WASM_LOG_FPRINT(true,"market_id:% quant:% amount_out:% to:%", market_id, quant, amount_out, to)  

    check(quant > 0, uniswap_failed{}, "insufficient input amount");

    market_t market(market_id);
    check(wasm::db::get(market), uniswap_failed{}, "market does not exist"); 
    check(!market.closed, uniswap_failed{}, "market has been closed");

    asset amount0_out, amount1_out;
    asset amount0_in, amount1_in;

    //WASM_LOG_FPRINT(true,"quant:% amount_out:%", quant.symbol.raw(), amount_out.symbol.raw())
   
    if(quant.symbol.raw() < amount_out.symbol.raw()){
      
        check(quant.symbol       == market.reserve0.symbol, uniswap_failed{}, "token0 % and reserve0 % symbol mismatch", quant.symbol, market.reserve0.symbol);
        check(amount_out.symbol  == market.reserve1.symbol, uniswap_failed{}, "token1 % and reserve1 % symbol mismatch", amount_out.symbol, market.reserve1.symbol);

        amount0_in  = quant;
        amount1_out = amount_out; 
        amount0_out = asset(0,market.reserve0.symbol);
        amount1_in  = asset(0,market.reserve1.symbol);

    }else {
        check(amount_out.symbol  == market.reserve0.symbol, uniswap_failed{}, "token0 % and reserve0 % symbol mismatch", amount_out.symbol, market.reserve0.symbol);
        check(quant.symbol       == market.reserve1.symbol, uniswap_failed{}, "token1 % and reserve1 % symbol mismatch", quant.symbol, market.reserve1.symbol);

        amount0_out = amount_out;
        amount1_in  = quant;
        amount0_in  = asset(0,market.reserve0.symbol);
        amount1_out = asset(0,market.reserve1.symbol);       
    }

    WASM_LOG_PRINT(true, 
      " to:", to,   
      " amount0:", amount0_in,
      " amount1:", amount1_in,
      " amount0_out:", amount0_out,
      " amount1_out:", amount1_out,
      " market.reserve0:", market.reserve0,
      " market.reserve1:", market.reserve1,
      "\n")  

    //check(false, uniswap_failed{}, "insufficient output amount");

    check(amount0_out > 0 || amount1_out > 0, uniswap_failed{}, "insufficient output amount");
    check(amount0_out < market.reserve0  || amount1_out < market.reserve1, uniswap_failed{}, "insufficient liquidity");

    asset balance0, balance1;
    {
        check(market.token0 != to  && market.token1 != to, "invalid to");
        if(amount0_out > 0) TRANSFER(market.token0, get_self(), to, amount0_out);
        if(amount1_out > 0) TRANSFER(market.token1, get_self(), to, amount1_out);

        balance0 = market.reserve0 + amount0_in - amount0_out;
        balance1 = market.reserve1 + amount1_in - amount1_out;
    }

    {
        asset balance0_adjusted = balance0 * 1000 - amount0_in * 3;
        asset balance1_adjusted = balance1 * 1000 - amount1_in * 3;
        check(multiply_decimal(balance0_adjusted.amount, balance1_adjusted.amount) >= multiply_decimal(market.reserve0.amount, market.reserve1.amount) * 1000 * 1000, "invalid amount in");
    }

    WASM_LOG_FPRINT(true,"to:% balance0:% balance1:%", to, balance0, balance1);

    market.reserve0 = balance0 ;
    market.reserve1 = balance1 ;  
    market.block_timestamp_last = current_block_time();   
    wasm::db::set(market);
}

ACTION uniswap::close(std::string market_id, bool closed)
{
    require_auth( get_maintainer(get_self()));

    market_t market(string_to_market_id(market_id));
    check(wasm::db::get(market), uniswap_failed{}, "market does not exist");

    market.closed = closed;
    wasm::db::set(market);
}

ACTION uniswap::skim(std::string market_id, regid to, asset balance0, asset balance1)
{
    require_auth( get_maintainer(get_self()));

    market_t market(string_to_market_id(market_id));
    check(wasm::db::get(market), uniswap_failed{}, "market does not exist");
    check(market.closed, uniswap_failed{}, "market must be closed first"); 

    check(balance0.symbol  == market.reserve0.symbol, uniswap_failed{}, "token0 symbol mismatch");
    check(balance1.symbol  == market.reserve1.symbol, uniswap_failed{}, "token1 symbol mismatch");

    TRANSFER(market.token0, get_self(), to, balance0 - market.reserve0);
    TRANSFER(market.token1, get_self(), to, balance1 - market.reserve1);

    market.skim_last_time = current_block_time();
    wasm::db::set(market);

}

ACTION uniswap::sync(std::string market_id, asset balance0, asset balance1)
{
    require_auth( get_maintainer(get_self()));
    market_t market(string_to_market_id(market_id));
    check(wasm::db::get(market), uniswap_failed{}, "market does not exist");
    check(market.closed, uniswap_failed{}, "market must be closed first"); 

    check(balance0.symbol == market.reserve0.symbol, uniswap_failed{}, "token0 symbol mismatch");
    check(balance1.symbol == market.reserve1.symbol, uniswap_failed{}, "token1 symbol mismatch");

    market.reserve0 = balance0 ;
    market.reserve1 = balance1 ;  
    market.block_timestamp_last = current_block_time();   

    market.sync_last_time = current_block_time();
    wasm::db::set(market);

}

ACTION uniswap::get_market(std::string market_id)
{
    market_t market(string_to_market_id(market_id));
    check(wasm::db::get(market), uniswap_failed{}, "market % does not exist", market_id);

    WASM_LOG_FPRINT(true, "market:%", market)  
}

ACTION uniswap::get_action(regid to, uint64_t nonce)
{
    mint_action_t mint_action(to.value, nonce);
    check(wasm::db::get(mint_action), uniswap_failed{}, "mint_action step1 does not exist"); 

    WASM_LOG_FPRINT(true, "mint_action:%", mint_action)  
}


extern "C" bool pre_dispatch(regid self, regid original_receiver, name action) {
   return true;
}

extern "C" void post_dispatch(regid self, regid original_receiver, name action) {

}
