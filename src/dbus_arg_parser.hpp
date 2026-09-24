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
#ifndef DBUS_ARG_PARSER_HPP
#define DBUS_ARG_PARSER_HPP

#include <ultrabus.hpp>
#include <string>
#include <memory>



/**
 *
 */
class dbus_arg_parser {
public:
    std::unique_ptr<ultrabus::dbus_type> operator() (
            const std::string& signature,
            const std::string& value);

    const std::string& error () {return error_msg;}


private:
    std::string error_msg;

    std::unique_ptr<ultrabus::dbus_type> parse_dbus_arg_string (
            const char* signature,
            const char* value_buffer,
            std::pair<size_t, size_t>& len);

    std::unique_ptr<ultrabus::dbus_type> parse_dbus_basic_arg_string (
            const char* signature,
            const char* value_buffer,
            std::pair<size_t, size_t>& len,
            bool& ok);
    std::unique_ptr<ultrabus::dbus_type> parse_dbus_array_arg_string (
            const char* signature,
            const char* value_buffer,
            std::pair<size_t, size_t>& len,
            bool& ok);
    std::unique_ptr<ultrabus::dbus_type> parse_dbus_dict_arg_string (
            const char* signature,
            const char* value_buffer,
            std::pair<size_t, size_t>& len,
            bool& ok);
    std::unique_ptr<ultrabus::dbus_type> parse_dbus_struct_arg_string (
            const char* signature,
            const char* value_buffer,
            std::pair<size_t, size_t>& len,
            bool& ok);
    std::unique_ptr<ultrabus::dbus_type> parse_dbus_variant_arg_string (
            const char* signature,
            const char* value_buffer,
            std::pair<size_t, size_t>& len,
            bool& ok);
};


#endif
