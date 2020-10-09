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
         
      private:
         void sub_balance(regid owner, asset value);
         void add_balance(regid owner, asset value, regid ram_player);
   };
} /// namespace wasm
