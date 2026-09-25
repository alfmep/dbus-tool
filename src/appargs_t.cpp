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
#include <functional>
#include <map>
#include <unistd.h>
#include <getopt.h>

#include "appargs_t.hpp"
#include "config.hpp"

#ifdef __GLIBC__
#define PROGRAM_NAME program_invocation_short_name
#else
#include <stdlib.h>
#define PROGRAM_NAME getprogname()
#endif

using namespace std;

namespace {
    static constexpr const char* prog_name = "dbus-tool";

    using help_command_t = std::function<void()>;

    void print_help ();
    void print_help_list ();
    void print_help_call ();
    void print_help_introspect ();
    void print_help_get ();
    void print_help_set ();
    void print_help_objects ();
    void print_help_listen ();
    void print_help_start ();
    void print_help_owner ();
    void print_help_names ();
    void print_help_ping ();
    void print_help_monitor ();
    void print_help_signal ();

    std::map<std::string, help_command_t> help_commands = {
        {"list", print_help_list},
        {"call", print_help_call},
        {"introspect", print_help_introspect},
        {"get", print_help_get},
        {"set", print_help_set},
        {"objects", print_help_objects},
        {"listen", print_help_listen},
        {"start", print_help_start},
        {"owner", print_help_owner},
        {"names", print_help_names},
        {"ping", print_help_ping},
        {"monitor", print_help_monitor},
        {"signal", print_help_signal},
    };


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void print_common_options ()
    {
        cout << "  -y, --system       Connect to the system bus instead of the session bus." << endl;
        cout << "  -t, --timeout=MS   Set a specific timeout in milliseconds for DBus replies." << endl;
        cout << "  -v, --version      Print version and exit." << endl;
        cout << "  -h, --help         Print help message and exit." << endl;
        cout << "                     If a command is given, print help on that specific command." << endl;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void print_help ()
    {
        cout << endl;
        cout << "Usage: " << PROGRAM_NAME << " [OPTIONS] <command> [command argument ...]" << endl;
        cout << endl;
        cout << "Common options:" << endl;
        print_common_options ();
        cout << "Commands:" << endl;
        cout << "  list        List the bus names(services) on the bus." << endl;
        cout << "  call        Call a specific method on an object in a DBus service." << endl;
        cout << "  introspect  Print introspect data for a specific object in a DBus service." << endl;
        cout << "  get         Get the property of an object in a DBus service." << endl;
        cout << "  set         Set the property of an object in a DBus service." << endl;
        cout << "  objects     List all objects beloning to a specific service." << endl;
        cout << "  listen      Listen for DBus signals." << endl;
        cout << "  start       Launch the executable associated with a service name." << endl;
        cout << "  owner       Print the unique bus name of the primary owner of a service name." << endl;
        cout << "  names       Print the names acquired by a bus connection." << endl;
        cout << "  ping        Ping a service." << endl;
        cout << "  monitor     Monitor messages on the message bus." << endl;
        cout << "  signal      Send a DBus signal." << endl;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void print_help_list ()
    {
        cout << endl;
        cout << "Usage: " << PROGRAM_NAME << " list [OPTIONS]" << endl;
        cout << endl;
        cout << "  List the bus names(services) on the bus." << endl;
        cout << endl;
        cout << "Options:" << endl;
        cout << "  -a, --all          Also include unique bus names." << endl;
        cout << "  -x, --activatable  Instead of only already connected bus names," << endl;
        cout << "                     also list all names that can be activated on the bus." << endl;
        print_common_options ();
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void print_help_call ()
    {
        cout << endl;
        cout << "Usage: " << PROGRAM_NAME << " call [OPTIONS] <service> <object_path> <interface> <method> [signature argument ...]" << endl;
        cout << endl;
        cout << "  Call a specific method on an object in a DBus service." << endl;
        cout << "  Any returned argument is printed to standard output." << endl;
        cout << "  Arguments to the method begins with a DBus signature," << endl;
        cout << "  followed by the argument value. If there is only a " << endl;
        cout << "  single argument, the signature can be omitted if the" << endl;
        cout << "  argument is a boolean(true|false), a string, or a" << endl;
        cout << "  signed 32-bit integer." << endl;
        cout << endl;
        cout << "Options:" << endl;
        cout << "  -j, --json         Print the DBus message reply in JSON format." << endl;
        cout << "  -s, --signature    If not JSON output, when printing the reply" << endl;
        cout << "                     arguments, also print the DBus signature of" << endl;
        cout << "                     the arguments." << endl;
        print_common_options ();
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void print_help_introspect ()
    {
        cout << endl;
        cout << "Usage: " << PROGRAM_NAME << " introspect [OPTIONS] <service> [object_path]" << endl;
        cout << endl;
        cout << "  Print introspect data for a specific object in a DBus service." << endl;
        cout << "  If the object_path arguments is omitted, the root object \"/\" is used." << endl;
        cout << endl;
        cout << "Options:" << endl;
        cout << "  -s, --skip         Skip output of standard DBus interfaces." << endl;
        cout << "  -j, --json         Print the introspect data in JSON format." << endl;
        cout << "  -r, --raw          Don't parse the introspect data, print it \"as is\"." << endl;
        cout << "                     This parameter invalidates parameters -s and -j." << endl;
        print_common_options ();
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void print_help_get ()
    {
        cout << endl;
        cout << "Usage: " << PROGRAM_NAME << " get [OPTIONS] <service> <object_path> <interface> [property]" << endl;
        cout << endl;
        cout << "  Get the property of an object in a DBus service." << endl;
        cout << "  If argument 'property' is omitted, the names and values" << endl;
        cout << "  of all properties are printed to standard output." << endl;
        cout << endl;
        cout << "Options:" << endl;
        cout << "  -j, --json         Print the DBus property value(s) in JSON format." << endl;
        cout << "  -s, --signature    If not JSON output, print the DBus signature of" << endl;
        cout << "                     each property before the value." << endl;
        print_common_options ();
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void print_help_set ()
    {
        cout << endl;
        cout << "Usage: " << PROGRAM_NAME << " set [OPTIONS] <service> <object_path> <interface> <property> [value_signature] <value>" << endl;
        cout << endl;
        cout << "  Set the property of an object in a DBus service." << endl;
        cout << "  The signature of the value can omitted if the value" << endl;
        cout << "  is a boolean(true|false), string, or a signed integer." << endl;
        cout << endl;
        cout << "Options:" << endl;
        print_common_options ();
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void print_help_objects ()
    {
        cout << endl;
        cout << "Usage: " << PROGRAM_NAME << " objects [OPTIONS] <service> [object_path]" << endl;
        cout << endl;
        cout << "  List all objects beloning to a specific service at an object path." << endl;
        cout << "  If the object_path arguments is omitted, the root object \"/\" is used." << endl;
        cout << endl;
        cout << "Options:" << endl;
        cout << "  -a, --all          For each object, list interface and property names." << endl;
        print_common_options ();
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void print_help_listen ()
    {
        cout << endl;
        cout << "Usage: " << PROGRAM_NAME << " listen [OPTIONS] [service] [object_path] [interface] [signal-name]" << endl;
        cout << endl;
        cout << "  Listen for DBus signals." << endl;
        cout << "  Stop listening and exit the program by pressing Ctrl-C." << endl;
        cout << "  When a signal is received, it is printed to standard output." << endl;
        cout << "  Any argument may be an empty string or omitted, in which case" << endl;
        cout << "  it is treated as a wild card." << endl;
        cout << endl;
        cout << "Options:" << endl;
        cout << "  -r, --recursive    If an object path is specified, listen for" << endl;
        cout << "                     signals on all of its sub-paths also." << endl;
        cout << "  -j, --json         Print the DBus signals in JSON format." << endl;
        cout << "  -s, --signature    If not JSON output, when printing the signal" << endl;
        cout << "                     arguments, also print the DBus signature of" << endl;
        cout << "                     the arguments." << endl;
        print_common_options ();
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void print_help_start ()
    {
        cout << endl;
        cout << "Usage: " << PROGRAM_NAME << " start [OPTIONS] <service>" << endl;
        cout << endl;
        cout << "  Launch the executable associated with a service name." << endl;
        cout << endl;
        cout << "Options:" << endl;
        cout << "  -q, --quiet        Suppress output and exit quietly with 0 on success and 1 on failure." << endl;
        print_common_options ();
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void print_help_owner ()
    {
        cout << endl;
        cout << "Usage: " << PROGRAM_NAME << " owner [OPTIONS] <service>" << endl;
        cout << endl;
        cout << "  Print the unique bus name of the primary owner of the service name." << endl;
        cout << endl;
        cout << "Options:" << endl;
        print_common_options ();
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void print_help_names ()
    {
        cout << endl;
        cout << "Usage: " << PROGRAM_NAME << " names [OPTIONS] <bus-name>" << endl;
        cout << endl;
        cout << "  Print the names acquired by the bus connection." << endl;
        cout << endl;
        cout << "Options:" << endl;
        print_common_options ();
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void print_help_ping ()
    {
        cout << endl;
        cout << "Usage: " << PROGRAM_NAME << " ping [OPTIONS] <service>" << endl;
        cout << endl;
        cout << "  Ping a service on the bus and print the response time in milliseconds." << endl;
        cout << endl;
        cout << "Options:" << endl;
        cout << "  -q, --quiet        Suppress output and exit quietly with 0 on success and 1 on failure." << endl;
        print_common_options ();
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void print_help_monitor ()
    {
        cout << endl;
        cout << "Usage: " << PROGRAM_NAME << " monitor [OPTIONS]" << endl;
        cout << endl;
        cout << "  Monitor messages on the message bus and display them on standard output in JSON format." << endl;
        cout << endl;
        cout << "Options:" << endl;
        cout << "  -e, --eavesdrop    Use an eavesdrop DBus match match rule instead" << endl;
        cout << "                     of trying to call method BecomeMonitor." << endl;
        print_common_options ();
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    void print_help_signal ()
    {
        cout << endl;
        cout << "Usage: " << PROGRAM_NAME << " signal [OPTIONS] <service> <object_path> <interface> <signal> [signature argument ...]" << endl;
        cout << endl;
        cout << "  Send a DBus signal." << endl;
        cout << "  This command will connect to the DBus and acquire the specified service name." << endl;
        cout << "  Then it will send a signal with the specified object path and interface." << endl;
        cout << "  Arguments to the signals begins with a DBus signature, then the argument value." << endl;
        cout << "  If there is only a single argument, the signature can be omitted if the argument" << endl;
        cout << "  is a boolean(true|false), string, or a signed integer." << endl;
        cout << endl;
        cout << "Options:" << endl;
        print_common_options ();
    }


} // Anonymous namespace



//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
appargs_t::appargs_t (int argc, char* argv[])
    : bus {DBUS_BUS_SESSION},
      timeout {DBUS_TIMEOUT_USE_DEFAULT},
      all {false},
      activatable {false},
      print_signature {false},
      quiet {false},
      raw {false},
      recursive {false},
      eavesdrop {false},
      json_output {false}
{
    static struct option long_options[] = {
        { "system",      no_argument,       0, 'y'},
        { "json",        no_argument,       0, 'j'},
        { "bus",         required_argument, 0, 'b'},
        { "timeout",     required_argument, 0, 't'},
        { "all",         no_argument,       0, 'a'},
        { "activatable", no_argument,       0, 'x'},
        { "signature",   no_argument,       0, 's'},
        { "skip",        no_argument,       0, 's'},
        { "quiet",       no_argument,       0, 'q'},
        { "raw",         no_argument,       0, 'r'},
        { "recursive",   no_argument,       0, 'r'},
        { "eavesdrop",   no_argument,       0, 'e'},
        { "version",     no_argument,       0, 'v'},
        { "help",        no_argument,       0, 'h'},
        { 0, 0, 0, 0}
    };
    static const char* arg_format = "yjb:t:axsqrevh";
    bool be_quiet = false;
    bool help_needed = false;

    while (true) {
        int c = getopt_long (argc, argv, arg_format, long_options, nullptr);
        if (c == -1)
            break;
        switch (c) {
        case 'y':
            bus = DBUS_BUS_SYSTEM;
            break;
        case 'j':
            json_output = true;
            break;
        case 'a':
            all = true;
            break;
        // case 'b':
        //     bus_address = std::string (optarg);
        //     break;
        case 't':
            timeout = atoi (optarg);
            if (timeout <= 0) {
                cerr << "Error: Invalid timeout argument" << endl;
                exit (1);
            }
            break;
        case 'x':
            activatable = true;
            break;
        case 's':
            print_signature = true;
            skip = true;
            break;
        case 'q':
            be_quiet = true;
            break;
        case 'r':
            raw = true;
            recursive = true;
            break;
        case 'e':
            eavesdrop = true;
            break;
        case 'v': // --version
            cout << DBUS_TOOL_PACKAGE_STRING << std::endl;
            exit (0);
            break;
        case 'h': // --help
            help_needed = true;
            break;
        default:
            cerr << "Invalid option (--help for help)" << endl;
            break;
        }
    }
    if (help_needed) {
        if (optind >= argc) {
            print_help ();
        }else{
            cmd = argv[optind++];
            auto help_cmd = help_commands.find (cmd);
            if (help_cmd == help_commands.end()) {
                cerr << "Error: Unknown command (-h for help)." << endl;
                exit (1);
            }
            help_cmd->second ();
        }
        cout << endl;
        exit (0);
    }

    if (optind >= argc) {
        cerr << "Error: missing command (--help for help)" << endl;
        exit (1);
    }

    cmd = argv[optind++];

    if (cmd == "list") {
        ;
    }
    else if (cmd == "call") {
        if (optind > argc-4) {
            cerr << "Error: too few arguments (--help for help)" << endl;
            exit (1);
        }
        service = argv[optind++];
        opath   = argv[optind++];
        iface   = argv[optind++];
        name    = argv[optind++]; // method name
        while (optind < argc)
            args.emplace_back (argv[optind++]);
    }
    else if (cmd == "introspect") {
        if (optind > argc-1) {
            cerr << "Error: too few arguments (--help for help)" << endl;
            exit (1);
        }
        service = argv[optind++];
        if (optind < argc)
            opath = argv[optind++];
        else
            opath = "/";
    }
    else if (cmd == "get") {
        if (optind > argc-3) {
            cerr << "Error: too few arguments (--help for help)" << endl;
            exit (1);
        }
        service = argv[optind++];
        opath   = argv[optind++];
        iface   = argv[optind++]; // property interface
        if (optind < argc)
            name = argv[optind++]; // property name
    }
    else if (cmd == "set") {
        if (optind > argc-5) {
            cerr << "Error: too few arguments (--help for help)" << endl;
            exit (1);
        }
        service = argv[optind++];
        opath   = argv[optind++];
        iface   = argv[optind++]; // property interface
        name    = argv[optind++]; // property name
        args.emplace_back (argv[optind++]); // signature or value
        if (optind < argc)
            args.emplace_back (argv[optind++]); // value
    }
    else if (cmd == "objects") {
        if (optind > argc-1) {
            cerr << "Error: too few arguments (--help for help)" << endl;
            exit (1);
        }
        service = argv[optind++];
        if (optind < argc)
            opath = argv[optind++];
        else
            opath = "/";
    }
    else if (cmd == "listen") {
        if (optind < argc)
            service = argv[optind++];
        if (optind < argc)
            opath   = argv[optind++];
        if (optind < argc)
            iface   = argv[optind++];
        if (optind < argc)
            name = argv[optind++]; // A specific signal name
        else
            name = ""; // Any signal name
    }
    else if (cmd == "start") {
        quiet = be_quiet;
        if (optind > argc-1) {
            cerr << "Error: too few arguments (--help for help)" << endl;
            exit (1);
        }
        service = argv[optind++];
    }
    else if (cmd == "owner") {
        if (optind > argc-1) {
            cerr << "Error: too few arguments (--help for help)" << endl;
            exit (1);
        }
        service = argv[optind++];
    }
    else if (cmd == "names") {
        if (optind > argc-1) {
            cerr << "Error: too few arguments (--help for help)" << endl;
            exit (1);
        }
        service = argv[optind++];
    }
    else if (cmd == "ping") {
        quiet = be_quiet;
        if (optind > argc-1) {
            cerr << "Error: too few arguments (--help for help)" << endl;
            exit (1);
        }
        service = argv[optind++];
    }
    else if (cmd == "monitor") {
        ;
    }
    else if (cmd == "signal") {
        if (optind > argc-4) {
            cerr << "Error: too few arguments (--help for help)" << endl;
            exit (1);
        }
        service = argv[optind++];
        opath   = argv[optind++];
        iface   = argv[optind++];
        name    = argv[optind++]; // signal name
        while (optind < argc)
            args.emplace_back (argv[optind++]);
    }

    if (optind < argc) {
        cerr << "Error: too many arguments (--help for help)" << endl;
        exit (1);
    }

    // Strip trailing '/' in the object path argument if it's not the root node
    if (opath.length() > 1  &&  opath.back() == '/')
        opath.pop_back ();
}
