#include <wasm.hpp>
#include <asset.hpp>
#include <table.hpp>
#include <string>
using namespace wasm;
using namespace std;

static constexpr int64_t  min_lock_time = 60;
static constexpr int64_t  max_lock_time = 7*24*60*60;
static constexpr int64_t  hash_time_lock_scope = 100;

CONTRACT hashtimelock : public contract {
   public:
      using contract::contract;
      ACTION transfer( regid from, regid to, asset quantity, string memo );
      ACTION unlock  ( string key, regid unlocker);
      ACTION refund  ( string key, regid locker  );

  private:
     TABLE  hash_time_lock {
        regid       bank;
        checksum256 lock_hash;
        regid       locker;
        regid       unlocker;
        asset       locked_quantity;
        uint64_t    locked_block_time;
        uint64_t    refund_lock_seconds;
        bool        closed;

        checksum256 primary_key()const { return lock_hash; }
     };

     uint64_t get_lock_deadline(const hash_time_lock& htl);
     void     lock(checksum256 lock_hash, regid locker, regid unlocker, asset quantity, uint64_t refund_lock_seconds);

     typedef wasm::table< "htls"_n, hash_time_lock, checksum256 > htls;  	

};
