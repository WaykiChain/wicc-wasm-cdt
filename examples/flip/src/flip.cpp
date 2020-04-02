#include <flip.hpp>
#include <system.hpp>
#include <strings.hpp>

using namespace wasm;

ACTION flip::transfer(regid from,
                      regid to,
                      asset quantity,
                      string memo) 
{
    if (to != get_self()) return;

    vector <string> transfer_memo = string_split(memo, ':');

    uint64_t operate = name(transfer_memo[0]).value;
    switch(operate)
    {
       case name("kick").value:
       {
        check(transfer_memo.size() == 7,
           "memo must be kick:bid_id:tab:bank:beg:one:gal, eg. kick:68feb6a4097a45d6e56f5b84f6c381b0c638a1306eb95b7ee2354e19838461e4:1000.00000000 WUSD:wasmio.bank:10.00000000 WUSD:10.00000000 WICC:xiaoyu1111111");
        check(transfer_memo[1].size() == 64, "bid_id size must be 32 bytes from sha256");

        uint8_t hash[32];
        memcpy(&hash, from_hex(transfer_memo[1]).data(), 32);
        checksum256 id = {hash};

        auto tab        = asset::from_string(transfer_memo[2]);
        auto bid        = tab; bid.set_amount(0);
        auto bid_issuer = regid(transfer_memo[3]);        

        auto lot        = quantity;
        auto lot_issuer = get_first_receiver(); 
        auto beg        = asset::from_string(transfer_memo[4]);
        auto one        = asset::from_string(transfer_memo[5]);

        //auto usr        = name(transfer_memo[6]);
        auto usr        = from;
        auto gal        = regid(transfer_memo[6]);
        //auto tab        = asset::from_string(transfer_memo[8]);
        
        check(bid.symbol == beg.symbol, "not matching beg symbol");
        check(one.symbol == lot.symbol, "not matching one symbol");
        check(bid.symbol == tab.symbol, "not matching tab symbol");

        kick(id, bid, bid_issuer, lot, lot_issuer, beg, one, usr, gal, tab);    

        break;    
       }

       case name("tend").value:
       case name("dent").value:
       {
         check(transfer_memo.size() == 3,
           "memo must be tend:bid_id:asset:asset, eg. tend:68feb6a4097a45d6e56f5b84f6c381b0c638a1306eb95b7ee2354e19838461e4:20.00000000 WICC");
        check(transfer_memo[1].size() == 64, "bid_id size must be 32 bytes from sha256");

        uint8_t hash[32];
        memcpy(&hash, from_hex(transfer_memo[1]).data(), 32);
        checksum256 id = {hash};

        asset lot = asset::from_string(transfer_memo[2]);
        asset bid = quantity;
        //asset bid = asset::from_string(transfer_memo[3]);

        if(operate == name("tend").value)
            tend(id, lot, bid, from);
        else
            dent(id, lot, bid, from);

        break;       
       }
       case name("yank").value:
       {
           check(transfer_memo.size() == 2,
                "memo must be yank:bid_id, eg. tend:68feb6a4097a45d6e56f5b84f6c381b0c638a1306eb95b7ee2354e19838461e4");
           check(transfer_memo[1].size() == 64, "bid_id size must be 32 bytes from sha256");

           uint8_t hash[32];
           memcpy(&hash, from_hex(transfer_memo[1]).data(), 32);
           checksum256 id = {hash};

           auto usr       = from;
           yank(id, usr, quantity);
       }
       break;
    }


    // if (name(transfer_memo[0]) == name("kick")) {
    //     check(transfer_memo.size() == 7,
    //        "memo must be kick:bid_id:tab:bank:beg:one:gal, eg. kick:68feb6a4097a45d6e56f5b84f6c381b0c638a1306eb95b7ee2354e19838461e4:1000.00000000 WUSD:wasmio.bank:10.00000000 WUSD:10.00000000 WICC:xiaoyu1111111");
    //     check(transfer_memo[1].size() == 64, "bid_id size must be 32 bytes from sha256");

    //     uint8_t hash[32];
    //     memcpy(&hash, from_hex(transfer_memo[1]).data(), 32);
    //     checksum256 id = {hash};

    //     auto tab        = asset::from_string(transfer_memo[2]);
    //     auto bid        = tab; bid.set_amount(0);
    //     auto bid_issuer = name(transfer_memo[3]);        

    //     auto lot        = quantity;
    //     auto lot_issuer = get_first_receiver(); 
    //     auto beg        = asset::from_string(transfer_memo[4]);
    //     auto one        = asset::from_string(transfer_memo[5]);

    //     //auto usr        = name(transfer_memo[6]);
    //     auto usr        = from;
    //     auto gal        = name(transfer_memo[6]);
    //     //auto tab        = asset::from_string(transfer_memo[8]);
        
    //     check(bid.symbol == beg.symbol, "not matching beg symbol");
    //     check(one.symbol == lot.symbol, "not matching one symbol");
    //     check(bid.symbol == tab.symbol, "not matching tab symbol");

    //     kick(id, bid, bid_issuer, lot, lot_issuer, beg, one, usr, gal, tab);
    // } else if (name(transfer_memo[0]) == name("tend") || transfer_memo[0] == "dent") {
    //     check(transfer_memo.size() == 3,
    //        "memo must be tend:bid_id:asset:asset, eg. tend:68feb6a4097a45d6e56f5b84f6c381b0c638a1306eb95b7ee2354e19838461e4:20.00000000 WICC");
    //     check(transfer_memo[1].size() == 64, "bid_id size must be 32 bytes from sha256");

    //     uint8_t hash[32];
    //     memcpy(&hash, from_hex(transfer_memo[1]).data(), 32);
    //     checksum256 id = {hash};

    //     asset lot = asset::from_string(transfer_memo[2]);
    //     asset bid = quantity;
    //     //asset bid = asset::from_string(transfer_memo[3]);

    //     if(transfer_memo[0] == "tend")
    //         tend(id, lot, bid, from);
    //     else
    //         dent(id, lot, bid, from);
    // } else if (transfer_memo[0] == "yank") {
    //     check(transfer_memo.size() == 2,
    //        "memo must be yank:bid_id, eg. tend:68feb6a4097a45d6e56f5b84f6c381b0c638a1306eb95b7ee2354e19838461e4");
    //     check(transfer_memo[1].size() == 64, "bid_id size must be 32 bytes from sha256");

    //     uint8_t hash[32];
    //     memcpy(&hash, from_hex(transfer_memo[1]).data(), 32);
    //     checksum256 id = {hash};

    //     auto usr       = from;
    //     yank(id, usr, quantity);
    // }

}

void flip::kick(checksum256 id,
                asset bid,
                regid bid_issuer,
                asset lot,
                regid lot_issuer,
                asset beg,
                asset one,
                regid usr,
                regid gal,
                asset tab) 
{

    //require_auth(usr);
    bid_t bid_object;
    bids bids_table(get_self(), bid_scope);
    check(!bids_table.get(bid_object, id), "bid has already exists");
    check(lot > 0, "lot must > 0");

    auto now = current_block_time(); 

    bids_table.emplace(get_self(), [&](auto &s) {
        s.id         = id;
        s.bid        = bid;
        s.bid_issuer = bid_issuer;
        s.lot        = lot;
        s.lot_issuer = lot_issuer;
        s.beg        = beg;
        s.one        = one;
        s.usr        = usr;
        s.gal        = gal;     
        s.tab        = tab;   
        s.guy        = usr;

        s.tic        = now + ttl;     
        s.end        = now + tau;   
    });

}

void flip::tend(checksum256 id, asset lot, asset bid, regid guy) 
{

    //require_auth(guy);
    bid_t bid_object;
    bids bids_table(get_self(), bid_scope);
    check(bids_table.get(bid_object, id), "bid doesn't exists");
    //check(bids_table.guy != {0}, "guy not set");

    auto now = current_block_time();
    check(bid_object.bid_issuer == get_first_receiver(), "not matching bid issuer");

    check(bid_object.lot.symbol == lot.symbol, "not matching lot symbol");
    check(bid_object.bid.symbol == bid.symbol, "not matching bid symbol");

    check(bid_object.tic != 0, "bid closed");
    check(bid_object.tic > now, "already finished tic");
    check(bid_object.end > now, "already finished end");

    check(lot == bid_object.lot , "not matching lot");
    check(bid <  bid_object.tab , "higher than tab" );

    check(bid - bid_object.bid  >= bid_object.beg  , "insufficient increase");  

    //退回给上次出价者
    if (bid_object.bid > 0) {
        wasm::transaction inline_trx1(bid_object.bid_issuer,
                                     name("transfer"),
                                     std::vector < permission > {{get_self(), name("wasmio.code")}},
                                     std::tuple(get_self(), bid_object.guy, bid_object.bid, string("tend")));
        inline_trx1.send();
    }


    //本次多出的部分转给拍卖接收账户
    wasm::transaction inline_trx2(bid_object.bid_issuer,
                                 name("transfer"),
                                 std::vector < permission > {{get_self(), name("wasmio.code")}},
                                 std::tuple(get_self(), bid_object.gal, bid - bid_object.bid, string("tend")));
    inline_trx2.send();

    bids_table.modify(bid_object, wasm::no_payer, [&](auto &s) {
        s.guy = guy;
        s.bid = bid;
        s.tic = now + ttl;
    });  

}

void flip::dent(checksum256 id, asset lot, asset bid, regid guy) 
{

    //require_auth(guy);
    bid_t bid_object;
    bids bids_table(get_self(), bid_scope);
    check(bids_table.get(bid_object, id), "bid doesn't exists");
    //check(bids_table.guy != {0}, "guy not set");

    auto now = current_block_time();
    check(bid_object.bid_issuer == get_first_receiver(), "not matching bid issuer");
    check(bid_object.lot.symbol == lot.symbol, "not matching lot symbol");
    check(bid_object.bid.symbol == bid.symbol, "not matching bid symbol");

    check(bid_object.tic != 0, "bid closed");
    check(bid_object.tic > now, "already finished tic");
    check(bid_object.end > now, "already finished end");

    // check(bid == bid_object.bid , "not matching bid" );
    check(bid == bid_object.tab , "tend not finished");
    check(lot <  bid_object.lot , "lot not lower"    );   

    check(bid_object.lot - lot  >= bid_object.one  , "insufficient decrease");  
   
    //退回给上次出价者
    if (bid_object.bid > 0) {
        wasm::transaction inline_trx1(bid_object.bid_issuer,
                                      name("transfer"),
                                      std::vector < permission > {{get_self(), name("wasmio.code")}},
                                      std::tuple(get_self(), bid_object.guy, bid_object.bid, string("dent")));
        inline_trx1.send();
    }

    //本次多出的部分转给拍卖lot接收账户
    wasm::transaction inline_trx2(bid_object.lot_issuer,
                                  name("transfer"),
                                  std::vector < permission > {{get_self(), name("wasmio.code")}},
                                  std::tuple(get_self(), bid_object.usr, bid_object.lot - lot, string("dent")));
    inline_trx2.send();

    bids_table.modify(bid_object, wasm::no_payer, [&](auto &s) {
        s.guy = guy;
        s.bid = bid;
        s.lot = lot;
        s.tic = now + ttl;
    });  

}

void flip::deal(checksum256 id, regid guy) 
{
    require_auth(guy);

    auto now = current_block_time();

    bid_t bid_object;
    bids bids_table(get_self(), bid_scope);
    check(bids_table.get(bid_object, id), "bid doesn't exists");
    check(bid_object.tic != 0, "bid closed");
    check(bid_object.tic < now && bid_object.end < now, "bid not finished");
    check(bid_object.guy == guy , "not matching guy");

    //抵押物转账给guy
    if (bid_object.lot > 0) {
        wasm::transaction inline_trx(bid_object.lot_issuer,
                                     name("transfer"),
                                     std::vector < permission > {{get_self(), name("wasmio.code")}},
                                     std::tuple(get_self(), guy, bid_object.lot, string("deal")));
        inline_trx.send();
    }

    //close bid
    bids_table.modify(bid_object, wasm::no_payer, [&](auto &s) {
        s.tic = 0;
    });
}

void flip::yank(checksum256 id, regid usr, asset payback) 
{
    require_auth(usr);

    bid_t bid_object;
    bids bids_table(get_self(), bid_scope);
    check(bids_table.get(bid_object, id), "bid doesn't exists");  

    check(bid_object.tic != 0, "closed");
    check(bid_object.bid <  bid_object.tab, "already dent phase");
    check(bid_object.bid <= payback, "pay back money is not enough");
    check(bid_object.usr == usr, "not matching usr");

    //退回上次出价者
    if (bid_object.bid > 0) {
        wasm::transaction inline_trx1(bid_object.bid_issuer,
                                      name("transfer"),
                                      std::vector < permission > {{get_self(), name("wasmio.code")}},
                                      std::tuple(get_self(), bid_object.guy, bid_object.bid, string("yank")));
        inline_trx1.send();
    }

    //抵押物退回usr
    if (bid_object.lot > 0) {
        wasm::transaction inline_trx2(bid_object.lot_issuer,
                                      name("transfer"),
                                      std::vector < permission > {{get_self(), name("wasmio.code")}},
                                      std::tuple(get_self(), usr, bid_object.lot, string("yank")));  
        inline_trx2.send(); 
    }

    //close bid
    bids_table.modify(bid_object, wasm::no_payer, [&](auto &s) {
        s.tic = 0;
    }); 


}


extern "C" {
void apply(uint64_t receiver, uint64_t code, uint64_t action) {
        switch (action) {
            case wasm::name("transfer").value:
                wasm::execute_action(wasm::regid(receiver), wasm::regid(code), &flip::transfer);
                break;
            case wasm::name("deal").value:
                wasm::execute_action(wasm::regid(receiver), wasm::regid(code), &flip::deal);
                break;
            default:
                check(false, "action does not exist");
                break;
        }
    }
}



