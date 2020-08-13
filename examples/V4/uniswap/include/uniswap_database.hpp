#include <wasm.hpp>
#include <asset.hpp>
#include <db.hpp>
//#include <index_table.hpp>
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


#define TABLE_IN_CONTRACT [[wasm::table, wasm::contract("uniswap")]]
static constexpr uint64_t UNISWAP_SCOPE_RESEVED = 100;

struct TABLE_IN_CONTRACT market_t: wasm::db::index<"markets"_n, uint64_t> {
   regid    id;
   regid    token0;
   regid    token1;

   regid    token_contract;
   asset    total_supply;

   asset    reserve0;
   asset    reserve1;

   uint64_t block_timestamp_last;
   bool     closed;

   int128_t price0_cumulative_last = 0;
   int128_t price1_cumulative_last = 0;

   market_t(){}
   market_t(uint64_t r):id(regid(r)){}
   uint64_t scope() const { return UNISWAP_SCOPE_RESEVED; }
   uint64_t primary_key() const { return id.value; }

   WASMLIB_SERIALIZE( market_t, (id)(token0)(token1)(token_contract)(total_supply)
                                (reserve0)(reserve1)(block_timestamp_last)(closed)
                                (price0_cumulative_last)(price1_cumulative_last))

};

// TABLE  TABLE_IN_CONTRACT market_t{
//    regid    id;
//    regid    token0;
//    regid    token1;

//    regid    token_contract;
//    asset    total_supply;

//    asset    reserve0;
//    asset    reserve1;

//    uint64_t block_timestamp_last;
//    bool     closed;

//    int128_t price0_cumulative_last = 0;
//    int128_t price1_cumulative_last = 0;

//    market_t(){}
//    market_t(uint64_t r):id(regid(r)){}
//    uint64_t scope() const { return UNISWAP_SCOPE_RESEVED; }
//    uint64_t primary_key() const { return id.value; }

//    WASMLIB_SERIALIZE( market_t, (id)(token0)(token1)(token_contract)(total_supply)
//                                 (reserve0)(reserve1)(block_timestamp_last)(closed)
//                                 (price0_cumulative_last)(price1_cumulative_last))

// };

// typedef wasm::index_table< "market"_n, market_t, uint64_t> market;

// namespace wasm { namespace db {

//     static constexpr uint64_t  reserved  = 0;
//     enum return_t{
//         NONE    = 0,
//         MODIFIED,
//         APPENDED,
//     };

//     template<typename Type>
//     struct get_table_type{
//         using type = uint64_t;
//     };

//     template<>
//     struct get_table_type<market_t>{
//         using type = market;
//     };

//     template<typename ObjectType>
//     bool get(ObjectType& object) {
//         typename get_table_type<ObjectType>::type objects(regid(wasm::db::reserved), object.scope());
//         if ( !objects.get( object, object.primary_key())) {          
//           return false;
//         }
//         return true;
//     }

//     template<typename ObjectType>
//     return_t set(const ObjectType& object) {
//         ObjectType obj;
//         typename get_table_type<ObjectType>::type objects(regid(wasm::db::reserved), object.scope());

//         if (objects.get( obj, object.primary_key() )) {
//             objects.modify( obj, wasm::no_payer, [&]( auto& s ) {
//                s = object;
//             });
//             return return_t::MODIFIED;
//         } else {
//             objects.emplace( wasm::no_payer, [&]( auto& s ) {
//                s = object;
//             });
//             return return_t::APPENDED;
//         }
//     }

//     template<typename ObjectType>
//     void del(const ObjectType& object) {
//         typename get_table_type<ObjectType>::type objects(regid(wasm::db::reserved), object.scope());
//         objects.erase(object.primary_key(), wasm::no_payer);
//     }

//     template<typename ObjectType>
//     bool get(ObjectType& object, uint64_t indexed_by) {
//         typedef typename get_table_type<ObjectType>::type table_type;
//         table_type objects(regid(wasm::db::reserved), object.scope());

//         typename table_type::primary_key_type primary_key;
//         if(!objects.get(primary_key, object, indexed_by)) return false; 

//         if(!objects.get( object, primary_key)) return false;
//         return true;   
//     }

// }}//db//wasm