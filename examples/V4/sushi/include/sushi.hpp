#include <wasm.hpp>
#include <asset.hpp>

#include <string>
using namespace wasm;

namespace wasm {

   using std::string;

   CONTRACT master_chef : public contract {
       public:
           using contract::contract;
           ACTION init(regid sushi, regid migrator, uint64_t sushi_per_block, uint64_t bonus_end_block);
           ACTION add_pool(uint64_t alloc_point, regid lp_token, symbol lp_symbol, bool with_update);
           ACTION set_pool(uint64_t pid, uint64_t alloc_point, bool with_update);
           ACTION deposit(uint64_t pid, regid from);
           ACTION withdraw(uint64_t pid, regid to, asset quantity);
           // ACTION emer_withdraw(uint64_t pid, asset quantity);
           ACTION update_pool(uint64_t pid);
           ACTION update_pools();

           ACTION get_pending(uint64_t pid, regid owner);
           ACTION get_pool(uint64_t pid);

      private:
           uint64_t _get_multiplier(uint64_t from, uint64_t to);

   };
} /// namespace wasm
