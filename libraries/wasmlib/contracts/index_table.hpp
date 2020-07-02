#pragma once

#include "../core/datastream.hpp"
#include "../core/name.hpp"
#include "../core/database.h"

#include <vector>
#include <memory>
#include <boost/hana.hpp>

using namespace std;
using namespace wasm;
namespace hana = boost::hana;

namespace wasm {

constexpr static inline regid no_payer{};
constexpr static uint64_t SECONDARY_KEY_PREFIX = 0x000000000000000FULL;

template<class Class, typename Type, Type (Class::*PtrToMemberFunction)()const>
struct const_mem_fun
{
	typedef typename std::remove_reference<Type>::type result_type;

	template<typename ChainedPtr>
    auto operator()(const ChainedPtr& x)const -> std::enable_if_t<!std::is_convertible<const ChainedPtr&, const Class&>::value, Type>
    {
    	return operator()(*x);
    }

	Type operator()(const Class& x) const
	{
		return (x.*PtrToMemberFunction)();
	}

    Type operator()(const std::reference_wrapper<const Class>& x)const
    {
        return operator()(x.get());
    }

    Type operator()(const std::reference_wrapper<Class>& x)const
    {
        return operator()(x.get());
    }
};

template<name::raw IndexName, typename Extractor>
struct indexed_by {
   enum constants { index_name   = static_cast<uint64_t>(IndexName) };
   typedef Extractor secondary_extractor_type;
};


template<name::raw TableName, typename T, typename PrimaryKeyType, typename... Indices>
class index_table
{

public:
	  template<name::raw IndexName, typename Extractor, uint64_t Number, bool IsConst>
      struct index {
         public:
            typedef Extractor  secondary_extractor_type;
            typedef typename std::decay<decltype( Extractor()(nullptr) )>::type secondary_key_type;

            constexpr static bool validate_index_name( wasm::name n ) {
               return n.value != 0 && n != wasm::name("primary"); // Primary is a reserve index name.
            }

            static_assert( validate_index_name( wasm::name(IndexName) ), "invalid index name used in index_table" );

            enum constants {
               table_name   = static_cast<uint64_t>(TableName),
               index_name   = static_cast<uint64_t>(IndexName),
               index_number = Number,
               index_table_name = (static_cast<uint64_t>(TableName) & 0xFFFFFFFFFFFFFFF0ULL)
                                    | (Number & 0x000000000000000FULL) // Assuming no more than 16 secondary indices are allowed
            };

            constexpr static uint64_t name()   { return index_table_name; }
            constexpr static uint64_t number() { return Number; }
            uint64_t  get_code()const  { return _idx_table->get_code(); }
            uint64_t  get_scope()const { return _idx_table->get_scope(); }

            static auto extract_secondary_key(const T& obj) { return secondary_extractor_type()(obj); }
         private:
            friend class index_table;

            index( typename std::conditional<IsConst, const index_table*, index_table*>::type midx )
            :_idx_table(midx){}

            typename std::conditional<IsConst, const index_table*, index_table*>::type _idx_table;
      }; /// struct index_table::index

	template<uint64_t I>
	struct intc { enum e{ value = I }; operator uint64_t()const{ return I; }  };

	static constexpr auto transform_indices( ) {

		typedef decltype( hana::zip_shortest(
		                 hana::make_tuple( intc<0>(), intc<1>(), intc<2>(), intc<3>(), intc<4>(), intc<5>(),
		                                   intc<6>(), intc<7>(), intc<8>(), intc<9>(), intc<10>(), intc<11>(),
		                                   intc<12>(), intc<13>(), intc<14>(), intc<15>() ),
		                 hana::tuple<Indices...>() ) ) indices_input_type;

		return hana::transform( indices_input_type(), [&]( auto&& idx ){
			typedef typename std::decay<decltype(hana::at_c<0>(idx))>::type num_type;
			typedef typename std::decay<decltype(hana::at_c<1>(idx))>::type idx_type;
			return hana::make_tuple( hana::type_c<index<wasm::name::raw(static_cast<uint64_t>(idx_type::index_name)),
			                                         typename idx_type::secondary_extractor_type,
			                                         num_type::e::value, false> >,
				                     hana::type_c<index<wasm::name::raw(static_cast<uint64_t>(idx_type::index_name)),
				                                         typename idx_type::secondary_extractor_type,
				                                         num_type::e::value, true> > );

		});
	}

	typedef decltype( index_table::transform_indices() ) indices_type;
	indices_type _indices;

public:
	typedef T object_type;
	typedef PrimaryKeyType primary_key_type;
	
public:
	index_table( regid code, uint64_t scope )
	:_code(code),_scope(scope)
	{}

	template<typename Lambda>
	void emplace( regid payer, Lambda&& constructor ) {

		object_type obj;
		constructor(obj);

		auto key   = pack(std::tuple(TableName, _scope, obj.primary_key()));
		auto value = pack(obj);

        hana::for_each( _indices, [&]( auto& idx ) {
           typedef typename decltype(+hana::at_c<0>(idx))::type index_type;

           //auto secondary = pack(std::tuple(TableName, _scope, index_type::name(), index_type::extract_secondary_key(obj)));
           auto secondary = pack(std::tuple(SECONDARY_KEY_PREFIX, TableName, _scope, index_type::name(), index_type::extract_secondary_key(obj)));
           auto primary   = pack(obj.primary_key());

           db_store(payer.value, secondary.data(), secondary.size(), primary.data(), primary.size());


        });

		db_store(payer.value, key.data(), key.size(), value.data(), value.size());

	}

	template<typename Lambda>
	void modify( const object_type& obj, regid payer, Lambda&& updater ) {

		auto& mutableobj = const_cast<T&>(obj); // Do not forget the auto& otherwise it would make a copy and thus not update at all.
		updater( mutableobj );

	    auto key   = pack(std::tuple(TableName, _scope, obj.primary_key()));
		auto value = pack(obj);

		object_type old_object;
	    bool has_old_object = get(old_object, obj.primary_key());

        hana::for_each( _indices, [&]( auto& idx ) {
           typedef typename decltype(+hana::at_c<0>(idx))::type index_type;

           if(has_old_object && index_type::extract_secondary_key(old_object) != index_type::extract_secondary_key(obj))
           {
           	   //auto old_secondary = pack(std::tuple(TableName, _scope, index_type::name(), index_type::extract_secondary_key(old_object)));
           	   auto old_secondary = pack(std::tuple(SECONDARY_KEY_PREFIX, TableName, _scope, index_type::name(), index_type::extract_secondary_key(old_object)));
           	   db_remove(payer.value, old_secondary.data(), old_secondary.size());

	           //auto secondary = pack(std::tuple(TableName, _scope, index_type::name(), index_type::extract_secondary_key(obj)));
	           auto secondary = pack(std::tuple(SECONDARY_KEY_PREFIX, TableName, _scope, index_type::name(), index_type::extract_secondary_key(obj)));
	           auto primary   = pack(obj.primary_key());
	           db_store(payer.value, secondary.data(), secondary.size(), primary.data(), primary.size());
           }

        });

		db_update(payer.value, key.data(), key.size(), value.data(), value.size());   

	}

	void erase( const object_type& obj, regid payer) {

	    erase(obj.primary_key(), payer);
	}

	void erase( const primary_key_type& primary, regid payer ) {

        auto key = pack(std::tuple(TableName, _scope, primary));

		object_type object;
		bool has_object = get(object, primary);

        hana::for_each( _indices, [&]( auto& idx ) {
           typedef typename decltype(+hana::at_c<0>(idx))::type index_type;
           if(has_object)
           {
	       	   //vector<char> secondary = pack(std::tuple(TableName, _scope, index_type::name(), index_type::extract_secondary_key(object)));
           	   vector<char> secondary = pack(std::tuple(SECONDARY_KEY_PREFIX, TableName, _scope, index_type::name(),index_type::extract_secondary_key(object)));
	       	   db_remove(payer.value, secondary.data(), secondary.size());
       	   }
        });	

        db_remove(payer.value, key.data(), key.size());    
	}

	bool get(object_type& t, const primary_key_type& primary, const char* error_msg = "unable to find key" ) {

        auto key         = pack(std::tuple(TableName, _scope, primary));
		uint32_t key_len = key.size();

		auto value_len = db_get(key.data(), key_len, NULL, 0);
		if(value_len == 0){
			return false;
		}

		auto v = (char*)malloc(sizeof(char) * value_len);
		db_get( key.data(), key_len, v, value_len);
		t = unpack<object_type>(v, value_len);
		free(v);

		return true;
	}

	bool get(primary_key_type& primary, const object_type& t, uint64_t by_index, const char* error_msg = "unable to find key" ) {
        bool has_object = false;
        hana::for_each( _indices, [&]( auto& idx ) {
           typedef typename decltype(+hana::at_c<0>(idx))::type index_type;
           if(index_type::index_name == by_index)
           {
           	   //auto secondary   = pack(std::tuple(TableName, _scope, index_type::name(), index_type::extract_secondary_key(t)));
               auto secondary   = pack(std::tuple(SECONDARY_KEY_PREFIX, TableName, _scope,  index_type::name(), index_type::extract_secondary_key(t)));
           	   auto primary_len = db_get(secondary.data(), secondary.size(), NULL, 0);

           	   if(primary_len == 0) {
           	   	   return;
           	   }
               
		  	   auto v = (char*)malloc(sizeof(char) * primary_len);
			   db_get( secondary.data(), secondary.size(), v, primary_len);

		       primary = unpack<primary_key_type>(v, primary_len);
			   free(v);  

			   has_object = true;
			   return;
			}

        });

        return has_object;
	}


public:
	uint64_t get_code(){ return _code.value; }
	uint64_t get_scope(){ return _scope; }

private:
	regid    _code;
    uint64_t _scope;

};
}