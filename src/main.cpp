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
#include <ultrabus.hpp>
#include <functional>
#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <condition_variable>
#include <mutex>
#include <cstring>
#include <unistd.h>
#include <signal.h>

#include "appargs_t.hpp"
#include "dbus_arg_parser.hpp"
#include "introspect_parser.hpp"

namespace ubus = ultrabus;
using namespace std;


namespace {
    using command_t = std::function<bool (ubus::connection&, appargs_t&)>;

    bool list_services (ubus::connection& conn, appargs_t& opt);
    bool call_method (ubus::connection& conn, const appargs_t& opt);
    bool introspect (ubus::connection& conn, const appargs_t& opt);
    bool get_property (ubus::connection& conn, const appargs_t& opt);
    bool set_property (ubus::connection& conn, const appargs_t& opt);
    bool objects (ubus::connection& conn, const appargs_t& opt);
    bool listen_for_signals (ubus::connection& conn, const appargs_t& opt);
    bool start_service (ubus::connection& conn, appargs_t& opt);
    bool print_owner (ubus::connection& conn, appargs_t& opt);
    bool print_names (ubus::connection& conn, appargs_t& opt);
    bool ping (ubus::connection& conn, appargs_t& opt);
    bool monitor (ubus::connection& conn, appargs_t& opt);
    bool send_signal (ubus::connection& conn, appargs_t& opt);

    std::unique_ptr<ubus::dbus_type> get_single_message_argument (const std::string& arg);

    std::map<std::string, command_t> commands = {
        {"list", list_services},
        {"call", call_method},
        {"introspect", introspect},
        {"get", get_property},
        {"set", set_property},
        {"objects", objects},
        {"listen", listen_for_signals},
        {"start", start_service},
        {"owner", print_owner},
        {"names", print_names},
        {"ping", ping},
        {"monitor", monitor},
        {"signal", send_signal},
    };


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    std::condition_variable cv;
    std::mutex m;
    volatile bool stop_sleep_loop = false;
    void stop_signal_handler (int sig)
    {
        stop_sleep_loop = true;
        cv.notify_one ();
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool list_services (ubus::connection& conn, appargs_t& opt)
    {
        auto names = opt.activatable ? conn.list_activatable_names(opt.timeout) : conn.list_names(opt.timeout);
        if (names.err()) {
            cerr << names.what() << endl;
            return false;
        }
        for (auto& name : names.get()) {
            if (opt.all || name[0]!=':')
                cout << name << endl;
        }
        return true;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    std::unique_ptr<ubus::dbus_type> get_single_message_argument (const std::string& arg)
    {
        std::unique_ptr<ubus::dbus_type> retval;

        if (strcasecmp(arg.c_str(), "true") == 0) {
            // Boolean true
            retval.reset (new ubus::dbus_bool(true));
        }
        else if (strcasecmp(arg.c_str(), "false") == 0) {
            // Boolean false
            retval.reset (new ubus::dbus_bool(false));
        }
        else {
            try {
                // Try adding the argument as a signed 32-bit integer...
                retval.reset (new ubus::dbus_i32(std::stoi(arg, nullptr, 0)));
            }
            catch (...) {
                // ...no? Add the argument as a string
                retval.reset (new ubus::dbus_string(arg));
            }
        }
        return retval;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool call_method (ubus::connection& conn, const appargs_t& opt)
    {
        ubus::object_proxy op (conn, opt.service, opt.opath, opt.iface);
        ubus::message msg (opt.service, opt.opath, opt.iface, opt.name);

        auto num_args = opt.args.size ();
        if (num_args == 0) {
            ; // No arguments to the method call
        }
        else if (num_args == 1) {
            msg.append_args (*get_single_message_argument(opt.args[0]));
        }
        else if (num_args & 0x01) {
            // Un-even number of arguments
            std::cerr << "Error: Invalid method call argument format, missing signature or value." << std::endl;
            return false;
        }
        else{
            dbus_arg_parser p;
            for (size_t i=0; i<num_args; i+=2) {
                auto value = p (opt.args[i], opt.args[i+1]);
                if (value) {
                    msg.append_args (*value);
                }else{
                    std::cerr << "Error: Invalid argument format." << std::endl;
                    return false;
                }
            }
        }

        auto reply = conn.send_and_wait (msg, opt.timeout);
        if (opt.json_output) {
            cout << reply.to_json() << endl;
        }else{
            if (reply.is_error()) {
                cerr << "Error: " << reply.error_name() << " - " << reply.error_msg() << endl;
            }else{
                auto args = reply.arguments ();
                for (auto& arg : args) {
                    if (opt.print_signature)
                        cout << arg->signature() << ' ' << arg->to_string() << endl;
                    else
                        cout << arg->to_string() << endl;
                }
            }
        }
        return reply.is_error() == false;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool introspect (ubus::connection& conn, const appargs_t& opt)
    {
        ubus::org_freedesktop_DBus_Introspectable is (conn);
        auto xml_doc = is.introspect (opt.service, opt.opath, opt.timeout);
        if (xml_doc.err()) {
            cerr << "Error: " << xml_doc.what() << endl;
        }
        else if (opt.raw) {
            cout << xml_doc.get() << endl;
        }else{
            introspect_parser ip (opt.skip);
            ip.print (ip.parse_xml(xml_doc), opt.service, opt.opath, cout, opt.json_output);
        }
        return xml_doc.err() == 0;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool get_property (ubus::connection& conn, const appargs_t& opt)
    {
        ubus::org_freedesktop_DBus_Properties properties (conn);

        if (!opt.name.empty()) {
            //
            // Get a specific property
            //
            auto result = properties.get (opt.service, opt.opath, opt.iface, opt.name, opt.timeout);
            if (result.err()) {
                cerr << "Error: " << result.what() << endl;
                return false;
            }
            ubus::dbus_type& value = result.get().get ();
            if (opt.json_output) {
                cout << value.to_json() << endl;
            }else if (opt.print_signature) {
                cout << value.signature() << ' ' << value.to_string() << endl;
            }else{
                cout << value.to_string() << endl;
            }
        }else{
            //
            // Get all properties
            //
            auto retval = properties.get_all (opt.service, opt.opath, opt.iface);
            if (retval.err()) {
                cerr << "Error: " << retval.what() << endl;
                return false;
            }

            const auto& props = retval.get ();

            if (opt.json_output) {
                cout << props.to_json() << endl;
                return true;
            }
            // Make a nice output format
            size_t max_width = 1;
            size_t max_sig_width = 1;
            for (const auto& prop : props) {
                const ubus::dbus_string& key = prop.first;
                size_t len = key.get().size ();
                if (len > max_width)
                    max_width = len;
                if (opt.print_signature) {
                    const ubus::dbus_type& value = prop.second.is_variant() ?
                        prop.second.cast<ubus::dbus_variant>().get() :
                        prop.second;
                    len = value.signature().size ();
                    if (len > max_sig_width)
                        max_sig_width = len;
                }
            }
            for (const auto& prop : props) {
                const ubus::dbus_string& key = prop.first;
                const ubus::dbus_type& value = prop.second.is_variant() ?
                    prop.second.cast<ubus::dbus_variant>().get() :
                    prop.second;
                if (opt.print_signature) {
                    cout << setw(max_width) << key.get() << ' '
                         << setw(max_sig_width) << value.signature() << ": "
                         << value.to_string() << endl;
                }else{
                    cout << setw(max_width) << key.get() << ": " << value.to_string() << endl;
                }
            }
        }
        return true;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool set_property (ubus::connection& conn, const appargs_t& opt)
    {
        ubus::org_freedesktop_DBus_Properties prop (conn);
        std::unique_ptr<ubus::dbus_type> property_value;

        if (opt.args.size() == 1) {
            property_value = get_single_message_argument (opt.args[0]);
        }else if (opt.args.size() == 2) {
            dbus_arg_parser p;
            property_value = p (opt.args[0], opt.args[1]);
        }
        if (!property_value) {
            std::cerr << "Error: Invalid argument format." << std::endl;
            return false;
        }

        auto result = prop.set (opt.service, opt.opath, opt.iface, opt.name, *property_value, opt.timeout);
        if (result.err()) {
            cerr << "Error: " << result.what() << endl;
            return false;
        }
        return true;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool objects (ubus::connection& conn, const appargs_t& opt)
    {
        ubus::org_freedesktop_DBus_ObjectManager om (conn);
        auto reply = om.get_managed_objects (opt.service, opt.opath, opt.timeout);
        if (reply.err()) {
            cerr << "Error: " << reply.what() << endl;
            return false;
        }
        for (const auto& entry : reply.get()) {
            cout << entry.first << endl;
            if (!opt.all)
                continue;
            for (const auto& if_entry : entry.second) {
                cout << "    " << if_entry.first << endl;
                for (const auto& prop : if_entry.second) {
                    cout << "        " << prop.first.cast<ubus::dbus_string>().get() << endl;
                }
            }
            cout << endl;
        }
        return true;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool listen_for_signals (ubus::connection& conn, const appargs_t& opt)
    {
        // Install signal handler to exit gracefully on Ctrl-C
        stop_sleep_loop = false;
        struct sigaction sa;
        memset (&sa, 0, sizeof(sa));
        sigemptyset (&sa.sa_mask);
        sa.sa_handler = stop_signal_handler;
        sigaction (SIGINT, &sa, nullptr);

        ubus::callback_message_filter cmf (conn);
        cmf.set_signal_cb ([&opt, &conn](ubus::message& signal)->bool{
            if (opt.json_output) {
                cout << signal.to_json() << endl;
            }else{
                cout << "Signal       " << signal.name() << endl;
                cout << "Sender:      " << signal.sender() << endl;
                cout << "Object path: " << signal.path() << endl;
                cout << "Interface:   " << signal.interface() << endl;
                auto args = signal.arguments ();
                if (!args.empty()) {
                    cout << "Arguments: " << endl;
                    for (auto& arg : args) {
                        if (opt.print_signature)
                            cout << "    " << arg->signature() << ' ' << arg->to_string() << endl;
                        else
                            cout << "    " << arg->to_string() << endl;
                    }
                }
                cout << endl;
            }
            return true;
        });
        std::string rule ("type='signal'");
        if ( ! opt.service.empty()) {
            rule.append (",sender='");
            rule.append (opt.service);
            rule.push_back ('\'');
        }
        if ( ! opt.opath.empty()) {
            if (opt.recursive)
                rule.append (",path_namespace='");
            else
                rule.append (",path='");
            rule.append (opt.opath);
            rule.push_back ('\'');
        }
        if ( ! opt.iface.empty()) {
            rule.append (",interface='");
            rule.append (opt.iface);
            rule.push_back ('\'');
        }
        if ( ! opt.name.empty()) {
            rule.append (",member='");
            rule.append (opt.name);
            rule.push_back ('\'');
        }
        auto res = cmf.add_match (rule, opt.timeout);
        if (res.err()) {
            cerr << "Error: " << res.what() << endl;
            return false;
        }

        // Sleep until Ctrl-C (SIGINT)
        std::unique_lock ul (m);
        cv.wait (ul, []{return stop_sleep_loop;});
        return true;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool start_service (ubus::connection& conn, appargs_t& opt)
    {
        auto result = conn.start_service_by_name (opt.service, opt.timeout);
        if (result.err()) {
            if (!opt.quiet)
                cerr << result.what() << endl;
            return false;
        }
        switch (result.get()) {
        case DBUS_START_REPLY_SUCCESS:
            if (!opt.quiet)
                cout << opt.service << " started" << endl;
            break;
        case DBUS_START_REPLY_ALREADY_RUNNING:
            if (!opt.quiet)
                cout << opt.service << " already running" << endl;
            break;
        default:
            if (!opt.quiet)
                cerr << "Error: Unknown return value: " << result.get() << endl;
            return false;
        }
        return true;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool print_owner (ubus::connection& conn, appargs_t& opt)
    {
        auto owner = conn.get_name_owner (opt.service, opt.timeout);
        if (owner.err()) {
            cerr << owner.what() << endl;
            return false;
        }
        cout << owner.get() << endl;
        return true;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool print_names (ubus::connection& conn, appargs_t& opt)
    {
        string bus_name = opt.service;
        if (!opt.service.empty() && opt.service[0]!=':') {
            auto owner = conn.get_name_owner (opt.service, opt.timeout);
            if (owner.err()) {
                cerr << owner.what() << endl;
                return false;
            }
            bus_name = owner;
        }
        cout << bus_name << endl;

        auto names = conn.list_names (opt.timeout);
        for (auto& name : names.get()) {
            if (name.empty() || name[0]==':')
                continue;
            auto owner = conn.get_name_owner (name, opt.timeout);
            if (bus_name == owner.get())
                cout << '\t' << name << endl;
        }
        return true;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool ping (ubus::connection& conn, appargs_t& opt)
    {
        ubus::org_freedesktop_DBus_Peer peer (conn);

        auto result = peer.ping (opt.service, opt.timeout);
        if (result.err()) {
            if (!opt.quiet)
                cerr << "Error: " << result.what() << endl;
            return false;
        }
        if (!opt.quiet)
            cout << ((float)result.get()/1000) << " ms" << endl;
        return true;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool monitor (ubus::connection& conn, appargs_t& opt)
    {
        // Install signal handler to exit gracefully on Ctrl-C
        stop_sleep_loop = false;
        struct sigaction sa;
        memset (&sa, 0, sizeof(sa));
        sigemptyset (&sa.sa_mask);
        sa.sa_handler = stop_signal_handler;
        sigaction (SIGINT, &sa, nullptr);

        ubus::callback_message_filter cmf (conn);
        cmf.set_signal_cb ([](ubus::message& msg)->bool{
            cout << msg.to_json() << endl;
            return true;
        });

        bool use_eavesdrop = opt.eavesdrop;
        if ( ! use_eavesdrop) {
            auto result = conn.become_monitor (std::list<std::string>(), opt.timeout);
            if (result.err()) {
                cerr << "Warning: " << result.what() << endl;
                cerr << "Install eavesdrop match rule to monitor messages instead." << endl;
                use_eavesdrop = true;
            }
        }
        if (use_eavesdrop)
            cmf.add_match ("eavesdrop='true'", opt.timeout);

        // Sleep until Ctrl-C (SIGINT)
        std::unique_lock ul (m);
        cv.wait (ul, []{return stop_sleep_loop;});
        return true;
    }


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    bool send_signal (ubus::connection& conn, appargs_t& opt)
    {
        // Request a service name
        //
        if ( ! opt.service.empty()) {
            auto result = conn.request_name (opt.service, opt.timeout);
            if (result != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
                if (!opt.quiet) {
                    if (result.err())
                        cerr << "Error: " << result.what() << endl;
                    else
                        cerr << "Error: Unable to request the service name." << endl;
                }
                return false;
            }
        }

        // Create the signal
        //
        ubus::message signal (opt.opath, opt.iface, opt.name);
        auto num_args = opt.args.size ();
        if (num_args == 0) {
            ; // No arguments to the signal
        }
        else if (num_args == 1) {
            signal.append_args (*get_single_message_argument(opt.args[0]));
        }
        else if (num_args & 0x01) {
            std::cerr << "Error: Invalid argument format, missing signature or value." << std::endl;
            return false;
        }
        else{
            dbus_arg_parser p;
            for (size_t i=0; i<num_args; i+=2) {
                auto value = p (opt.args[i], opt.args[i+1]);
                if (value) {
                    signal.append_args (*value);
                }else{
                    std::cerr << "Error: Invalid argument format." << std::endl;
                    return false;
                }
            }
        }

        // Send the signal
        //
        if ( ! conn.send(signal)) {
            cerr << "Error sending signal" << endl;
            return false;
        }
        return true;
    }


} // Anonymous namespace


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int main (int argc, char* argv[])
{
    appargs_t opt (argc, argv);

    auto cmd = commands.find (opt.cmd);
    if (cmd == commands.end()) {
        cerr << "Error: Unknown command (-h for help)." << endl;
        return 1;
    }

    int retval = 0;
    try {
        ubus::connection conn;

        // if (opt.bus_address.empty()) {
            conn.connect (opt.bus);
            if (!conn.is_connected()) {
                cerr << "Error: Failed to connect to " <<
                    (opt.bus==DBUS_BUS_SESSION?"session":"system") << " bus" << endl;
                return 1;
            }
        // }else{
        //     conn.connect (opt.bus_address, opt.timeout, true);
        //     if (!conn.is_connected()) {
        //         cerr << "Error: Failed connecting to bus " << opt.bus_address << endl;
        //         return 1;
        //     }
        // }

        retval = cmd->second(conn, opt) ? 0 : 1;
    }
    catch (std::exception& e) {
        if (!opt.quiet)
            cerr << "Error - exception caught: " << e.what() << endl;
        retval = 1;
    }

    return retval;
}
