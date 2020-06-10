#include <rpc_get.hpp>
#include <rpc.hpp>

using namespace wasm;
// using std::chrono::system_clock;

ACTION rpc::recharge(regid from, regid to, asset quantity, string memo) {

    check(from != to, "cannot recharge to self");
    require_auth(from);
    check(is_account(to), "to account does not exist");
    auto sym = quantity.symbol.code();

    //  currency_stats st;
    //  stats statstable( _self, sym.raw() );
    //  statstable.get( st, sym.raw() );

    notify_recipient(from);
    notify_recipient(to);

    //  check( quantity.is_valid(), "invalid quantity" );
    //  check( quantity.amount > 0, "must transfer positive quantity" );
    //  check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    //  check( memo.size() <= 256, "memo has more than 256 bytes" );

    //  auto payer = has_auth( to ) ? to : from;

    add_balance(to, quantity);
}

ACTION rpc::get_balance(regid id, symbol_code sym) {

    accounts accts(_self, id.value);
    account_t acct;
    uint64_t balance = 0;
    if (accts.get(acct, sym.raw())) {
        balance = acct.balance.amount;
    }
    auto value = wasm::pack<uint64_t>(balance);
    set_rpc_result(wasm::name("uint64").value, value);
}

// void rpc::sub_balance( regid owner, asset value ) {
//    accounts from_acnts( _self, owner.value );

//    account from;
//    check( from_acnts.get( from, value.symbol.code().raw()), "no balance object found" );
//    check( from.balance.amount >= value.amount, "overdrawn balance" );

//    from_acnts.modify( from, owner, [&]( auto& a ) {
//          a.balance -= value;
//       });
// }

void rpc::add_balance(regid id, asset value) {
    accounts accts(_self, id.value);

    account_t acct;
    if (!accts.get(acct, value.symbol.code().raw())) {
        accts.emplace(id, [&](auto &a) {
            a.id      = id;
            a.balance = value;
        });
    } else {
        accts.modify(acct, wasm::no_payer, [&](auto &a) { a.balance += value; });
    }
}

WASM_DISPATCH(rpc, (recharge))
