#include <token.hpp>


ACTION token::create( regid  issuer,
                      asset  maximum_supply )
{
   require_auth( issuer );
   auto maintainer = get_maintainer(_self);
   check( issuer == maintainer, "issuer not contract maintainer");

   auto sym = maximum_supply.symbol;
   check( sym.is_valid(), "invalid symbol name" );
   check( maximum_supply.is_valid(), "invalid supply" );
   check( maximum_supply.amount > 0, "max-supply must be positive" );

   currency_stats st(sym.code().raw());
     
   st.supply.symbol = sym;
   st.max_supply = maximum_supply;
   st.issuer = issuer;

   check( !db::get(st), "token with symbol already exists" );
   db::set(st);

}

ACTION token::issue( regid to, asset quantity, string memo )
{  
    auto sym = quantity.symbol;
    check( sym.is_valid(), "invalid symbol name" );
    check( memo.size() <= 256, "memo has more than 256 bytes" );

    currency_stats st(sym.code().raw());
    st.supply.symbol = sym;
    check( db::get( st ), "token with symbol does not exist, create token before issue" );
    require_auth( st.issuer );
    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must issue positive quantity" );
    check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    check( quantity.amount <= st.max_supply.amount - st.supply.amount, "quantity exceeds available supply");
    st.supply += quantity;
    db::set( st );
    add_balance( st.issuer, quantity, st.issuer );
    if( to != st.issuer ) {
      wasm::transaction inline_trx(get_self(), name("transfer"), std::vector<permission>{{st.issuer, name("wasmio.owner")}}, std::tuple(st.issuer, to, quantity, memo));
      inline_trx.send();

    }
}

ACTION token::retire( asset quantity, string memo )
{
    auto sym = quantity.symbol;
    check( sym.is_valid(), "invalid symbol name" );
    check( memo.size() <= 256, "memo has more than 256 bytes" );

    currency_stats st(sym.code().raw());
    st.supply.symbol = sym;
    check( db::get( st ), "token with symbol does not exist" );

    require_auth( st.issuer );
    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must retire positive quantity" );
    check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );

    st.supply -= quantity;
    db::set( st );

    sub_balance( st.issuer, quantity );
}

ACTION token::transfer( regid    from,
                        regid    to,
                        asset   quantity,
                        string  memo )
{
    check( from != to, "cannot transfer to self" );
    require_auth( from );
    check( is_account( to ), "to account does not exist");
    auto sym = quantity.symbol.code();

    currency_stats st(sym.raw());
    st.supply.symbol = quantity.symbol;
    db::get( st );

    notify_recipient( from );
    notify_recipient( to );

    check( quantity.is_valid(), "invalid quantity" );
    check( quantity.amount > 0, "must transfer positive quantity" );
    check( quantity.symbol == st.supply.symbol, "symbol precision mismatch" );
    check( memo.size() <= 256, "memo has more than 256 bytes" );

    auto payer = has_auth( to ) ? to : from;

    sub_balance( from, quantity );
    add_balance( to, quantity, payer );
}

void token::sub_balance( regid owner, asset value ) {
   account from(owner.value);
   from.balance.symbol = value.symbol;

   check( db::get( from ), "no balance object found" );
   check( from.balance.amount >= value.amount, "overdrawn balance" );

   from.balance -= value;
   db::set( from );
}


void token::add_balance( regid owner, asset value, regid payer )
{
   account to(owner.value);
   to.balance.symbol = value.symbol;
   db::get(to);
   if( !db::get( to )) {
        to.owner   = owner;
        to.balance = value;
   } else {
        to.balance += value;
   }
   
   db::set(to);
}

ACTION token::open( regid owner, const symbol& symbol, regid payer )
{
   require_auth( payer );

   auto sym_code_raw = symbol.code().raw();
   currency_stats st(sym_code_raw);
   st.supply.symbol = symbol;
   check( db::get( st ), "symbol does not exist" );
   check( st.supply.symbol == symbol, "symbol precision mismatch" );

   account account(owner.value);
   account.balance.symbol = symbol;

   if( !db::get( account ) ) {
      account.owner = owner;
      account.balance = asset{0, symbol};
      db::set(account);
   }
}

ACTION token::close( regid owner, const symbol& symbol )
{
   require_auth( owner );

   account account(owner.value);
   account.balance.symbol = symbol;

   check( db::get(account), "Balance row already deleted or never existed. Action won't have any effect." );
   check( account.balance.amount == 0, "Cannot close because the balance is not zero." );
   db::del( account );
}


WASM_DISPATCH( token, (create)(issue)(retire)(transfer)(open)(close))
