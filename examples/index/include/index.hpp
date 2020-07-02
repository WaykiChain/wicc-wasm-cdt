#include <wasm.hpp>
#include <asset.hpp>
#include <inline_transaction.hpp>
#include <database.hpp>

#include <string>
using namespace wasm;

namespace wasm {

   using std::string;

   CONTRACT index : public contract {
      public:
         using contract::contract;
 
         ACTION add(uint64_t id, regid owner, asset balance);
         ACTION get( uint64_t id, regid owner, symbol_code code);

   };
} /// namespace wasm
