/*
 * Copyright (C) 2023 Dan Arrhenius <dan@ultramarin.se>
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
#include <regex>
#include <set>


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

#define RE_BOOLEAN "true|false|1|0"

#define RE_DIGIT "[0-9]"
#define RE_DIGITS RE_DIGIT "+"
#define RE_MINUS "[-]"
#define RE_SIGN "[+-]"
#define RE_FRAC "[.]" RE_DIGITS
#define RE_EXP "[eE]" RE_SIGN "?" RE_DIGITS

#define RE_UNSIGNED_INTEGER RE_DIGITS
#define RE_SIGNED_INTEGER RE_MINUS "?" RE_DIGITS
#define RE_DOUBLE RE_SIGNED_INTEGER "|" RE_SIGNED_INTEGER RE_FRAC "|" RE_SIGNED_INTEGER RE_EXP "|" RE_SIGNED_INTEGER RE_FRAC RE_EXP




static size_t signature_len (const char* signature, bool allow_dict_entry=false);
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
        std::pair<size_t, size_t>& len,
        bool allow_dict_entry_sig)
{
    using std::regex_constants::match_continuous;

    std::unique_ptr<ultrabus::dbus_type> value (nullptr);
    auto sub_len = std::make_pair<size_t, size_t> (0, 0);
    bool ok = true;

    if (!dbus_signature_validate(signature, nullptr)) {
        if (!allow_dict_entry_sig)
            return value;
        if (signature_len(signature, true) == 0)
            return value;
    }

    if (basic_type_signatures.find(signature[0]) != basic_type_signatures.end()) {
        // The signature is a basic value
        auto val = parse_dbus_basic_arg_string (signature, value_buffer, sub_len, ok);
        if (ok)
            value.reset (new ultrabus::dbus_basic(val));
    }
    else if (signature[0] == 'a') {
        // The signature is an array
        auto val = parse_dbus_array_arg_string (signature, value_buffer, sub_len, ok);
        if (ok)
            value.reset (new ultrabus::dbus_array(val));
    }
    else if (signature[0] == '(') {
        // The signature is a struct
        auto val = parse_dbus_struct_arg_string (signature, value_buffer, sub_len, ok);
        if (ok)
            value.reset (new ultrabus::dbus_struct(val));
    }
    else if (signature[0] == 'v') {
        // The signature is a variant
        auto val = parse_dbus_variant_arg_string (signature, value_buffer, sub_len, ok);
        if (ok)
            value.reset (new ultrabus::dbus_variant(val));
    }
    else if (signature[0] == '{') {
        // The signature is a dict entry
        auto val = parse_dbus_dict_entry_arg_string (signature, value_buffer, sub_len, ok);
        if (ok)
            value.reset (new ultrabus::dbus_dict_entry(val));
    }

    len.first += sub_len.first;
    len.second += sub_len.second;

    return value;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
ultrabus::dbus_basic dbus_arg_parser::parse_dbus_basic_arg_string (
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

    ultrabus::dbus_basic value;
    std::cmatch m;

    len.first = len.second = 0;
    ok = false;

    switch (signature[0]) {
    case 'y': // BYTE - Unsigned 8-bit integer
        if (std::regex_search(value_buffer, m, re_unsigned, match_continuous)) {
            value.byt ((uint8_t)std::stoi(m[0], nullptr, 0));
            len.first = 1;
            len.second = m.length (0);
            ok = true;
        }
        break;

    case 'b': // BOOLEAN - 0, 1, false, or true
        if (std::regex_search(value_buffer, m, re_boolean, match_continuous)) {
            if (m[0] == "true")
                value.boolean (true);
            else if (m[0] == "false")
                value.boolean (false);
            else
                value.boolean ((bool)std::stoi(m[0], nullptr, 0));
            len.first = 1;
            len.second = m.length (0);
            ok = true;
        }
        break;

    case 'n': // INT16 - Signed 16-bit integer
        if (std::regex_search(value_buffer, m, re_signed, match_continuous)) {
            value.i16 ((int16_t)std::stoi(m[0], nullptr, 0));
            len.first = 1;
            len.second = m.length (0);
            ok = true;
        }
        break;

    case 'q': // UINT16 - Unsigned 16-bit integer
        if (std::regex_search(value_buffer, m, re_unsigned, match_continuous)) {
            value.u16 ((uint16_t)std::stoi(m[0], nullptr, 0));
            len.first = 1;
            len.second = m.length (0);
            ok = true;
        }
        break;

    case 'i': // INT32 - Signed 32-bit integer
        if (std::regex_search(value_buffer, m, re_signed, match_continuous)) {
            value.i32 ((int32_t)std::stoi(m[0], nullptr, 0));
            len.first = 1;
            len.second = m.length (0);
            ok = true;
        }
        break;

    case 'u': // UINT32 - Unsigned 32-bit integer
        if (std::regex_search(value_buffer, m, re_unsigned, match_continuous)) {
            value.u32 ((uint32_t)std::stoi(m[0], nullptr, 0));
            len.first = 1;
            len.second = m.length (0);
            ok = true;
        }
        break;

    case 'x': // INT64 - Signed 64-bit integer
        if (std::regex_search(value_buffer, m, re_signed, match_continuous)) {
            value.i64 ((int64_t)std::stoll(m[0], nullptr, 0));
            len.first = 1;
            len.second = m.length (0);
            ok = true;
        }
        break;

    case 't': // UINT64 - Unsigned 64-bit integer
        if (std::regex_search(value_buffer, m, re_unsigned, match_continuous)) {
            value.u64 ((uint64_t)std::stoll(m[0], nullptr, 0));
            len.first = 1;
            len.second = m.length (0);
            ok = true;
        }
        break;

    case 'd': // DOUBLE - IEEE 754 double-precision floating point
        if (std::regex_search(value_buffer, m, re_double, match_continuous)) {
            value.dbl (std::stod(m[0]));
            len.first = 1;
            len.second = m.length (0);
            ok = true;
        }
        break;

    case 'h': // UNIX_FD - Unsigned 32-bit integer
        if (std::regex_search(value_buffer, m, re_unsigned, match_continuous)) {
            value.fd (std::stoi(m[0], nullptr, 0));
            len.first = 1;
            len.second = m.length (0);
            ok = true;
        }
        break;

    case 's': // STRING - string
        if (std::regex_search(value_buffer, m, re_string, match_continuous)) {
            len.first = 1;
            len.second = m.length (0);
            value.str (unescape_escaped_str_value(m[0]));
            ok = true;
        }
        break;

    case 'o': // OBJECT_PATH - string
        if (std::regex_search(value_buffer, m, re_string, match_continuous)) {
            auto opath = unescape_escaped_str_value (m[0]);
            if (dbus_validate_path(opath.c_str(), nullptr)) {
                len.first = 1;
                len.second = m.length (0);
                value.set_opath (opath);
                ok = true;
            }
        }
        break;

    case 'g': // SIGNATURE - string
        if (std::regex_search(value_buffer, m, re_string, match_continuous)) {
            auto sig = unescape_escaped_str_value (m[0]);
            if (dbus_signature_validate(sig.c_str(), nullptr)) {
                len.first = 1;
                len.second = m.length (0);
                value.set_sig (sig);
                ok = true;
            }
        }
        break;
    }

    return value;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
ultrabus::dbus_array dbus_arg_parser::parse_dbus_array_arg_string (
        const char* signature,
        const char* value_buffer,
        std::pair<size_t, size_t>& len,
        bool& ok)
{
    ok = true;
    bool array_filled = false;

    len.first = signature_len (signature, true);
    std::string element_sig (signature+1, len.first-1);

    const char* buf_pos = value_buffer;
    ultrabus::dbus_array array (element_sig);

    if (element_sig.empty()) {
        // Invalid element signature
        ok = false;
        return array;
    }

    // Start of array
    if (*buf_pos != '[') {
        ok = false;
        return array;
    }
    ++buf_pos;

    if (*buf_pos == ']')
        array_filled = true; // Array empty
    else if (*buf_pos == '\0')
        ok = false;

    while (ok && !array_filled) {
        auto sub_len = std::make_pair<size_t, size_t> (0, 0);
        auto element_ptr = parse_dbus_arg_string (element_sig.c_str(), buf_pos, sub_len, true);
        if (!element_ptr) {
            ok = false;
            break;
        }

        array.add (*element_ptr);
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
    return array;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
ultrabus::dbus_struct dbus_arg_parser::parse_dbus_struct_arg_string (
        const char* signature,
        const char* value_buffer,
        std::pair<size_t, size_t>& len,
        bool& ok)
{
    ultrabus::dbus_struct s;
    ok = true;

    const char* buf_pos = value_buffer;
    const char* sig_buf = signature + 1; // Skip '(' in signature

    if (*buf_pos != '(') {
        ok = false;
        return s;
    }
    ++buf_pos; // Skip '(' in value buffer

    while (true) {
        size_t sub_sig_len = signature_len (sig_buf);
        std::string sub_sig (sig_buf, sub_sig_len);

        auto sub_len = std::make_pair<size_t, size_t> (0, 0);
        auto sub_value = parse_dbus_arg_string (sub_sig.c_str(), buf_pos, sub_len);
        if (!sub_value) {
            ok = false;
            break;
        }

        s.add (*sub_value); // Add a DBus value to the struct

        buf_pos += sub_len.second;
        sig_buf += sub_sig_len;
        if (*sig_buf == ')') {
            ++sig_buf; // Skip '(' in signature buffer
            break;
        }

        if (*buf_pos != ',') {
            ok = false;
            break;
        }
        ++buf_pos; // Skip ',' in value buffer
    }

    if (*buf_pos == ')')
        ++buf_pos; // Skip '(' in value buffer
    else
        ok = false;

    len.first = sig_buf - signature;
    len.second = buf_pos - value_buffer;

    return s;
}


//------------------------------------------------------------------------------
// Value of the variant holds the signature followed by an underscore.
// A variant holding the integer 32      : i_32
// A variant holding a string            : s_"hello_world"
// A variant holding an array of integers: ai_[1,2,3,4]
// A variant holding an array of variants: av_[i_1,s_"string",ai_[1,2,3]]
//------------------------------------------------------------------------------
ultrabus::dbus_variant dbus_arg_parser::parse_dbus_variant_arg_string (
        const char* signature,
        const char* value_buffer,
        std::pair<size_t, size_t>& len,
        bool& ok)
{
    const char* buf_pos = value_buffer;
    ultrabus::dbus_variant v;

    ok = true;
    len.first = 1; // Signature 'v'

    buf_pos += signature_len (buf_pos);
    if (buf_pos == value_buffer) {
        // Invalid value signature
        ok = false;
        return v;
    }
    std::string value_sig (value_buffer, buf_pos-value_buffer);
    if (*buf_pos != '_') {
        // Invalid value signature prefix
        ok = false;
        return v;
    }
    ++buf_pos; // Skip '_'

    auto sub_len = std::make_pair<size_t, size_t> (0, 0);
    auto sub_value = parse_dbus_arg_string (value_sig.c_str(), buf_pos, sub_len);
    if (sub_value) {
        v.value (*sub_value);
        buf_pos += sub_len.second;
    }else{
        ok = false;
    }

    len.second = buf_pos - value_buffer;
    return v;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
ultrabus::dbus_dict_entry dbus_arg_parser::parse_dbus_dict_entry_arg_string (
        const char* signature,
        const char* value_buffer,
        std::pair<size_t, size_t>& len,
        bool& ok)
{
    ultrabus::dbus_dict_entry d;
    ok = true;

    const char* buf_pos = value_buffer;
    const char* sig_buf = signature + 1; // Skip '{' in signature buffer
    auto sub_len = std::make_pair<size_t, size_t> (0, 0);

    if (*buf_pos != '{') {
        ok = false;
        return d;
    }
    ++buf_pos; // Skip '{' in value buffer

    // Get the signature of the key -- it must be a basic type !!!
    //
    if (basic_type_signatures.find(*sig_buf) == basic_type_signatures.end()) {
        ok = false;
        return d;
    }
    std::string key_signature (sig_buf++, 1);

    // Get the signature of the value
    //
    size_t sub_sig_len = signature_len (sig_buf);
    std::string value_signature (sig_buf, sub_sig_len);
    sig_buf += sub_sig_len;
    ++sig_buf; // Skip '}' in signature buffer

    // Get the key
    //
    auto key = parse_dbus_arg_string (key_signature.c_str(), buf_pos, sub_len);
    if (!key) {
        ok = false;
        return d;
    }
    d.key (*key);
    buf_pos += sub_len.second;

    if (*buf_pos != ',') {
        ok = false;
        return d;
    }
    ++buf_pos; // Skip ',' in value buffer

    // Get the value
    //
    sub_len.first = sub_len.second = 0;
    auto value = parse_dbus_arg_string (value_signature.c_str(), buf_pos, sub_len);
    if (!value) {
        ok = false;
        return d;
    }
    d.value (*value);
    buf_pos += sub_len.second;

    if (*buf_pos != '}') {
        ok = false;
        return d;
    }
    ++buf_pos; // Skip '}' in value buffer

    len.first = sig_buf - signature;
    len.second = buf_pos - value_buffer;

    return d;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static size_t signature_len (const char* signature, bool allow_dict_entry)
{
    std::string sig;
    size_t pos = 0;

    if (*signature == '{' && allow_dict_entry) {
        // Special case: a dict entry
        const char* sig_buf = signature + 1; // Skip beginning '{'
        size_t sub_pos;

        sub_pos = signature_len (sig_buf, false); // Signature of the key
        sig_buf += sub_pos;
        if (sub_pos == 0)
            return 0;

        sub_pos = signature_len (sig_buf, false); // Signature of the value
        sig_buf += sub_pos;
        if (sub_pos == 0  ||  *sig_buf != '}')
            return 0;
        ++sig_buf; // Skip trailing '}'

        return sig_buf - signature;
    }

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
