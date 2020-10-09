#pragma once
#include <wasm.hpp>
#include <asset.hpp>
#include <table.hpp>

namespace wasm{
    namespace db{
        #define TABLE_IN_CONTRACT [[wasm::table, wasm::contract("token")]]

        struct  TABLE_IN_CONTRACT account {
            regid owner;
            asset balance;

            uint64_t primary_key() const {return balance.symbol.code().raw(); }
            uint64_t scope() const {return owner.value; }

            account() {}
            account(const regid &owner_in, const symbol &symb_in): owner(owner_in), balance(asset(0, symb_in)) {}

            WASMLIB_SERIALIZE( account, (owner)(balance))
        };
        typedef wasm::table<"accounts"_n, account, uint64_t> accounts;
    

        struct TABLE_IN_CONTRACT currency_stats {
            symbol_code symb;
            asset supply;
            asset max_supply;
            regid issuer;

            uint64_t primary_key()const {return symb.raw(); }
            uint64_t scope()const {return symb.raw(); }

            currency_stats() {}
            currency_stats(const symbol_code &symb_in): symb(symb_in) {}

            WASMLIB_SERIALIZE( currency_stats, (symb)(supply)(max_supply)(issuer))
        };
        typedef wasm::table< "stat"_n, currency_stats, uint64_t > stats;


        static constexpr regid reserved = regid(0);

        enum return_t{
            NONE    = 0,
            MODIFIED,
            APPENDED,
        };

        template<typename Type>
        struct get_table_type{
        };

        template<>
        struct get_table_type<account>{
            using type = accounts;
        };

        template<>
        struct get_table_type<currency_stats>{
            using type = stats;
        };

        template<typename ObjectType>
        // get info from db
        bool get(ObjectType& object) {
            typename get_table_type<ObjectType>::type objects(reserved, object.scope());

            if (!objects.get(object, object.primary_key())) return false;

            return true;
        }

        template<typename ObjectType>
        return_t set(const ObjectType& object) {
            ObjectType obj;
            typename get_table_type<ObjectType>::type objects(reserved, object.scope());

            if (objects.get( obj, object.primary_key() )) {
                objects.modify( obj, wasm::no_payer, [&]( auto& s ) {
                    s = object;
                });
                return return_t::MODIFIED;
            } else {
                objects.emplace( wasm::no_payer, [&]( auto& s ) {
                    s = object;
                });
                return return_t::APPENDED;
            }
        }

        template<typename ObjectType>
        void del(const ObjectType& object) {
            typename get_table_type<ObjectType>::type objects(reserved, object.scope());
            objects.erase(object.primary_key(), wasm::no_payer);
        }
    }
}