/*
 * Copyright (C) 2021-2023 Dan Arrhenius <dan@ultramarin.se>
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
    void print_usage_and_exit (std::ostream& out, int exit_code);

    DBusBusType bus;
    std::string bus_address;
    int timeout;

    std::string cmd;
    std::string service;
    std::string opath;
    std::string iface;
    std::string name;
    bool all;
    bool activatable;
    bool print_signature;
    bool quiet;
    bool raw;
    std::vector<std::string> args;
};



#endif
