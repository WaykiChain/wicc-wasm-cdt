#include <sushi.hpp>
#include <sushi_utils.hpp>
#include <inline_transaction.hpp>
#include <sushi_database.hpp>
#include <return.hpp>
#include <make_log.hpp>
#include <exception.hpp>

using namespace wasm;

std::optional<sushi_t> g_sushi;

uint64_t master_chef::_get_multiplier(uint64_t from, uint64_t to)
{
    check(!g_sushi.has_value(), "sushi already initialize");
    if( to <= g_sushi->bonus_end_block){
        return (to - from)  * BONUS_MULTIPLIER * PRECISION_1 / 3;
    } else if(from >= bonus_end_block){
        return (to - from) * PRECISION_1 / 3;
    } else {
        return ((g_sushi->bonus_end_block - from) * BONUS_MULTIPLIER +  (to - g_sushi->bonus_end_block)) * PRECISION_1 / 3;
    }
}

ACTION master_chef::init(regid migrator, uint64_t sushi_per_block, uint64_t bonus_end_block, uint64_t start_block, regid sushi_bank, symbol sushi_symbol)
{
    require_auth( get_maintainer(get_self()));

    check(!g_sushi.has_value(), "sushi already initialize");

    sushi_t storage(get_self().value);
    storage.migrator     = migrator;

    storage.sushi_per_block = sushi_per_block;
    storage.bonus_end_block = bonus_end_block;
    storage.start_block     = start_block;

    storage.sushi_bank      = sushi_bank;
    storage.sushi_symbol    = sushi_symbol;

    WASM_LOG_FPRINT(AMPL_DEBUG, "storage:%", storage)
    g_sushi = storage;

}

ACTION master_chef::add_pool(uint64_t alloc_point, regid lp_token, symbol_code lp_symbol, bool with_update)
{
    require_auth( get_maintainer(get_self()));
    check(g_sushi.has_value(), "sushi was not initialized");

    if(with_update){
       update_pools();
    }

    uint64_t last_reward_block = current_block_time() > g_sushi->start_block ? current_block_time()  : g_sushi->start_block ;
    g_sushi->push_back({ lp_token, alloc_point, last_reward_block, 0, TO_ASSET(0, lp_symbol), false})

}

ACTION master_chef::set_pool(uint64_t pid, uint64_t alloc_point, bool with_update);
{
    require_auth( get_maintainer(get_self()));
    check(g_sushi.has_value(), "sushi was not initialized");
    check(pid < g_sushi->pools.size(), "pool no.% does not exist", pid);

    if(with_update){
       update_pools();
    }

    pool_info& pool            = g_sushi->pools[pid];
    g_sushi->total_alloc_point = g_sushi->total_alloc_point - pool.alloc_point + alloc_point;
    pool.alloc_point           = alloc_point;

}

//owner should be send transfer lp token to this contract from lp_token first
ACTION master_chef::deposit(uint64_t pid, regid from)
{
    require_auth( from ); 
    check(g_sushi.has_value(), "sushi was not initialized");
    check(pid < g_sushi->pools.size(), "pool no.% does not exist", pid);

    pool_info& pool = g_sushi->pools[pid];

    account_t user(from.value, pid);
    wasm::db::get(user);
    asset quant = BALANCE_OF(pool.lp_bank, from, pool.total_liquidity.symbol) - pool.total_liquidity;

    update_pool(pid);
    if(user.amount > 0){
        uint64_t pending = multiply_decimal(user.amount, pool.acc_sushi_per_share) - user.reward;
        TRANSFER(g_sushi.sushi_bank, get_self(), from, TO_ASSET(pending, g_sushi.sushi_symbol.code()))
    }

    user.amount      = user.amount + quant.amount;
    user.reward_debt = multiply_decimal(user.amount, pool.acc_sushi_per_share);

    pool.total_liquidity += quant;
    wasm::db::set(user);
}

ACTION master_chef::withdraw(uint64_t pid, regid to, asset quant)
{
    require_auth( to ); 

    check(g_sushi.has_value(), "sushi was not initialized");
    check(pid < g_sushi->pools.size(), "pool no.% does not exist", pid);
    pool_info& pool = g_sushi->pools[pid];

    check(quant.symbol == pool.total_liquidity.symbol, "symbol:% and lptoken symbole:% mismatch", quant.symbol, pool.total_liquidity.symbol);

    account_t user(to.value, pid);
    check(wasm::db::get(user), "% account does not exist", to);
    check(user.amount > quantity.amount, "withdraw insufficient")

    update_pool(pid);
    if(user.amount > 0){
        uint64_t pending = multiply_decimal(user.amount, pool.acc_sushi_per_share) - user.reward;
        TRANSFER(g_sushi.sushi_bank, get_self(), from, TO_ASSET(pending, g_sushi.sushi_symbol.code()))
    }

    user.amount      = user.amount - quant.amount;
    user.reward_debt = multiply_decimal(user.amount, pool.acc_sushi_per_share);
    TRANSFER(pool.lp_bank, get_self(), to, quant)

    pool.total_liquidity -= quant;
    wasm::db::set(user);
}

ACTION master_chef::update_pool(uint64_t pid){
    check(g_sushi.has_value(), "sushi was not initialized");
    check(pid < g_sushi->pools.size(), "pool no.% does not exist", pid);
    pool_info& pool = g_sushi->pools[pid];

    if(current_block_time() <= pool.last_reward_block) return;

    asset lp_supply = BALANCE_OF(pool.lp_bank, get_self(), pool.total_liquidity.symbol);
    if(lp_supply == 0) {
        pool.last_reward_block = current_block_time()
        return;
    }

    uint64_t multiplier   = _get_multiplier(pool.last_reward_block, current_block_time());
    uint64_t sushi_reward = divide_decimal(multiply_decimal(multiply_decimal(multiplier, g_sushi->sushi_per_block ), pool.alloc_point), g_sushi->total_alloc_point);

    MINT(g_sushi->sushi_bank, get_self(), TO_ASSET(sushi_reward, g_sushi->sushi_total_supply.symbol.code()))
    MINT(g_sushi->sushi_bank, g_sushi->dev_addr, TO_ASSET(sushi_reward / 10, g_sushi->sushi_total_supply.symbol.code()))   

    pool.acc_sushi_per_share = pool.acc_sushi_per_share + divide_decimal(sushi_reward, lp_supply.amount);
    pool.last_reward_block   = current_block_time();

}

ACTION master_chef::update_pools(){
   check(g_sushi.has_value(), "sushi was not initialized");

   uint64_t pools_size = g_sushi->pools.size();
   for(int i = 0; i < pools_size ; i++){
      update_pool(i);
   }
}

ACTION master_chef::get_pending(uint64_t pid, regid owner){
    check(g_sushi.has_value(), "sushi was not initialized");
    check(pid < g_sushi->pools.size(), "pool no.% does not exist", pid);
    pool_info& pool = g_sushi->pools[pid];

    uint64_t acc_sushi_per_share = pool.acc_sushi_per_share;
    asset lp_supply = BALANCE_OF(pool.lp_bank, get_self(), pool.total_liquidity.symbol);
    if(current_block_time() > pool.last_reward_block && lp_supply != 0){
        uint64_t multiplier   = _get_multiplier(pool.last_reward_block, current_block_time());
        uint64_t sushi_reward = divide_decimal(multiply_decimal(multiply_decimal(multiplier, g_sushi->sushi_per_block ), pool.alloc_point), g_sushi->total_alloc_point);
        acc_sushi_per_share = acc_sushi_per_share + divide_decimal(sushi_reward, lp_supply.amount);
    }

    asset reward = TO_ASSET(acc_sushi_per_share, g_sushi.sushi_symbol.code());
    set_return(wasm::pack(reward));

    WASM_LOG_FPRINT(DEBUG, "reward:%", reward)

}

ACTION master_chef::get_pool(){

    check(g_sushi.has_value(), "sushi was not initialized");
    check(pid < g_sushi->pools.size(), "pool no.% does not exist", pid);

    pool_info& pool = g_sushi->pools[pid];
    set_return(wasm::pack(pool));

    WASM_LOG_FPRINT(DEBUG, "pool:%", pool)    

}

ACTION master_chef::get_chef(){

    check(g_sushi.has_value(), "sushi was not initialized");
    set_return(wasm::pack(g_sushi));

    WASM_LOG_FPRINT(DEBUG, "chef:%", g_sushi)    

}



extern "C" bool pre_dispatch(regid self, regid original_receiver, name action) {
   sushi_t storage(self.value);
   if(wasm::db::get(sushi_t)) g_sushi = storage;

   return true;
}

extern "C" void post_dispatch(regid self, regid original_receiver, name action) {
  if(g_sushi.has_value()) wasm::db::set(g_sushi.value());

}


