#include <wasm.hpp>
#include <asset.hpp>
#include <index_table.hpp>
#include <inline_transaction.hpp>

#include <string>
using namespace wasm;

#define WASM_FUNCTION_PRINT_LENGTH 50

#define WASM_LOG_PRINT( debug,  ... ) {     \
if ( debug ) {                               \
   std::string str = std::string(__FILE__); \
   str += std::string(":");                 \
   str += std::to_string(__LINE__);         \
   str += std::string(":[");                \
   str += std::string(__FUNCTION__);        \
   str += std::string("]");                 \
   while(str.size() <= WASM_FUNCTION_PRINT_LENGTH) str += std::string(" ");\
   print(str);                                                             \
   print( __VA_ARGS__ ); }}


#define TABLE_IN_CONTRACT [[wasm::table, wasm::contract("index")]]
static constexpr uint64_t INDEX_SCOPE_RESEVED = 100;

TABLE  TABLE_IN_CONTRACT account {
   uint64_t id;
   regid    owner;
   asset    balance;

   uint64_t scope() const { return INDEX_SCOPE_RESEVED; }
   uint64_t primary_key()const { return id; }
   uint64_t get_symbol_code()const { return balance.symbol.code().raw(); }
   regid    get_owner()const{ return owner; }
};

typedef wasm::index_table< "accounts"_n, account, uint64_t,
                           indexed_by<"by_owner"_n, const_mem_fun<account, regid, &account::get_owner>>,
                           indexed_by<"by_code"_n, const_mem_fun<account, uint64_t, &account::get_symbol_code>>
                         > accounts;

namespace wasm { namespace db {

    static constexpr uint64_t  reserved  = 0;
    enum return_t{
        NONE    = 0,
        MODIFIED,
        APPENDED,
    };

    template<typename Type>
    struct get_table_type{
        using type = uint64_t;
    };

    template<>
    struct get_table_type<account>{
        using type = accounts;
    };

    template<typename ObjectType>
    bool get(ObjectType& object) {
        typename get_table_type<ObjectType>::type objects(regid(wasm::db::reserved), object.scope());
        if ( !objects.get( object, object.primary_key())) return false;
        return true;
    }

    template<typename ObjectType>
    return_t set(const ObjectType& object) {
        ObjectType obj;
        typename get_table_type<ObjectType>::type objects(regid(wasm::db::reserved), object.scope());

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
        typename get_table_type<ObjectType>::type objects(regid(wasm::db::reserved), object.scope());
        objects.erase(object.primary_key(), wasm::no_payer);
    }

    template<typename ObjectType>
    bool get(ObjectType& object, uint64_t indexed_by) {
        typedef typename get_table_type<ObjectType>::type table_type;
        table_type objects(regid(wasm::db::reserved), object.scope());

        typename table_type::primary_key_type primary_key;
        if(!objects.get(primary_key, object, indexed_by)) return false; 

        if(!objects.get( object, primary_key)) return false;
        return true;   
    }

}}//db//wasm