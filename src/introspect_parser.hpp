/*
 * Copyright (C) 2021,2023,2026 Dan Arrhenius <dan@ultramarin.se>
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
#ifndef INTROSPECT_PARSER_HPP
#define INTROSPECT_PARSER_HPP

#include <stdexcept>
#include <iostream>
#include <string>
#include <list>
#include <set>


/**
 *
 */
class introspect_parser {
public:

    struct arg_t {
        std::string name;
        std::string sig;
        arg_t () = default;
        arg_t (const std::string& name_arg, const std::string& sig_arg)
            : name(name_arg), sig(sig_arg)
        {}
    };

    struct method_t {
        std::string name;
        std::list<arg_t> in;
        arg_t out;
    };

    struct property_t {
        std::string name;
        std::string sig;
        std::string access;
    };

    struct iface_t {
        std::string name;
        std::list<method_t> methods;
        std::list<method_t> signals;
        std::list<property_t> props;
    };

    struct node_t {
        std::list<iface_t> ifaces;
        std::set<std::string> sub_nodes;
    };


    introspect_parser (bool skip_standard_interfaces_arg=false);
    node_t parse_xml (const std::string& xml);
    void print (const node_t& node,
                std::ostream& out=std::cout,
                bool json_output=false);

    void print (const node_t& node,
                const std::string& service,
                const std::string& object_path,
                std::ostream& out=std::cout,
                bool json_output=false);

private:
    bool skip_standard_interfaces;
    void print_json (const node_t& node,
                     const std::string& service,
                     const std::string& object_path,
                     std::ostream& out);
};


#endif
