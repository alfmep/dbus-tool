# dbus-tool
dbus-tool is a multipurpose command line utility for the DBus interprocess communication system.
Using dbus-tool we can inspect services, get/set property values, listen for signals, send signals, call methods, and more.


# Using dbus-tool

dbus-tool by default connects to the session bus and performs the requested command.
Normal output is printed to standard output, errors are printed to standard error. On errors, dbus-tool exits with exit code 1.

Synopsis: **`dbus-tool [COMMON-OPTIONS...] COMMAND [command arguments...]`**
### Common options

Option | Description
--|--
`-y`, `--system`      | Connect to the system bus instead of the session bus.
`-b`, `--bus=ADDRESS` | Connect to a specific bus address. Ignoring parameter `-y`.
`-t`, `--timeout=MILLISECONDS` | Set a specific timeout when waiting for message replies.
`-h`, `--help` | Print help and exit.
`-v`, `--version` | Print version information and exit.

## Commands

### list
**`dbus-tool [COMMON_OPTIONS] list [OPTIONS]`**

List the services (bus names) on this connection.
Options | Description
--|--
`-a`, `--all` | Also include unique bus names.
`-x`, `--activatable` | Instead of already connected names, list all names that can be activated on the bus.


### introspect
**`dbus-tool [COMMON_OPTIONS] introspect [OPTIONS] <service> [object_path]`**

Print introspect data for a specific object path in a DBus service. 
If the `object_path` arguments is omitted, the root object `/` is used.
Options | Description
--|--
`-r`, `--raw` | Don't parse the introspect data, print it "as is".


### objects
**`dbus-tool [COMMON_OPTIONS] objects <service> [object_path]`**

List all objects belonging to a specific service and object path.
If the `object_path` arguments is omitted, the root object `/` is used.


### get
**`dbus-tool [COMMON_OPTIONS] get [OPTIONS] <service> <object_path> <interface> [property]`**

Get the property of an object in a DBus service.
If argument `property` is omitted, the names and values of all properties are printed to standard output.
Options | Description
--|--
`-s`, `--signature` | Also print the DBus signature of the properties.


### set
**`dbus-tool [COMMON_OPTIONS] set <service> <object_path> <interface> <property> [signature] <value>`**

Set the property of an object in a DBus service.
The signature of the value can omitted if the value is a boolean(true|false), string, or a signed integer.


### call
**`dbus-tool [COMMON_OPTIONS] call [OPTIONS] <service> <object_path> <interface> <method> [signature argument...]`**

Call a specific method in an object/interface in a DBus service.
Any returned argument from the method is printed to standard output.
Arguments to the method begins with a DBus signature, followed by the argument value. If there is only a single argument, the signature can be omitted if the argument is a boolean(true|false), string, or a signed integer.
Options | Description
--|--
`-s`, `--signature` | When printing the signal arguments, also print the DBus signature of the arguments.


### signal
**`dbus-tool [COMMON_OPTIONS] signal <service> <object_path> <interface> <signal-name> [signature argument...]`**

Acquire a service name and send a DBus signal from that service.
This command will connect to the DBus and acquire the specified service name. Then it will send a signal with the specified object path and interface. Arguments to the signal begins with a DBus signature, then the argument value. If there is only a single argument, the signature can be omitted if the argument is a boolean(true|false), string, or a signed integer.


### listen
**`dbus-tool [COMMON_OPTIONS] listen [OPTIONS] <service> <object_path> <interface> [signal]`**

Listen for DBus signals from the specified object path using the specified interface.
If a signal name is specified, only that signal will be caught. If no signal name is specified, all signals from that object path and interface will be caught.
The signals will be printed to standard output. Stop listening and exit the program by pressing Ctrl-C.
Options | Description
--|--
`-s`, `--signature` | When printing the signal arguments, also print the DBus signature of the arguments.


### monitor
**`dbus-tool [COMMON_OPTIONS] monitor`**

Monitor messages on the message bus and display them on standard output.
Stop and exit by pressing Ctrl-C.


### ping
**`dbus-tool [COMMON_OPTIONS] ping [OPTIONS] <service>`**

Ping a service on the bus and print the response time in milliseconds.
Options | Description
--|--
`-q`, `--quiet` | Suppress normal output, exit with 0 on success and 1 on failure.


### owner
**`dbus-tool [COMMON_OPTIONS] owner <service>`**

Print the unique bus name of the primary owner of the service name.


### names
**`dbus-tool [COMMON_OPTIONS] names <bus-name>`**

Print the names acquired by the specified bus connection.


### start
**`dbus-tool [COMMON_OPTIONS] start [OPTIONS] <service>`**

Try to launch the executable associated with a service name.
Options | Description
--|--
`-q`, `--quiet` | Suppress normal output, exit with 0 on success and 1 on failure.
