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
#include "introspect_parser.hpp"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <cstring>
#include <iostream>

using ip = introspect_parser;
using std::endl;

static const std::set<std::string> standard_interfaces = {
    "org.freedesktop.DBus",
    "org.freedesktop.DBus.Introspectable",
    "org.freedesktop.DBus.ObjectManager",
    "org.freedesktop.DBus.Peer",
    "org.freedesktop.DBus.Properties",
};


static void parse_xml_node (xmlNode* xml, ip::node_t& node, bool skip_standard_interfaces);
static void get_type_name (xmlNode* node, std::string& type, std::string& name);
static void parse_xml_iface (xmlNode* xml_node, ip::iface_t& iface);
static void parse_xml_method (xmlNode* xml_node, ip::method_t& method);
static void parse_xml_signal (xmlNode* xml_node, ip::method_t& signal);
static void parse_xml_prop (xmlNode* xml_node, ip::property_t& prop);


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
introspect_parser::introspect_parser (bool skip_standard_interfaces_arg)
    : skip_standard_interfaces (skip_standard_interfaces_arg)
{
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
ip::node_t introspect_parser::parse_xml (const std::string& xml)
{
    ip::node_t node;
    xmlDoc* doc = nullptr;
    xmlNode* root = nullptr;

    doc = xmlReadMemory (xml.c_str(), xml.size(), nullptr, nullptr, 0);
    if (!doc)
        throw std::invalid_argument ("Invalid introspect data");

    root = xmlDocGetRootElement (doc);
    if (root->children)
        parse_xml_node (root->children, node, skip_standard_interfaces);

    xmlFreeDoc (doc);
    return node;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void introspect_parser::print (const node_t& node, std::ostream& out, bool json_output)
{
    print (node, "", "", out, json_output);
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void introspect_parser::print (const node_t& node,
                               const std::string& service,
                               const std::string& object_path,
                               std::ostream& out,
                               bool json_output)
{
    if (json_output) {
        print_json (node, service, object_path, out);
        return;
    }
    if ( ! service.empty())
        out << "Service: " << service << endl;
    if ( ! object_path.empty())
        out << "Object path: " << object_path << endl;

    bool first_iface = true;
    out << "Interfaces:" << endl;
    for (auto& iface : node.ifaces) {
        if (first_iface)
            first_iface = false;
        else
            out << endl;
        out << "    " << iface.name << endl;

        auto& methods = iface.methods;
        if (!methods.empty()) {
            out << "        Methods:" << endl;
            for (auto& m : methods) {

                out << "            " << m.name << endl;

                if (!m.in.empty()) {
                    out << "                ";
                    out << "IN: ";
                    bool first_item = true;
                    for (auto& arg : m.in) {
                        if (!first_item)
                            out << ", ";
                        else
                            first_item = false;
                        if (arg.name.empty())
                            out << arg.sig;
                        else
                            out << arg.name << "(" << arg.sig << ")";
                    }
                    out << endl;
                }
                if (!m.out.sig.empty()) {
                    if (m.out.name.empty())
                        out << "                " << "OUT: " << m.out.sig;
                    else
                        out << "                " << "OUT: " << m.out.name << "(" << m.out.sig << ")";
                    out << endl;
                }
            }
        }

        auto& signals = iface.signals;
        if (!signals.empty()) {
            out << "        Signals:" << endl;
            for (auto& s : signals) {
                out << "            " << s.name << endl;
                if (s.in.empty())
                    continue;
                out << "                ";
                out << " ARG: ";
                bool first_item = true;
                for (auto& arg : s.in) {
                    if (arg.name.empty() && arg.sig.empty())
                        continue;
                    if (!first_item)
                        out << ", ";
                    else
                        first_item = false;
                    if (arg.name.empty())
                        out << arg.sig;
                    else
                        out << arg.name << "(" << arg.sig << ")";
                }
                out << endl;
            }
        }

        auto& props = iface.props;
        if (!props.empty()) {
            out << "        Properties:" << endl;
            for (auto& p : props) {
                out << "            " << p.name << endl;
                out << "                Signature: " << p.sig << endl;
                out << "                Access: " << p.access << endl;
            }
        }
    }
    if (!node.sub_nodes.empty()) {
        out << endl;
        out << "Nodes:" << endl;
        for (auto& node : node.sub_nodes)
            out << "    " << node << endl;
    }
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void introspect_parser::print_json (const node_t& node,
                                    const std::string& service,
                                    const std::string& object_path,
                                    std::ostream& out)
{
    bool first_iface = true;
    bool got_attrib = false;

    out << "{";

    if ( ! service.empty()) {
        out << R"("service":")" << service << '"';
        got_attrib = true;
    }
    if ( ! object_path.empty()) {
        if (got_attrib)
            out << ',';
        out << R"("object_path":")" << object_path << '"';
        got_attrib = true;
    }

    if ( ! node.ifaces.empty()) {
        if (got_attrib)
            out << ',';
        out << R"("interfaces":[)";
    }
    for (auto& iface : node.ifaces) {
        if (first_iface)
            first_iface = false;
        else
            out << ',';

        out << R"({"name":")" << iface.name << '"';

        auto& methods = iface.methods;
        if ( ! methods.empty())
            out << R"(,"methods":[)";
        bool first_method = true;
        for (auto& m : methods) {
            if (first_method)
                first_method = false;
            else
                out << ',';
            out << '{';
            out << R"("name":")" << m.name << '"';
            if (!m.in.empty()) {
                out << R"(,"in":[)";
                bool first_item = true;
                for (auto& arg : m.in) {
                    if (!first_item)
                        out << ',';
                    else
                        first_item = false;
                    out << '{';
                    if ( ! arg.name.empty())
                        out << R"("name":")" << arg.name << R"(",)";
                    out << R"("signature":")" << arg.sig << R"("})";
                }
                out << ']';
            }
            if (!m.out.sig.empty()) {
                out << R"(,"out":{)";
                if ( ! m.out.name.empty())
                    out << R"("name":")" << m.out.name << R"(",)";
                out << R"("signature":")" << m.out.sig << R"("})";
            }
            out << '}';
        }
        if ( ! methods.empty())
            out << ']';


        auto& signals = iface.signals;
        if ( ! signals.empty()) {
            out << R"(,"signals":[)";
            bool first_signal = true;
            for (auto& s : signals) {
                if (first_signal)
                    first_signal = false;
                else
                    out << ',';

                out << R"({"name":")" << s.name << '"';

                if (s.in.empty()) {
                    out << '}';
                    continue;
                }
                out << R"(,"arguments":[)";
                bool first_item = true;
                for (auto& arg : s.in) {
                    if (arg.name.empty() && arg.sig.empty())
                        continue;
                    if (!first_item)
                        out << ',';
                    else
                        first_item = false;
                    out << '{';
                    if ( ! arg.name.empty())
                        out << R"("name":")" << arg.name << R"(",)";
                    out << R"("signature":")" << arg.sig << R"("})";
                }
                out << "]}";
            }
            out << ']';
        }

        auto& props = iface.props;
        if (!props.empty()) {
            out << R"(,"properties":[)";
            bool first_item = true;
            for (auto& p : props) {
                if (!first_item)
                    out << ',';
                else
                    first_item = false;
                out << '{';
                out << R"("name":")" << p.name << R"(",)";
                out << R"("signature":")" << p.sig << R"(",)";
                out << R"("access":")" << p.access << '"';
                out << '}';
            }
            out << ']';
        }
        out << '}';
    }
    if ( ! node.ifaces.empty()) {
        out << ']';
    }

    if (!node.sub_nodes.empty()) {
        out << R"(,"nodes":[)";
        bool first_item = true;
        for (auto& node : node.sub_nodes) {
            if (!first_item)
                out << ',';
            else
                first_item = false;
            out << '"' << node << '"';
        }
        out << ']';
    }
    out << '}';
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void parse_xml_node (xmlNode* xml_node, ip::node_t& node, bool skip_standard_interfaces)
{
    for (; xml_node!=nullptr; xml_node=xml_node->next) {
        std::string type;
        std::string name;

        if (xml_node->type != XML_ELEMENT_NODE)
            continue;
        get_type_name (xml_node, type, name);

        if (type == "interface") {
            if (skip_standard_interfaces &&
                standard_interfaces.find(name)!=standard_interfaces.end())
            {
                continue; // Skip standard interfaces
            }
            ip::iface_t iface;
            iface.name = name;
            parse_xml_iface (xml_node->children, iface);
            node.ifaces.push_back (iface);
        }
        else if (type == "node") {
            node.sub_nodes.insert (name);
        }
    }
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void get_type_name (xmlNode* node, std::string& type, std::string& name)
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
static void parse_xml_iface (xmlNode* xml_node, ip::iface_t& iface)
{
    for (; xml_node; xml_node=xml_node->next) {
        std::string type;
        std::string name;

        if (xml_node->type != XML_ELEMENT_NODE)
            continue;
        get_type_name (xml_node, type, name);

        if (type == "method") {
            ip::method_t method;
            method.name = name;
            parse_xml_method (xml_node->children, method);
            iface.methods.emplace_back (method);
        }
        else if (type == "signal") {
            ip::method_t signal;
            signal.name = name;
            parse_xml_signal (xml_node->children, signal);
            iface.signals.emplace_back (signal);
        }
        else if (type == "property") {
            ip::property_t prop;
            prop.name = name;
            parse_xml_prop (xml_node, prop);
            iface.props.emplace_back (prop);
        }
    }
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
static void parse_xml_method (xmlNode* xml_node, ip::method_t& method)
{
    for (; xml_node; xml_node=xml_node->next) {
        std::string name = "";
        std::string dir = "";
        std::string signature = "";

        if (xml_node->type != XML_ELEMENT_NODE)
            continue;
        if (strcmp((const char*)xml_node->name, "arg") !=0 )
            continue;

        for (xmlAttr* attr=xml_node->properties; attr; attr=attr->next) {
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
static void parse_xml_signal (xmlNode* xml_node, ip::method_t& signal)
{
    for (; xml_node; xml_node=xml_node->next) {
        if (xml_node->type != XML_ELEMENT_NODE)
            continue;

        std::string type = (const char*)xml_node->name;
        std::string name = "";
        std::string signature = "";

        if (type == "arg") {
            xmlAttr* attr = xml_node->properties;
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
static void parse_xml_prop (xmlNode* xml_node, ip::property_t& prop)
{
    std::string type = (const char*)xml_node->name;

    xmlAttr* attr = xml_node->properties;
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
