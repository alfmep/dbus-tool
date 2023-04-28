# dbus-tool
dbus-tool is a multipurpose command line utility for the DBus interprocess communication system.
Using dbus-tool we can inspect services, get/set property values, listen for signals, send signals, call methods, and more.

### Table of contents
- **[Using dbus-tool](#using-dbus-tool)**
  - **[Commands](#commands)**
    - **[list](#list)**
    - **[introspect](#introspect)**
    - **[get](#get)**
    - **[set](#set)**
    - **[call](#call)**
    - **[signal](#signal)**
    - **[listen](#listen)**
    - **[monitor](#monitor)**
    - **[ping](#ping)**
    - **[owner](#owner)**
    - **[names](#names)**
    - **[start](#start)**
- **[Passing DBus arguments](#passing-dbus-arguments)**
- **[Examples](#examples)**


# Using dbus-tool

dbus-tool by default connects to the session bus and performs the requested command.
Normal output is printed to standard output, errors are printed to standard error. On errors, dbus-tool exits with exit code 1, and on success the exit code is 0.

Synopsis: **`dbus-tool [COMMON-OPTIONS...] COMMAND [command arguments...]`**
### Common options

Option | Description
--|--
`-y`, `--system`      | Connect to the system bus instead of the session bus.
`-b`, `--bus=ADDRESS` | Connect to a specific bus address. Ignoring parameter `-y`.
`-t`, `--timeout=MILLISECONDS` | Set a specific timeout when waiting for message replies.
`-v`, `--version` | Print version information and exit.
`-h`, `--help` | Print help and exit.

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
`-s`, `--signature` | When printing the reply arguments, also print the DBus signature of the arguments.


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



## Passing DBus arguments
When using commands **set**, **call**, and **signal**, there is an option to send arguments to the commands. All DBus arguments must be of a specific type. There are 13 primitive types and 4 container types in the DBus protocol. All types has what is called a *signature* that tells what type it is.
So when sending DBus arguments, first type the signature of the argument, and then the argument itself. This sequence is repeated for all arguments sent with the command. An example:
```
$ dbus-tool call some.interface /an/object/path an.interface Method i 42 b true
```
In this example, `i 42 b true` are the two arguments sent to the method we call. The first argument has signature `i` (a 32-bit signed integer) and value `42`. The second argument has signature `b` (boolean) and value `true`.

### Signatures
Here is a list of the 13 primitive types and their signatures:
Signature | Type | Description
:--|:--|:--
`y` | byte | Unsigned 8-bit integer.
`b` | boolean | A boolean value. Can be written as `0`, `1`, `false`, or `true`.
`n` | int16 | Signed 16-bit integer.
`q` | uint16 | Unsigned 16-bit integer.
`i` | int32 | Signed 32-bit integer.
`u` | uint32 | Unsigned 32-bit integer.
`x` | int64 | Signed 64-bit integer.
`t` | uint64 | Signed 64-bit integer.
`d` | double | IEEE 754 double-precision floating point.
`h` | unix_fd | Unix file descriptor. A signed 32-bit integer.
`s` | string | A string. When passing string values, they must be enclosed by either double quotes `"` or single quotes `'`.
`o` | object_path | An object path (string).
`g` | signature | A DBus signature (string).

And here are the 4 container types and their signatures:
Signature | Type | Description
:--|:--|:--
`a...` | array | An array. The `a` is followed by the signature of the array element type.
`(...)` | struct | A struct. Within the parenthesis are the signatures of the struct attributes.
`{...}` | dict_entry | A dict entry is a key-value pair. Within the brackets are the signatures of the key and the value. The key must be a primitive type. A dict entry is a type that can *only* be found as an array element.
`v` | variant | A variant. A variant means that the value can be of any DBus type.


**Signature examples:**
- An unsigned 32-bit integer: `u`
- An array with signed 16-bit integers: `an`
- A struct containing a boolean and a string: `(bs)`
- An array of structs containing boolean and a string: `a(bs)`
- An array of key-value pairs where the key is a signed 32-bit integer and the value is a string: `a{is}`
- A struct with a boolean, an array of key-value pairs as above, and an object path: `(ba{is}o)`

### Values
After the signature of the DBus argument comes the actual value of the argument.

#### Numeric values
Numeric values are written as normal numerical values.

*Example:* `42`


#### Boolean values
Boolean values are written as `0`, `1`, `false`, or `true`.

*Example:* `false`

*Example:* `1`


#### String values
String values are written as UTF-8 strings enclosed by single or double quotes.
Some special characters must be escaped within the string:
Character | Escape sequence | Comment
:--|:--|:--
carriage return | `\r` |
line feed | `\n` | 
form feed | `\f` |
tab | `\t` |
backspace | `\b` |
reverse solidus | `\\` |
double quote | `\"` | Only required when the string is enclosed by double quotes.
single quote | `\'` | Only required when the string is enclosed by single quotes.

*Example:* `"Hello World!"`

*Example:* `'First line.\nSecond line.'`

*Example:* `"This is a \"quoted\" word in a string."`


***Important Note!***
When entering a string on the command line in a shell, the string command line argument needs to be enclosed by (double)quotes, or special characters escaped. So, on the command line in a bash shell, a DBus string value would be written as:
`'"Hello World!"'`
or:
`"'Hello World!'"`
or:
`\"Hello\ World!\"`


#### Array values
Array values are written as a sequence of item values separated by comma(`,`) and enclosed by `[` and `]`.

*Example:* `[0,1,2,3]`

*Example:* `[false,true,false,false,false,true]`

*Example:* `["Array","of","strings"]`


#### Struct values
Struct values are written as a sequence of attribute values separated by comma(`,`) and enclosed by `{` and `}`.

*Example (a struct with a string and an integer):*
`{"Answer",42}`

*Example (a struct with a boolean, an integer and a string):*
`{true,42,"Hello World!"}`


#### Dict entry values
A dict entry value written as a key and value separated by comma(`,`) enclosed by `(` and `)`.
Since a dice entry type is only permitted as an element type of an array, they can only written as array values.

*Example (the key is an integer and the value a string):* `[(1,"first"),(2,"second")]`

*Example (the key is a string and the value a boolean):* `[("array",true),("struct",false),("dict-entry",true)]`


#### Variant values
A variant is a special DBus type that can hold a value of any other DBus type. But since the dbus-tool must know what DBus type we are to send as the variant value, we must specify what value type we are sending. In order to do that we enter a prefix to the value. The prefix is the signature of the value followed by an underscore `_`.

So in order to send a signed 32-bit integer as a variant value we write the value like this: `i_42`.

*Example (we are sending a 32-bit signed integer as a variant value):* `i_42`

*Example (a string as variant value):* `s_"Hello World!"`

*Example (an array of 32-bit unsigned integers as variant value):* `au_[1,2,3,4]`

*Example (an array of variants as variant value):* `av_[i_42,s_"string",b_true]`


#### Examples of writing DBus arguments from a bash command line shell
An example of a dbus-tool command that sends DBus arguments can look like this:

`$ dbus-tool call my.example.service /object/path my.example.interface Method i 42`
In the examples bellow we will, for clarity, shorten the part `my.example.service /object/path my.example.interface Method` to `...`.

**Pass an integer from a bash shell:**

`$ dbus-tool call ... i 42`

**Pass a string from a bash shell:**

`$ dbus-tool call ... s '"Hello World"'`

**Pass an integer and a string from a bash shell:**

`$ dbus-tool call ... i 42  s '"Hello World"'`

**Pass an array of 32-bit signed integers from a bash shell:**

`$ dbus-tool cal ... ai [1,2,3,4]`

**Pass a struct with an unsigned integer and a string from a bash shell:**

`$ dbus-tool call ... '(us)' '{42,"Answer"}'`

**Pass a booelan, a signed integer, and a struct with an unsigned integer and a string as attributes from a bash shell:**

`$ dbus-tool call ... b false  i  32  '(us)' '{42,"Answer"}'`

**Pass an array of structs with a string and a boolean as attributes from a bash shell:**

`$ dbus-tool call ... 'a(sb)' '["First",true,"Second",true,"Third",false]'`

**Pass an array of arrays of integers from a bash shell:**

`$ dbus-tool call ... aai [[0,1,2,3],[42,32],[3489,2343,23543,52326]]`

**Pass an array of dict entries with strings as keys and booleans as values from a bash shell:**

`$ dbus-tool call ... a{sb} '[("On",true),("Off",false),("Other",false)]'`

**Pass an array of dict entries with integers as keys and variants as values from a bash shell:**

`$ dbus-tool call ... a{iv} '[(0,i_42),(1,s_"Hello\nLine 2"),(2,ai_[1,2,3])]'`




# Examples

The following examples were executed in a bash shell window running in an Ubuntu Linux system.

#### List all running DBus session services:
```
$ dbus-tool list
org.freedesktop.DBus
org.freedesktop.IBus
org.freedesktop.IBus.Panel.Extension.Gtk3
org.freedesktop.Notifications
org.freedesktop.PackageKit
org.freedesktop.ScreenSaver
org.freedesktop.systemd1
...
```

#### Ping DBus service "org.freedesktop.DBus":
```
$ dbus-tool ping org.freedesktop.DBus
0.493 ms
```

#### Get the unique bus name of the DBus service "org.freedesktop.Notifications", then list all service names registered by that DBus connection:
```
$ dbus-tool owner org.freedesktop.Notifications
:1.50

$ dbus-tool names :1.50
:1.50
	org.freedesktop.Notifications
	org.gnome.Shell.Notifications
```
When using command **names**, the argument doesn't have to be a unique name:
```
$ dbus-tool names org.freedesktop.Notifications
:1.50
	org.freedesktop.Notifications
	org.gnome.Shell.Notifications
```

#### Explore DBus service "org.freedesktop.Notifications" using command introspect:
```
$ dbus-tool introspect org.freedesktop.Notifications
Service: org.freedesktop.Notifications
Object path: /
Interfaces:

Nodes:
    org
```
```
$ dbus-tool introspect org.freedesktop.Notifications /org
Service: org.freedesktop.Notifications
Object path: /org
Interfaces:

Nodes:
    freedesktop
```
```
$ dbus-tool introspect org.freedesktop.Notifications /org/freedesktop
Service: org.freedesktop.Notifications
Object path: /org/freedesktop
Interfaces:

Nodes:
    Notifications
```
```
$ dbus-tool introspect org.freedesktop.Notifications /org/freedesktop/Notifications
Service: org.freedesktop.Notifications
Object path: /org/freedesktop/Notifications
Interfaces:
    org.freedesktop.DBus.Properties
        Methods:
            Get
                IN: interface_name(s), property_name(s)
                OUT: value(v)
            GetAll
                IN: interface_name(s)
                OUT: properties(a{sv})
            Set
                IN: interface_name(s), property_name(s), value(v)
        Signals:
            PropertiesChanged
                 ARG: interface_name(s), changed_properties(a{sv}), invalidated_properties(as)

    org.freedesktop.DBus.Introspectable
        Methods:
            Introspect
                OUT: xml_data(s)

    org.freedesktop.DBus.Peer
        Methods:
            Ping
            GetMachineId
                OUT: machine_uuid(s)

    org.freedesktop.Notifications
        Methods:
            Notify
                IN: arg_0(s), arg_1(u), arg_2(s), arg_3(s), arg_4(s), arg_5(as), arg_6(a{sv}), arg_7(i)
                OUT: arg_8(u)
            CloseNotification
                IN: arg_0(u)
            GetCapabilities
                OUT: arg_0(as)
            GetServerInformation
                OUT: arg_3(s)
        Signals:
            NotificationClosed
                 ARG: arg_0(u), arg_1(u)
            ActionInvoked
                 ARG: arg_0(u), arg_1(s)
```

#### Make the same introspection as above, but print the raw introspect data instead of parsing it:
```
$ dbus-tool introspect --raw org.freedesktop.Notifications /org/freedesktop/Notifications
<!DOCTYPE node PUBLIC "-//freedesktop//DTD D-BUS Object Introspection 1.0//EN"
                      "http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd">
<!-- GDBus 2.72.4 -->
<node>
  <interface name="org.freedesktop.DBus.Properties">
    <method name="Get">
      <arg type="s" name="interface_name" direction="in"/>
      <arg type="s" name="property_name" direction="in"/>
      <arg type="v" name="value" direction="out"/>
    </method>
    <method name="GetAll">
      <arg type="s" name="interface_name" direction="in"/>
      <arg type="a{sv}" name="properties" direction="out"/>
    </method>
    <method name="Set">
      <arg type="s" name="interface_name" direction="in"/>
      <arg type="s" name="property_name" direction="in"/>
      <arg type="v" name="value" direction="in"/>
    </method>
    <signal name="PropertiesChanged">
      <arg type="s" name="interface_name"/>
      <arg type="a{sv}" name="changed_properties"/>
      <arg type="as" name="invalidated_properties"/>
    </signal>
  </interface>
  <interface name="org.freedesktop.DBus.Introspectable">
    <method name="Introspect">
      <arg type="s" name="xml_data" direction="out"/>
    </method>
  </interface>
  <interface name="org.freedesktop.DBus.Peer">
    <method name="Ping"/>
    <method name="GetMachineId">
      <arg type="s" name="machine_uuid" direction="out"/>
    </method>
  </interface>
  <interface name="org.freedesktop.Notifications">
    <method name="Notify">
      <arg type="s" name="arg_0" direction="in">
      </arg>
      <arg type="u" name="arg_1" direction="in">
      </arg>
      <arg type="s" name="arg_2" direction="in">
      </arg>
      <arg type="s" name="arg_3" direction="in">
      </arg>
      <arg type="s" name="arg_4" direction="in">
      </arg>
      <arg type="as" name="arg_5" direction="in">
      </arg>
      <arg type="a{sv}" name="arg_6" direction="in">
      </arg>
      <arg type="i" name="arg_7" direction="in">
      </arg>
      <arg type="u" name="arg_8" direction="out">
      </arg>
    </method>
    <method name="CloseNotification">
      <arg type="u" name="arg_0" direction="in">
      </arg>
    </method>
    <method name="GetCapabilities">
      <arg type="as" name="arg_0" direction="out">
      </arg>
    </method>
    <method name="GetServerInformation">
      <arg type="s" name="arg_0" direction="out">
      </arg>
      <arg type="s" name="arg_1" direction="out">
      </arg>
      <arg type="s" name="arg_2" direction="out">
      </arg>
      <arg type="s" name="arg_3" direction="out">
      </arg>
    </method>
    <signal name="NotificationClosed">
      <arg type="u" name="arg_0">
      </arg>
      <arg type="u" name="arg_1">
      </arg>
    </signal>
    <signal name="ActionInvoked">
      <arg type="u" name="arg_0">
      </arg>
      <arg type="s" name="arg_1">
      </arg>
    </signal>
  </interface>
</node>
```

#### Call a method in service org.freedesktop.Notifications to display a desktop notification:
```bash
$ dbus-tool call org.freedesktop.Notifications /org/freedesktop/Notifications org.freedesktop.Notifications Notify  s '"dbus-tool"'  u 0  s '"applications-utilities"'  s '"Example Notification"'  s '"This is an example of a notification sent from dbus-tool."'  as []  a{sv} []  i 3000
```

#### Get the brightness of the screen by reading property "Brightness" of interface "org.gnome.SettingsDaemon.Power.Screen" at object path "/org/gnome/SettingsDaemon/Power" in service "org.gnome.SettingsDaemon.Power":
```
$ dbus-tool get org.gnome.SettingsDaemon.Power /org/gnome/SettingsDaemon/Power org.gnome.SettingsDaemon.Power.Screen Brightness
41
```

#### Set the brightness of the screen by setting property "Brightness" of interface "org.gnome.SettingsDaemon.Power.Screen" at object path "/org/gnome/SettingsDaemon/Power" in service "org.gnome.SettingsDaemon.Power":
_(Since the property value is a single signed 32-bit integer, we don't have to specify the DBus signature of the argument)_
```
$ dbus-tool set org.gnome.SettingsDaemon.Power /org/gnome/SettingsDaemon/Power org.gnome.SettingsDaemon.Power.Screen Brightness 60
```

#### Listen to DBus signals from service "org.gnome.SettingsDaemon.Power":
_(option -s is used to see the DBus signature of the arguments)_
```
$ dbus-tool listen -s org.gnome.SettingsDaemon.Power /org/gnome/SettingsDaemon/Power org.freedesktop.DBus.Properties PropertiesChanged
Got signal: PropertiesChanged
Interface:  org.freedesktop.DBus.Properties
Arguments: 
    s org.gnome.SettingsDaemon.Power.Keyboard
    a{sv} [(Brightness,50)]
    as []

Got signal: PropertiesChanged
Interface:  org.freedesktop.DBus.Properties
Arguments: 
    s org.gnome.SettingsDaemon.Power.Screen
    a{sv} [(Brightness,55)]
    as []

Got signal: PropertiesChanged
Interface:  org.freedesktop.DBus.Properties
Arguments: 
    s org.gnome.SettingsDaemon.Power.Screen
    a{sv} [(Brightness,60)]
    as []

^C$
```

#### Sending a signal with dbus-tool
This example makes a connections to the session bus, registers service name "se.ultramarin.ultrabus" and then sends a signal named "TestSignal" using interface "se.ultramarin.ultrabus.example" and object path "/se/ultramarin/ultrabus/example".
The signal is sent with three arguments. The first argument is a boolean. The second argument is a string. And the third argument is an array of variants. The array of variants has three elements, one signed 32-bit integer, one string, and finally one array of unsigned 32-bit integers.
```bash
$ dbus-tool signal se.ultramarin.ultrabus /se/ultramarin/ultrabus/example se.ultramarin.ultrabus.example TestSignal  b true  s '"Hello World!"'  av [i_42,s_\"Hello\",au_[1,2,3]]
```
To se the signal, first open another window with a shell and use dbus-tool to listen for signals from object path "/se/ultramarin/ultrabus/example" in service "se.ultramarin.ultrabus" _(option -s is used to see the DBus signature of the arguments)_:
```
$ dbus-tool listen -s se.ultramarin.ultrabus /se/ultramarin/ultrabus/example "" ""
Got signal: TestSignal
Interface:  se.ultramarin.ultrabus.example
Arguments: 
    b true
    s Hello World!
    av [42,Hello,[1,2,3]]

^C$
```
