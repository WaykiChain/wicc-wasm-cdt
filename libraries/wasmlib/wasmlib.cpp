#include "core/datastream.hpp"
#include "contracts/system.hpp"
#include "contracts/privileged.hpp"

namespace wasm {
   extern "C" {
      __attribute__((wasm_wasm_import))
      uint64_t current_time();

     __attribute__((wasm_wasm_import))
     uint32_t get_active_producers(uint64_t*, uint32_t);
   }

   // system.hpp
   time_point current_time_point() {
      static auto ct = time_point(microseconds(static_cast<int64_t>(current_time())));
      return ct;
   }

   block_timestamp current_block_time() {
      static auto bt = block_timestamp(current_time_point());
      return bt;
   }

   std::vector<name> get_active_producers() {
      auto prod_cnt = get_active_producers(nullptr, 0)/8;
     std::vector<name> active_prods(prod_cnt);
     get_active_producers((uint64_t*)active_prods.data(), active_prods.size() * sizeof(name));
     return active_prods;
   }

} // namespace wasm
