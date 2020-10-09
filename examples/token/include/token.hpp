#include <wasm.hpp>
#include <asset.hpp>
#include <table.hpp>
#include <inline_transaction.hpp>

#include <tokendb.hpp>
#include <string>
using namespace wasm;
using namespace wasm::db;

namespace wasm {

   using std::string;

   CONTRACT token : public contract {
      public:
         using contract::contract;
 
         ACTION create  (regid issuer, asset maximum_supply);
         ACTION issue   (regid to,     asset quantity, string memo );
         ACTION retire  (asset quantity, string memo );
         ACTION transfer(regid from, regid to, asset quantity, string  memo );
         ACTION open (regid owner, const symbol& symbol, regid payer );
         ACTION close(regid owner, const symbol& symbol );

         static asset get_supply( regid token_contract_account, symbol_code sym_code )
         {  
            currency_stats st;
            st.supply.symbol.code() = sym_code;
            db::get(st);
            return st.supply;
         }

         static asset get_balance( regid token_contract_account, regid owner, symbol_code sym_code )
         {
            account ac;
            ac.balance.symbol.code() = sym_code;
            db::get( ac );
            return ac.balance;
         }
         
      private:
         void sub_balance(regid owner, asset value);
         void add_balance(regid owner, asset value, regid ram_player);
   };
} /// namespace wasm
