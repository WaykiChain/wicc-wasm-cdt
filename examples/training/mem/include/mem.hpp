#include <wasm.hpp>
#include <asset.hpp>
#include <inline_transaction.hpp>

#include <string>
using namespace wasm;

namespace wasm {

   CONTRACT mem : contract{
   	public:
   		 using contract::contract;
         uint32_t balance1;
         uint32_t balance2;
         //mem( uint64_t self, uint64_t first_receiver):_self(self),_first_receiver(first_receiver) {}

      public:
         ACTION prints(uint32_t b1, uint32_t b2);

   public:
      /**
       * The regid of the account this contract is deployed on.
       */
      uint64_t _self;

      /**
       * The account the incoming action was first received at.
       */
      uint64_t _first_receiver;

   };
} /// namespace wasm
