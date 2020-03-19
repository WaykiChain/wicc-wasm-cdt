/**
 *  @file
 *  @copyright defined in wasm/LICENSE
 */
#pragma once

#include <vector>
#include <map>
#include <string>

using namespace std;
namespace wasm {

    inline string ltrim( const string& str ) {
        int h = 0, t = str.size();
        while (h < t && (str[h] == ' ' || str[h] == '\t')) h++;
        return str.substr(h, t - h);
    }

    inline string rtrim( const string& str ) {
        int h = 0, t = str.size();
        while (h < t && (str[t - 1] == ' ' || str[t - 1] == '\t')) t--;
        return str.substr(h, t - h);
    }

    inline string trim( const string &s ) {
        string str = s;
        ltrim(rtrim(str));
        return str;
    }

    inline vector <string> string_split(string str, char delimiter) {
        vector <string> r;
        string tmpstr;
        while (!str.empty()) {
            int ind = str.find_first_of(delimiter);
            if (ind == -1) {
                r.push_back(str);
                str.clear();
            } else {
                r.push_back(str.substr(0, ind));
                str = str.substr(ind + 1, str.size() - ind - 1);
            }
        }
        return r;

    }

    inline string from_hex(const string& str) {

        std::map<char, uint8_t> hex = {
                {'0', 0x00},
                {'1', 0x01},
                {'2', 0x02},
                {'3', 0x03},
                {'4', 0x04},
                {'5', 0x05},
                {'6', 0x06},
                {'7', 0x07},
                {'8', 0x08},
                {'9', 0x09},
                {'a', 0x0a},
                {'b', 0x0b},
                {'c', 0x0c},
                {'d', 0x0d},
                {'e', 0x0e},
                {'f', 0x0f}
        };
        vector <uint8_t> v;

        for (int i = 0; i < str.size();) {

            uint8_t h = hex[(char) str[i]];
            uint8_t l = hex[(char) str[i + 1]];
            uint8_t t = l | h << 4;
            v.push_back(t);

            i += 2;
        }

        return string(v.begin(), v.end());
    }


    inline vector <char> from_hex(const vector <char>& bytes) {

        std::map<char, uint8_t> hex = {
                {'0', 0x00},
                {'1', 0x01},
                {'2', 0x02},
                {'3', 0x03},
                {'4', 0x04},
                {'5', 0x05},
                {'6', 0x06},
                {'7', 0x07},
                {'8', 0x08},
                {'9', 0x09},
                {'a', 0x0a},
                {'b', 0x0b},
                {'c', 0x0c},
                {'d', 0x0d},
                {'e', 0x0e},
                {'f', 0x0f}
        };
        vector <uint8_t> v;

        for (int i = 0; i < bytes.size();) {

            uint8_t h = hex[(char) bytes[i]];
            uint8_t l = hex[(char) bytes[i + 1]];
            uint8_t t = l | h << 4;
            v.push_back(t);

            i += 2;
        }

        return vector <char>(v.begin(), v.end());
    }

}
