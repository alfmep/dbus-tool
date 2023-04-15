/*
 * Copyright (C) 2021,2023 Dan Arrhenius <dan@ultramarin.se>
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
#include <iostream>
#include <utility>
#include <string>
#include <list>
#include <cstring>

using namespace std;


#ifdef NO_LIBXML2
//
// Don't use libxml to parse introspect data, print introspect data "as is"
//
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void print_introspect (const std::string& opath, const std::string& str)
{
    cout << str << endl;
}
#else


//
// Use libxml2 to parse and print introspect xml data.
//
#include <libxml/parser.h>
#include <libxml/tree.h>


struct arg_t {
    string name;
    string sig;
    arg_t () = default;
    arg_t (const string& name_arg, const string& sig_arg)
        : name(name_arg), sig(sig_arg)
        {}
};
struct method_t {
    string name;
    list<arg_t> in;
    arg_t out;
};
struct property_t {
    string name;
    string sig;
    string access;
};
struct iface_t {
    string name;
    list<method_t> methods;
    list<method_t> signals;
    list<property_t> props;
};
struct object_t {
    string path;
    list<iface_t> ifaces;
    list<string> nodes;
};


static void get_type_name (xmlNode* node, string& type, string& name);
static void parse_method (xmlNode* root, method_t& method);
static void parse_signal (xmlNode* root, method_t& signal);
static void parse_prop (xmlNode* node, property_t& prop);
static void parse_iface (xmlNode* root, iface_t& iface);
static void parse_node (xmlNode* root, object_t& obj);




//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void print_introspect (const std::string& opath, const std::string& str)
{
    object_t obj;
    obj.path = opath;

    xmlDoc* doc = nullptr;
    xmlNode* root = nullptr;
    doc = xmlReadMemory (str.c_str(), str.size(), nullptr, nullptr, 0);
    if (!doc) {
        cerr << "Error: Can't parse introspect result" << endl;
        exit (1);
    }
    root = xmlDocGetRootElement (doc);
    if (root->children)
        parse_node (root->children, obj);


    bool first_iface = true;
    cout << "Interfaces:" << endl;
    for (auto& iface : obj.ifaces) {
        if (first_iface)
            first_iface = false;
        else
            cout << endl;
        cout << "    " << iface.name << endl;

        auto& methods = iface.methods;
        if (!methods.empty()) {
            cout << "        Methods:" << endl;
            for (auto& m : methods) {

                cout << "            " << m.name << endl;

                if (!m.in.empty()) {
                    cout << "                ";
                    cout << "IN: ";
                    bool first_item = true;
                    for (auto& arg : m.in) {
                        if (!first_item)
                            cout << ", ";
                        else
                            first_item = false;
                        if (arg.name.empty())
                            cout << arg.sig;
                        else
                            cout << arg.name << "(" << arg.sig << ")";
                    }
                    cout << endl;
                }
                if (!m.out.sig.empty()) {
                    if (m.out.name.empty())
                        cout << "                " << "OUT: " << m.out.sig;
                    else
                        cout << "                " << "OUT: " << m.out.name << "(" << m.out.sig << ")";
                    cout << endl;
                }
            }
        }

        auto& signals = iface.signals;
        if (!signals.empty()) {
            cout << "        Signals:" << endl;
            for (auto& s : signals) {
                cout << "            " << s.name << endl;
                if (s.in.empty())
                    continue;
                cout << "                ";
                cout << " ARG: ";
                bool first_item = true;
                for (auto& arg : s.in) {
                    if (arg.name.empty() && arg.sig.empty())
                        continue;
                    if (!first_item)
                        cout << ", ";
                    else
                        first_item = false;
                    if (arg.name.empty())
                        cout << arg.sig;
                    else
                        cout << arg.name << "(" << arg.sig << ")";
                }
                cout << endl;
            }
        }

        auto& props = iface.props;
        if (!props.empty()) {
            cout << "        Properties:" << endl;
            for (auto& p : props) {
                cout << "            " << p.name << endl;
                cout << "                Signature: " << p.sig << endl;
                cout << "                Access: " << p.access << endl;
            }
        }
    }
    if (!obj.nodes.empty()) {
        cout << endl;
        cout << "Nodes:" << endl;
        for (auto& node : obj.nodes) {
            cout << "    " << node << endl;
        }
    }
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void get_type_name (xmlNode* node, string& type, string& name)
{
    type = (const char*)node->name;
    name = "";
    xmlAttr* attr = node->properties;
    while (attr) {
        if (attr->name && strcmp((const char*)attr->name, "name")==0)
            break;
        attr = attr->next;
    }
    if (attr && attr->children && attr->children->content)
        name = (const char*)attr->children->content;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void parse_method (xmlNode* root, method_t& method)
{
    for (xmlNode* node=root; node; node=node->next) {
        if (node->type != XML_ELEMENT_NODE)
            continue;
        if (strcmp((const char*)node->name, "arg") !=0 )
            continue;

        string name = "";
        string dir = "";
        string signature = "";

        for (xmlAttr* attr=node->properties; attr; attr=attr->next) {
            if (attr->name && strcmp((const char*)attr->name, "name")==0) {
                if (attr->children && attr->children->content)
                    name = (const char*)attr->children->content;
            }
            else if (attr->name && strcmp((const char*)attr->name, "direction")==0) {
                if (attr->children && attr->children->content)
                    dir = (const char*)attr->children->content;
            }
            else if (attr->name && strcmp((const char*)attr->name, "type")==0) {
                if (attr->children && attr->children->content)
                    signature = (const char*)attr->children->content;
            }
        }
        if (!dir.empty() && !signature.empty()) {
            if (dir == "out") {
                method.out.name = name;
                method.out.sig = signature;
            }else if (dir == "in") {
                method.in.emplace_back (name, signature);
            }
        }
    }
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void parse_signal (xmlNode* root, method_t& signal)
{
    for (xmlNode* node=root; node; node=node->next) {
        if (node->type != XML_ELEMENT_NODE)
            continue;

        string type = (const char*)node->name;
        string name = "";
        string signature = "";

        if (type == "arg") {
            xmlAttr* attr = node->properties;
            while (attr) {
                if (attr->name && strcmp((const char*)attr->name, "name")==0) {
                    if (attr->children && attr->children->content)
                        name = (const char*)attr->children->content;
                }
                if (attr->name && strcmp((const char*)attr->name, "type")==0) {
                    if (attr->children && attr->children->content)
                        signature = (const char*)attr->children->content;
                }
                attr = attr->next;
            }
            if (!signature.empty())
                signal.in.emplace_back (name, signature);
        }
    }
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void parse_prop (xmlNode* node, property_t& prop)
{
    string type = (const char*)node->name;

    xmlAttr* attr = node->properties;
    while (attr) {
        if (attr->name && strcmp((const char*)attr->name, "type")==0) {
            if (attr->children && attr->children->content)
                prop.sig = (const char*)attr->children->content;
        }
        if (attr->name && strcmp((const char*)attr->name, "access")==0) {
            if (attr->children && attr->children->content)
                prop.access = (const char*)attr->children->content;
        }
        attr = attr->next;
    }
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void parse_iface (xmlNode* root, iface_t& iface)
{
    for (xmlNode* node=root; node; node=node->next) {
        string type;
        string name;
        if (node->type != XML_ELEMENT_NODE)
            continue;
        get_type_name (node, type, name);

        if (type == "method") {
            method_t method;
            method.name = name;
            parse_method (node->children, method);
            iface.methods.emplace_back (method);
        }
        else if (type == "signal") {
            method_t signal;
            signal.name = name;
            parse_signal (node->children, signal);
            iface.signals.emplace_back (signal);
        }
        else if (type == "property") {
            property_t prop;
            prop.name = name;
            parse_prop (node, prop);
            iface.props.emplace_back (prop);
        }
    }
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void parse_node (xmlNode* root, object_t& obj)
{
    for (xmlNode* node=root; node; node=node->next) {
        string type;
        string name;
        if (node->type != XML_ELEMENT_NODE)
            continue;
        get_type_name (node, type, name);

        if (type == "interface") {
            iface_t iface;
            iface.name = name;
            parse_iface (node->children, iface);
            obj.ifaces.push_back (iface);
        }
        else if (type == "node") {
            obj.nodes.push_back (name);
        }
    }
}

#endif // NO_LIBXML2
