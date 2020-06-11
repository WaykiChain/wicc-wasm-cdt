#include "core/datastream.hpp"
#include "contracts/system.hpp"
#include "contracts/privileged.hpp"
#include "rpc/rpc.hpp"

namespace wasm {
   extern "C" {
      __attribute__((wasm_wasm_import))
      uint64_t current_time();

     __attribute__((wasm_wasm_import))
     uint32_t get_active_producers(uint64_t*, uint32_t);

     __attribute__((wasm_wasm_import))
     uint32_t get_txid(void*);

        __attribute__((wasm_wasm_import))
     uint64_t get_maintainer(uint64_t);

     __attribute__((wasm_wasm_import))
     uint32_t get_system_asset_price(uint64_t, uint64_t, void*, uint32_t);

     __attribute__((wasm_wasm_import))
     void emit_result(const char *name, uint32_t name_sz, const char *type, uint32_t type_sz,
                         const char *value, uint32_t value_sz);
   }

   uint64_t current_block_time() {
      return current_time();
   }

   std::vector<regid> get_active_producers() {
       auto prod_cnt = get_active_producers(nullptr, 0)/8;
       std::vector<regid> active_prods(prod_cnt);
       get_active_producers((uint64_t*)active_prods.data(), active_prods.size() * sizeof(regid));
       return active_prods;
   }

    wasm::hash256 get_txid(){
      uint8_t hash[32];
      get_txid(hash);
      return wasm::hash256(hash);
  }


  wasm::regid get_maintainer(wasm::regid contract){
     return wasm::regid(get_maintainer(contract.value));
  }

  wasm::asset get_system_asset_price(symbol base, symbol quote){
      auto size = get_system_asset_price(base.raw(), quote.raw(), nullptr, 0);
      if( size == 0 ) return {};

      std::vector<char> bytes(size);
      get_system_asset_price(base.raw(), quote.raw(), bytes.data(), size);

      return wasm::unpack<wasm::asset>(bytes);
  }

  void emit_result_value(const string &name, const string &type, const std::vector<char> &value) {
      emit_result(name.c_str(), name.size(), type.c_str(), type.size(), value.data(),
                     value.size());
  }

} // namespace wasm
