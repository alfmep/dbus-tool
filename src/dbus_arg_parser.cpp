/*
 * Copyright (C) 2023,2026 Dan Arrhenius <dan@ultramarin.se>
 *
 * This file is part of dbus-tool.
 *
 * dbus-tool is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "dbus_arg_parser.hpp"
#include <cstring>
#include <regex>
#include <set>

#include <iostream>


#define RE_ASCII_NO_DBL_QUOTE "[\\x20-\\x21\\x23-\\x5b\\x5d-\\x7f]"
#define RE_ASCII_NO_QUOTE     "[\\x20-\\x26\\x28-\\x5b\\x5d-\\x7f]"
#define RE_U     "[\\x80-\\xbf]"
#define RE_U2    "[\\xc2-\\xdf]"
#define RE_U3    "[\\xe0-\\xef]"
#define RE_U4    "[\\xf0-\\xf4]"
#define RE_ESCAPED_CHAR_DBL_QUOTE "\\\\[\\\"\\\\bfnrt]" //==>  \\[\"\\bfnrt]  ==>  \["\bfnrt]
#define RE_ESCAPED_CHAR_QUOTE     "\\\\['\\\\bfnrt]"    //==>  \\['\\bfnrt]   ==>  \['\bfnrt]
#define RE_UANY_NO_DBL_QUOTE RE_ASCII_NO_DBL_QUOTE "|" RE_U2 RE_U "|" RE_U3 RE_U RE_U "|" RE_U4 RE_U RE_U RE_U
#define RE_UANY_NO_QUOTE RE_ASCII_NO_QUOTE "|" RE_U2 RE_U "|" RE_U3 RE_U RE_U "|" RE_U4 RE_U RE_U RE_U
#define RE_CHAR_NO_DBL_QUOTE "(" RE_UANY_NO_DBL_QUOTE ")|(" RE_ESCAPED_CHAR_DBL_QUOTE ")"
#define RE_CHAR_NO_QUOTE "(" RE_UANY_NO_QUOTE ")|(" RE_ESCAPED_CHAR_QUOTE ")"
#define RE_CHARS_NO_DBL_QUOTE "(" RE_CHAR_NO_DBL_QUOTE ")*"
#define RE_CHARS_NO_QUOTE "(" RE_CHAR_NO_QUOTE ")*"

#define RE_STRING_BY_DBL_QUOTE "\\\"" RE_CHARS_NO_DBL_QUOTE "\\\""
#define RE_STRING_BY_QUOTE "'" RE_CHARS_NO_QUOTE "'"
#define RE_STRING "(" RE_STRING_BY_DBL_QUOTE ")|(" RE_STRING_BY_QUOTE ")"

#define RE_BOOLEAN "[Tt][Rr][Uu][Ee]|[Ff][Aa][Ll][Ss][Ee]|1|0"

#define RE_DIGIT "[0-9]"
#define RE_DIGITS RE_DIGIT "+"
#define RE_MINUS "[-]"
#define RE_SIGN "[+-]"
#define RE_FRAC "[.]" RE_DIGITS
#define RE_EXP "[eE]" RE_SIGN "?" RE_DIGITS

#define RE_UNSIGNED_INTEGER RE_DIGITS
#define RE_SIGNED_INTEGER RE_MINUS "?" RE_DIGITS
#define RE_DOUBLE RE_SIGNED_INTEGER "|" RE_SIGNED_INTEGER RE_FRAC "|" RE_SIGNED_INTEGER RE_EXP "|" RE_SIGNED_INTEGER RE_FRAC RE_EXP

#define RE_OPATH "(/[A-Za-z0-9_]+)+|/"
#define RE_SIGNATURE "[ybnqiuxtdhsogav(){}]+"


namespace ubus = ultrabus;


static size_t signature_len (const char* signature);
static std::string unescape_escaped_str_value (const std::string& in);


static const std::set<char> basic_type_signatures = {
    // DBus signatures for all basic values
    'y','b','n','q','i','u','x','t','d','h','s','o','g'
};





//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::unique_ptr<ultrabus::dbus_type> dbus_arg_parser::operator() (
        const std::string& signature,
        const std::string& value)
{
    auto len = std::make_pair<size_t, size_t> (0, 0);
    return parse_dbus_arg_string (signature.c_str(),
                                  value.c_str(),
                                  len);
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::unique_ptr<ultrabus::dbus_type> dbus_arg_parser::parse_dbus_arg_string (
        const char* signature,
        const char* value_buffer,
        std::pair<size_t, size_t>& len)
{
    using std::regex_constants::match_continuous;

    std::unique_ptr<ubus::dbus_type> value (nullptr);
    auto sub_len = std::make_pair<size_t, size_t> (0, 0);
    bool ok = true;

    if (signature==nullptr || signature[0]=='\0' || !dbus_signature_validate(signature, nullptr)) {
        return value;
    }

    if (basic_type_signatures.find(signature[0]) != basic_type_signatures.end()) {
        // The signature is a basic value
        auto val = parse_dbus_basic_arg_string (signature, value_buffer, sub_len, ok);
        if (ok)
            value = std::move (val);
    }
    else if (signature[0] == 'a') {
        // The signature is an array or dict
        std::unique_ptr<ubus::dbus_type> val;
        if (signature[1] == '{')
            val = parse_dbus_dict_arg_string (signature, value_buffer, sub_len, ok);
        else
            val = parse_dbus_array_arg_string (signature, value_buffer, sub_len, ok);
        if (ok)
            value = std::move (val);
    }
    else if (signature[0] == '(') {
        // The signature is a struct
        auto val = parse_dbus_struct_arg_string (signature, value_buffer, sub_len, ok);
        if (ok)
            value = std::move (val);
    }
    else if (signature[0] == 'v') {
        // The signature is a variant
        auto val = parse_dbus_variant_arg_string (signature, value_buffer, sub_len, ok);
        if (ok)
            value = std::move (val);
    }
    // else if (signature[0] == '{') {
    //     // The signature is a dict entry
    //     auto val = parse_dbus_dict_entry_arg_string (signature, value_buffer, sub_len, ok);
    //     if (ok)
    //         value.reset (new ubus::dbus_dict_entry(val));
    // }
    else {
        std::cerr << "Parse a what?!?" << std::endl;
    }

    len.first += sub_len.first;
    len.second += sub_len.second;

    return value;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::unique_ptr<ultrabus::dbus_type> dbus_arg_parser::parse_dbus_basic_arg_string (
        const char* signature,
        const char* value_buffer,
        std::pair<size_t, size_t>& len,
        bool& ok)
{
    using std::regex_constants::match_continuous;
    static const std::regex re_boolean (RE_BOOLEAN);
    static const std::regex re_signed (RE_SIGNED_INTEGER);
    static const std::regex re_unsigned (RE_UNSIGNED_INTEGER);
    static const std::regex re_double (RE_DOUBLE);
    static const std::regex re_string (RE_STRING);
    static const std::regex re_opath (RE_OPATH);
    static const std::regex re_signature (RE_SIGNATURE);

    std::unique_ptr<ubus::dbus_type> value;
    std::cmatch m;

    len.first = len.second = 0;
    ok = false;

    switch (signature[0]) {
    case 'y': // BYTE - Unsigned 8-bit integer
        if (std::regex_search(value_buffer, m, re_unsigned, match_continuous)) {
            value.reset (new ubus::dbus_byte((uint8_t)std::stoi(m[0], nullptr, 0)));
            len.first = 1;
            len.second = m.length (0);
            ok = true;
        }
        break;

    case 'b': // BOOLEAN - 0, 1, false, or true
        if (std::regex_search(value_buffer, m, re_boolean, match_continuous)) {
            if (strcasecmp(m[0].str().c_str(), "true") == 0)
                value.reset (new ubus::dbus_bool(true));
            else if (strcasecmp(m[0].str().c_str(), "false") == 0)
                value.reset (new ubus::dbus_bool(false));
            else
                value.reset (new ubus::dbus_bool(static_cast<bool>(std::stoi(m[0], nullptr, 0))));
            len.first = 1;
            len.second = m.length (0);
            ok = true;
        }
        break;

    case 'n': // INT16 - Signed 16-bit integer
        if (std::regex_search(value_buffer, m, re_signed, match_continuous)) {
            value.reset (new ubus::dbus_i16(((int16_t)std::stoi(m[0], nullptr, 0))));
            len.first = 1;
            len.second = m.length (0);
            ok = true;
        }
        break;

    case 'q': // UINT16 - Unsigned 16-bit integer
        if (std::regex_search(value_buffer, m, re_unsigned, match_continuous)) {
            value.reset (new ubus::dbus_u16(((uint16_t)std::stoi(m[0], nullptr, 0))));
            len.first = 1;
            len.second = m.length (0);
            ok = true;
        }
        break;

    case 'i': // INT32 - Signed 32-bit integer
        if (std::regex_search(value_buffer, m, re_signed, match_continuous)) {
            value.reset (new ubus::dbus_i32(((int32_t)std::stoi(m[0], nullptr, 0))));
            len.first = 1;
            len.second = m.length (0);
            ok = true;
        }
        break;

    case 'u': // UINT32 - Unsigned 32-bit integer
        if (std::regex_search(value_buffer, m, re_unsigned, match_continuous)) {
            value.reset (new ubus::dbus_u32(((uint32_t)std::stoi(m[0], nullptr, 0))));
            len.first = 1;
            len.second = m.length (0);
            ok = true;
        }
        break;

    case 'x': // INT64 - Signed 64-bit integer
        if (std::regex_search(value_buffer, m, re_signed, match_continuous)) {
            value.reset (new ubus::dbus_i64(((int64_t)std::stoi(m[0], nullptr, 0))));
            len.first = 1;
            len.second = m.length (0);
            ok = true;
        }
        break;

    case 't': // UINT64 - Unsigned 64-bit integer
        if (std::regex_search(value_buffer, m, re_unsigned, match_continuous)) {
            value.reset (new ubus::dbus_u64(((uint64_t)std::stoi(m[0], nullptr, 0))));
            len.first = 1;
            len.second = m.length (0);
            ok = true;
        }
        break;

    case 'd': // DOUBLE - IEEE 754 double-precision floating point
        if (std::regex_search(value_buffer, m, re_double, match_continuous)) {
            value.reset (new ubus::dbus_double(std::stod(m[0])));
            len.first = 1;
            len.second = m.length (0);
            ok = true;
        }
        break;

    case 'h': // UNIX_FD - Unsigned 32-bit integer
        if (std::regex_search(value_buffer, m, re_unsigned, match_continuous)) {
            value.reset (new ubus::dbus_unix_fd(std::stoi(m[0], nullptr, 0)));
            len.first = 1;
            len.second = m.length (0);
            ok = true;
        }
        break;

    case 's': // STRING - string
        if (std::regex_search(value_buffer, m, re_string, match_continuous)) {
            len.first = 1;
            len.second = m.length (0);
            value.reset (new ubus::dbus_string(unescape_escaped_str_value(m[0])));
            ok = true;
        }
        break;

    case 'o': // OBJECT_PATH - object path
        if (std::regex_search(value_buffer, m, re_opath, match_continuous)) {
            std::string opath = m[0];
            if (dbus_validate_path(opath.c_str(), nullptr)) {
                len.first = 1;
                len.second = m.length (0);
                value.reset (new ubus::dbus_opath(opath));
                ok = true;
            }
        }
        break;

    case 'g': // SIGNATURE - string
        if (std::regex_search(value_buffer, m, re_signature, match_continuous)) {
            std::string sig = m[0];
            if (dbus_signature_validate(sig.c_str(), nullptr)) {
                len.first = 1;
                len.second = m.length (0);
                value.reset (new ubus::dbus_signature(sig));
                ok = true;
            }
        }
        break;
    }

    return value;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::unique_ptr<ultrabus::dbus_type> dbus_arg_parser::parse_dbus_array_arg_string (
        const char* signature,
        const char* value_buffer,
        std::pair<size_t, size_t>& len,
        bool& ok)
{
    std::unique_ptr<ultrabus::dbus_type> value;

    ok = true;
    bool array_filled = false;

    len.first = signature_len (signature);
    std::string element_sig (signature+1, len.first-1);
    if (element_sig.empty()) {
        // Invalid element signature
        ok = false;
        return value;
    }

    value.reset (new ubus::dbus_array(element_sig));
    ubus::dbus_array& array = value->cast<ubus::dbus_array>();
    const char* buf_pos = value_buffer;

    // Start of array
    if (*buf_pos != '[') {
        ok = false;
        return value;
    }
    ++buf_pos;

    if (*buf_pos == ']') {
        array_filled = true; // Array empty
        ++buf_pos;
    }else if (*buf_pos == '\0') {
        ok = false;
    }

    while (ok && !array_filled) {
        auto sub_len = std::make_pair<size_t, size_t> (0, 0);
        auto element_ptr = parse_dbus_arg_string (element_sig.c_str(), buf_pos, sub_len);
        if (!element_ptr) {
            ok = false;
            break;
        }

        //array.add (*element_ptr);
        array.push_back (std::move(element_ptr));
        buf_pos += sub_len.second;

        if (*buf_pos == ',') {
            ++buf_pos;
        }
        else if (*buf_pos == ']') {
            ++buf_pos;
            array_filled = true;
        }
        else if (*buf_pos == '\0') {
            ok = false;
        }
    }

    len.second = buf_pos - value_buffer;
    return value;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::unique_ptr<ultrabus::dbus_type> dbus_arg_parser::parse_dbus_struct_arg_string (
        const char* signature,
        const char* value_buffer,
        std::pair<size_t, size_t>& len,
        bool& ok)
{
    std::unique_ptr<ultrabus::dbus_type> value (new ubus::dbus_struct);
    ubus::dbus_struct& s = value->cast<ubus::dbus_struct>();
    ok = true;

    const char* buf_pos = value_buffer;
    const char* sig_buf = signature + 1; // Skip '(' in signature

    if (*buf_pos != '{') {
        ok = false;
        return value;
    }
    ++buf_pos; // Skip '{' in value buffer

    while (true) {
        size_t sub_sig_len = signature_len (sig_buf);
        std::string sub_sig (sig_buf, sub_sig_len);

        auto sub_len = std::make_pair<size_t, size_t> (0, 0);
        auto sub_value = parse_dbus_arg_string (sub_sig.c_str(), buf_pos, sub_len);
        if (!sub_value) {
            ok = false;
            break;
        }

        //s.add (*sub_value); // Add a DBus value to the struct
        s.add (std::move(sub_value)); // Add a DBus value to the struct

        buf_pos += sub_len.second;
        sig_buf += sub_sig_len;
        if (*sig_buf == ')') {
            ++sig_buf; // Skip ')' in signature buffer
            break;
        }

        if (*buf_pos != ',') {
            ok = false;
            break;
        }
        ++buf_pos; // Skip ',' in value buffer
    }

    if (*buf_pos == '}')
        ++buf_pos; // Skip '}' in value buffer
    else
        ok = false;

    len.first = sig_buf - signature;
    len.second = buf_pos - value_buffer;

    return value;
}


//------------------------------------------------------------------------------
// Value of the variant holds the signature followed by an underscore.
// A variant holding the integer 32      : i_32
// A variant holding a string            : s_"hello_world"
// A variant holding an array of integers: ai_[1,2,3,4]
// A variant holding an array of variants: av_[i_1,s_"string",ai_[1,2,3]]
//------------------------------------------------------------------------------
std::unique_ptr<ultrabus::dbus_type> dbus_arg_parser::parse_dbus_variant_arg_string (
        const char* signature,
        const char* value_buffer,
        std::pair<size_t, size_t>& len,
        bool& ok)
{
    std::unique_ptr<ultrabus::dbus_type> value (new ubus::dbus_variant);
    ubus::dbus_variant& v = value->cast<ubus::dbus_variant>();

    const char* buf_pos = value_buffer;

    ok = true;
    len.first = 1; // Signature 'v'

    buf_pos += signature_len (buf_pos);
    if (buf_pos == value_buffer) {
        // Invalid value signature
        ok = false;
        return value;
    }
    std::string value_sig (value_buffer, buf_pos-value_buffer);
    if (*buf_pos != '_') {
        // Invalid value signature prefix
        ok = false;
        return value;
    }
    ++buf_pos; // Skip '_'

    auto sub_len = std::make_pair<size_t, size_t> (0, 0);
    auto sub_value = parse_dbus_arg_string (value_sig.c_str(), buf_pos, sub_len);
    if (sub_value) {
        //v.value (*sub_value);
        v = *sub_value;
        buf_pos += sub_len.second;
    }else{
        ok = false;
    }

    len.second = buf_pos - value_buffer;
    return value;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
std::unique_ptr<ultrabus::dbus_type> dbus_arg_parser::parse_dbus_dict_arg_string (
        const char* signature,
        const char* value_buffer,
        std::pair<size_t, size_t>& len,
        bool& ok)
{
    ok = true;

    len.first = signature_len (signature);
    std::string element_sig (signature+1, len.first-1);
    if (element_sig.empty()) {
        // Invalid element signature
        ok = false;
        return nullptr; //value;
    }
    int key_type_code = element_sig[1];
    std::string key_sig (1, element_sig[1]);
    std::string value_sig = element_sig.substr (2, element_sig.size()-3);

    std::unique_ptr<ultrabus::dbus_type> value = ubus::create_dbus_dict (key_type_code, value_sig);
    ubus::dbus_dict_base& dict = value->cast<ubus::dbus_dict_base>();

    bool dict_filled = false;
    const char* buf_pos = value_buffer;

    // Start of dict
    if (*buf_pos != '[') {
        ok = false;
        return value;
    }
    ++buf_pos;

    if (*buf_pos == ']') {
        dict_filled = true; // Dict empty
        ++buf_pos;
    }else if (*buf_pos == '\0') {
        ok = false;
    }

    while (ok && !dict_filled) {
        auto sub_len = std::make_pair<size_t, size_t> (0, 0);

        if (*buf_pos != '(') {
            ok = false;
            break;
        }
        ++buf_pos;
        if (*buf_pos == '\0') {
            ok = false;
            break;
        }


        auto key_ptr = parse_dbus_arg_string (key_sig.c_str(), buf_pos, sub_len);
        if (!key_ptr) {
            ok = false;
            break;
        }
        buf_pos += sub_len.second;
        if (*buf_pos == '\0'  ||  *buf_pos != ',') {
            ok = false;
            break;
        }
        ++buf_pos;
        if (*buf_pos == '\0') {
            ok = false;
            break;
        }

        sub_len.first = sub_len.second = 0;
        auto value_ptr = parse_dbus_arg_string (value_sig.c_str(), buf_pos, sub_len);
        if (!value_ptr) {
            ok = false;
            break;
        }
        buf_pos += sub_len.second;

        dict.set (*key_ptr, *value_ptr);

        if (*buf_pos != ')') {
            ok = false;
            break;
        }
        ++buf_pos;

        if (*buf_pos == ',') {
            ++buf_pos;
        }
        else if (*buf_pos == ']') {
            ++buf_pos;
            dict_filled = true;
        }
        else if (*buf_pos == '\0') {
            ok = false;
        }
    }

    len.second = buf_pos - value_buffer;
    return value;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static size_t signature_len (const char* signature)
{
    std::string sig;
    size_t pos = 0;

    while (signature[pos]) {
        sig.push_back (signature[pos++]);
        if (dbus_signature_validate(sig.c_str(), nullptr))
            return pos;
    }
    return 0;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static std::string unescape_escaped_str_value (const std::string& in)
{
    std::string result;
    auto pos = in.begin ();
    auto in_end = in.end ();
    --in_end; // Strip last " (or ')

    // Begin past first " (or ')
    while (++pos != in_end) {
        if (*pos == '\\') {
            ++pos;
            switch (*pos) {
            case '"':
                result.push_back ('"');
                break;
            case '\'':
                result.push_back ('\'');
                break;
            case '\\':
                result.push_back ('\\');
                break;
            case 'b':
                result.push_back ('\b');
                break;
            case 'f':
                result.push_back ('\f');
                break;
            case 'n':
                result.push_back ('\n');
                break;
            case 'r':
                result.push_back ('\r');
                break;
            case 't':
                result.push_back ('\t');
                break;
            }
        }else{
            result.push_back (*pos);
        }
    }

    return result;
}
