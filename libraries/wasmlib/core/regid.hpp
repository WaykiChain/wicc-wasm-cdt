#pragma once

#include "check.hpp"
#include "serialize.hpp"

#include <string>
#include <string_view>

namespace wasm {

    //for account identity in wasm
    struct regid {
    public:
        // enum class raw : uint64_t {
        // };

        /**
         * Construct a new regid
         *
         * @brief Construct a new regid object defaulting to a value of 0
         *
         */
        constexpr regid() : value(0) {}

        /**
         * Construct a new regid given a unit64_t value
         *
         * @brief Construct a new regid object initialising value with v
         * @param v - The unit64_t value
         *
         */
        constexpr explicit regid( uint64_t v )
                : value(v) {}

        /**
         * Construct a new regid given a scoped enumerated type of raw (uint64_t).
         *
         * @brief Construct a new regid object initialising value with r
         * @param r - The raw value which is a scoped enumerated type of unit64_t
         *
         */
        // constexpr explicit regid( regid::raw r )
        //         : value(static_cast<uint64_t>(r)) {}

        /**
         * Construct a new regid given an string.
         *
         * @brief Construct a new regid object initialising value with str
         * @param str - The string value which validated then converted to unit64_t
         *
         */
        //constexpr explicit regid( std::string_view str )
        explicit regid( std::string_view  str )
                : value(0) {

               auto pos = str.find('-');

               check( pos > 0, "'-' must be between two numbers, ex. '999-80'");

               uint64_t height   = atoi(str.substr(0, pos).data());
               uint64_t index    = atoi(str.substr(pos + 1).data());   

               value = (height << 20) + index;
        }


        /**
         * Casts a regid to raw
         *
         * @return Returns an instance of raw based on the value of a regid
         */
        // constexpr operator raw() const { return raw(value); }

        /**
         * Explicit cast to bool of the uint64_t value of the regid
         *
         * @return Returns true if the regid is set to the default value of 0 else true.
         */
        constexpr explicit operator bool() const { return value != 0; }

        /**
         *  Returns the regid as a string.
         *
         *  @brief Returns the regid value as a string by calling write_as_string() and returning the buffer produced by write_as_string()
         */
        std::string to_string() const {
            uint64_t height = value >> 20;
            uint64_t index  = value & 0xFFFFF;

            char buffer[64];
            sprintf(buffer, "%lld-%lld", height, index);
            return std::string(buffer);       
        }

          /**
           * Prints an names as base32 encoded string
           *
           * @param name to be printed
           */
          inline void print()const {
            //internal_use_do_not_use::printn(value);
            std::string str = to_string();
            internal_use_do_not_use::prints_l(str.c_str(), str.size());
          }

        /**
         * Equivalency operator. Returns true if a == b (are the same)
         *
         * @return boolean - true if both provided %regid values are the same
         */
        friend constexpr bool operator==( const regid &a, const regid &b ) {
            return a.value == b.value;
        }

        /**
         * Inverted equivalency operator. Returns true if a != b (are different)
         *
         * @return boolean - true if both provided %regid values are not the same
         */
        friend constexpr bool operator!=( const regid &a, const regid &b ) {
            return a.value != b.value;
        }

        /**
         * Less than operator. Returns true if a < b.
         *
         * @return boolean - true if %regid `a` is less than `b`
         */
        friend constexpr bool operator<( const regid &a, const regid &b ) {
            return a.value < b.value;
        }


        uint64_t value = 0;


        WASMLIB_SERIALIZE( regid, (value) )

        /**  Serialize a symbol_code into a stream
        *
        *  @brief Serialize a symbol_code
        *  @param ds - The stream to write
        *  @param sym - The value to serialize
        *  @tparam DataStream - Type of datastream buffer
        *  @return DataStream& - Reference to the datastream
        */
        // template<typename DataStream>
        // friend inline DataStream &operator<<( DataStream &ds, const wasm::regid &regid ) {
        //     uint64_t raw = regid.value;
        //     ds.write((const char *) &raw, sizeof(uint64_t));
        //     return ds;
        // }

        /**
        *  Deserialize a symbol_code from a stream
        *
        *  @brief Deserialize a symbol_code
        *  @param ds - The stream to read
        *  @param symbol - The destination for deserialized value
        *  @tparam DataStream - Type of datastream buffer
        *  @return DataStream& - Reference to the datastream
        */
        // template<typename DataStream>
        // friend inline DataStream &operator>>( DataStream &ds, wasm::regid &regid ) {
        //     uint64_t raw = 0;
        //     ds.read((char *) &raw, sizeof(uint64_t));
        //     regid.value = raw;
        //     return ds;
        // }

    };

    // inline std::string to_string(const wasm::regid& v){
    //   return v.to_string();
    // }


} /// regidspace wayki

