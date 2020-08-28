#include <wasm.hpp>
#include <asset.hpp>

#include <string>
using namespace wasm;

namespace wasm {

   using std::string;

   CONTRACT uniswap : public contract {
       public:
           using contract::contract;

           ACTION init(regid token0, regid token1, symbol symbol0, symbol symbol1, symbol supply_symbol, regid liquidity_token);
           ACTION mint(regid to);   
           ACTION burn(regid to);    
           ACTION swap(asset amount0_out, asset amount1_out, regid to);     
           ACTION skim(regid to);  
           ACTION sync(); 
           ACTION get_market();

       private:
       	   void _update(asset balance0, asset balance1);
   };
} /// namespace wasm
