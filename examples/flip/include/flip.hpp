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
      ACTION transfer( name from, name to, asset quantity, string memo );
      ACTION deal(checksum256 id, name guy );

  private:
     TABLE  bid_t {
        checksum256    id;

        asset          bid;
        name           bid_issuer;//the contract issue the bid asset
        asset          lot;
        name           lot_issuer;//the contract issue the lot asset

        name           guy;//high bidder
        uint64_t       tic;//expiry time in minutes
        uint64_t       end;

        asset          beg;//minimum bid increase
        asset          one;//minimum log increase

        name           usr;//lot's owner
        name           gal;//receives bid income
        asset          tab;//total wanted

        checksum256 primary_key()const { return id; }
     };

     void kick( checksum256 id,
                asset bid,
                name  bid_issuer,
                asset lot,
                name  lot_issuer,
                asset beg,
                asset one,
                name  usr,
                name  gal,
                asset tab);
     void tend( checksum256 id, asset lot, asset bid, name guy);
     void dent( checksum256 id, asset lot, asset bid, name guy);
     void yank(checksum256 id, name usr, asset payback);


     typedef wasm::table< "bids"_n, bid_t, checksum256 > bids;  	


};
