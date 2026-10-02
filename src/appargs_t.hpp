/*
 * Copyright (C) 2021-2023,2026 Dan Arrhenius <dan@ultramarin.se>
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
#ifndef APPARGS_T_HPP
#define APPARGS_T_HPP

#include <ultrabus.hpp>
#include <iostream>
#include <string>
#include <vector>


struct appargs_t {
    appargs_t (int argc, char* argv[]);

    DBusBusType bus;
    int timeout;

    std::string cmd;
    std::string service;
    std::string opath;
    std::string iface;
    std::string name;
    std::vector<std::string> args;

    struct opt_list {
        bool all;
        bool unique;
        bool activatable;
    };
    struct opt_call {
        bool json_output;
        bool print_signature;
    };
    struct opt_introspect {
        bool skip;
        bool json_output;
        bool raw;
    };
    struct opt_get {
        bool json_output;
        bool print_signature;
    };
    struct opt_objects {
        bool all;
        bool json_output;
    };
    struct opt_listen {
        bool recursive;
        bool json_output;
        bool print_signature;
    };
    struct opt_start {
        bool quiet;
    };
    struct opt_ping {
        bool quiet;
    };
    struct opt_monitor {
        bool eavesdrop;
    };

    opt_list list;
    opt_call call;
    opt_introspect introspect;
    opt_get get;
    opt_objects objects;
    opt_listen listen;
    opt_start start;
    opt_ping ping;
    opt_monitor monitor;
};



#endif
