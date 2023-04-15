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
AC_INIT([dbus-tool], [0.0.0], [https://github.com/alfmep])
AM_INIT_AUTOMAKE([-Wall -Werror foreign dist-bzip2])

AC_PROG_CC
AC_PROG_CXX
AC_PROG_MKDIR_P
AC_PROG_INSTALL

LT_INIT()
LT_LANG([C++])

AC_CONFIG_MACRO_DIR([m4])
#AC_CONFIG_HEADERS(src/config.h src/dbus-tool_config.hpp)
AM_SILENT_RULES([yes])


#
# Check for ultrabus
#
PKG_CHECK_MODULES([ultrabus],
	[ultrabus],
	[],
	[AC_MSG_ERROR(Could not find ultrabus)])
AC_SUBST([ultrabus_CLAGS])
AC_SUBST([ultrabus_LIBS])


#
# Check for libxml2, used to parse dbus introspect data
#
AC_ARG_ENABLE([libxml2],
	[AS_HELP_STRING([--disable-libxml2], [don't use libxml2 - dbus-tool will not format introspect output [default=auto]])])
AM_CONDITIONAL([ENABLE_LIBXML2_SET], [test "x$enable_libxml2" != "xno"])

AM_COND_IF([ENABLE_LIBXML2_SET],
	[
	 PKG_CHECK_MODULES([libxml2],
		[libxml-2.0],
		[AM_CONDITIONAL([HAVE_LIBXML2], true)],
		[AC_MSG_WARN(Could not find libxml2 - dbus-tool will not format introspect output);
		 AM_CONDITIONAL([HAVE_LIBXML2], false)])
	],
	[AM_CONDITIONAL([HAVE_LIBXML2], false)]
)


AC_SUBST([libxml2_CFLAGS])
AC_SUBST([libxml2_LIBS])


AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/dbus-tool.1
])

AC_OUTPUT
