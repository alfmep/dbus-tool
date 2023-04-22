#
# Copyright (C) 2023 Dan Arrhenius <dan@ultramarin.se>
#
# This file is part of dbus-tool
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

AM_CPPFLAGS = -I$(srcdir) -D_GNU_SOURCE -DSYSCONFDIR='"${sysconfdir}"' -DLOCALSTATEDIR='"${localstatedir}"'
AM_CXXFLAGS = -Wall -pipe -O2 -g
AM_CXXFLAGS += $(ultrabus_CFLAGS)
AM_LDFLAGS = $(ultrabus_LIBS)

if HAVE_LIBXML2
    AM_CXXFLAGS += $(libxml2_CFLAGS)
    AM_LDFLAGS  += $(libxml2_LIBS)
else
    AM_CXXFLAGS += -DNO_LIBXML2
endif


bin_PROGRAMS = dbus-tool
dbus_tool_SOURCES  =
dbus_tool_SOURCES += appargs_t.hpp
dbus_tool_SOURCES += appargs_t.cpp
dbus_tool_SOURCES += dbus_arg_parser.hpp
dbus_tool_SOURCES += dbus_arg_parser.cpp
dbus_tool_SOURCES += print_introspect.hpp
dbus_tool_SOURCES += print_introspect.cpp
dbus_tool_SOURCES += main.cpp


dist_man1_MANS  =
dist_man1_MANS += dbus-tool.1
