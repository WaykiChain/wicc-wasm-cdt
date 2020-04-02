#include <wasm.hpp>
#include <asset.hpp>
#include <table.hpp>
#include <string>
using namespace wasm;
using namespace std;

// static constexpr int64_t  ttl = 3*60*60;
// static constexpr int64_t  tau = 2*24*60*60;
static constexpr int64_t  ttl = 1*60;
static constexpr int64_t  tau = 10*60;
static constexpr int64_t  bid_scope = 100;



CONTRACT flip : public contract {
   public:
      using contract::contract;
      ACTION transfer( regid from, regid to, asset quantity, string memo );
      ACTION deal(checksum256 id, regid guy );

  private:
     TABLE  bid_t {
        checksum256    id;

        asset          bid;
        regid          bid_issuer;//the contract issue the bid asset
        asset          lot;
        regid          lot_issuer;//the contract issue the lot asset

        regid           guy;//high bidder
        uint64_t       tic;//expiry time in minutes
        uint64_t       end;

        asset          beg;//minimum bid increase
        asset          one;//minimum log decrease

        regid          usr;//lot's owner
        regid          gal;//receives bid income
        asset          tab;//total wanted

        checksum256 primary_key()const { return id; }
     };

     void kick( checksum256 id,
                asset bid,
                regid bid_issuer,
                asset lot,
                regid lot_issuer,
                asset beg,
                asset one,
                regid usr,
                regid gal,
                asset tab);
     void tend( checksum256 id, asset lot, asset bid, regid guy);
     void dent( checksum256 id, asset lot, asset bid, regid guy);
     void yank( checksum256 id, regid usr,  asset payback);


     typedef wasm::table< "bids"_n, bid_t, checksum256 > bids;  	


};
