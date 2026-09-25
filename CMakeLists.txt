#
# Copyright (C) 2026 Dan Arrhenius <dan@ultramarin.se>
#
# This file is part of dbus-tool.
#
# dbus-tool is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
cmake_minimum_required (VERSION 3.25)

# Project name and info
#
project (dbus-tool
    VERSION 1.0.0
    LANGUAGES CXX
    DESCRIPTION "dbus-tool is a multipurpose command line utility for the DBus interprocess communication system."
)

set (PACKAGE_AUTHOR "Dan")
set (PACKAGE_BUGREPORT "dan@ultramarin.se")

# C++ standard
#
set (CMAKE_CXX_STANDARD 20)

# Default GNU installation directories
#
include (GNUInstallDirs)

# Common compiler flags
#
set (gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
add_compile_options ("$<${gcc_like_cxx}:$<BUILD_INTERFACE:-pipe;-Wall;-D_GNU_SOURCE>>")
if (CMAKE_BUILD_TYPE MATCHES "^[Dd]ebug")
    add_compile_options ("$<${gcc_like_cxx}:$<BUILD_INTERFACE:-Og;-Werror;-fsanitize=address>>")
    add_link_options ("$<${gcc_like_cxx}:$<BUILD_INTERFACE:-fsanitize=address;-static-libasan>>")
elseif (CMAKE_BUILD_TYPE STREQUAL "")
    add_compile_options ("$<${gcc_like_cxx}:$<BUILD_INTERFACE:-O2>>")
endif()


# Dependencies
#
find_package(ultrabus 1 REQUIRED)
if (ultrabus_FOUND)
    message (STATUS "Found ultrabus version ${ultrabus_VERSION}")
endif()

find_package(PkgConfig)
pkg_check_modules(libxml-2.0 REQUIRED IMPORTED_TARGET libxml-2.0)


# Configuration file
#
configure_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/src/config.hpp
)
configure_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dbus-tool.1.in
    ${CMAKE_CURRENT_BINARY_DIR}/src/dbus-tool.1
)


# Application dbus-tool
#
add_executable (dbus-tool)
target_sources (dbus-tool PRIVATE
    src/appargs_t.cpp
    src/dbus_arg_parser.cpp
    src/introspect_parser.cpp
    src/main.cpp
)
target_link_libraries (dbus-tool
    PRIVATE
    ultrabus::ultrabus
    PkgConfig::libxml-2.0
)
target_include_directories (dbus-tool
    PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src>
)


# Installation
#
install (TARGETS dbus-tool)

#
# Uninstall target
#
configure_file (
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
    @ONLY
)
add_custom_target (uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
    COMMENT "Uninstalling ultrabus"
)
