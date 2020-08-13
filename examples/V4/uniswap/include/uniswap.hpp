#include <wasm.hpp>
#include <asset.hpp>
#include <inline_transaction.hpp>
#include <uniswap_database.hpp>
#include <return.hpp>
#include <make_log.hpp>

#include <string>
using namespace wasm;

namespace wasm {

   using std::string;

   CONTRACT uniswap : public contract {
       public:
           using contract::contract;

           ACTION init(regid token0, regid token1, symbol symbol0, symbol symbol1, symbol supply_symbol, regid token_contract);
           ACTION mint(regid to);   
           ACTION burn(regid to);    
           ACTION swap(asset amount0_out, asset amount1_out, regid to);     
           ACTION skim(regid to);  
           ACTION sync(); 
           ACTION get_market();

       public:
       	   void update(asset balance0, asset balance1);
   };
} /// namespace wasm
