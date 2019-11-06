# SLAC Jungfrau Power Control Server
Simple server application written in C++ that runs on the Jungfrau power supply
Blackfin to facilitate controlling the power via an EPICS StreamDevice IOC.

## Build/Install

To cross-compile application for the Blackfin you need to download the
[ADI GUN toolchain](https://sourceforge.net/projects/adi-toolchain/).

If you install the full toolchain under __/opt/uClinux/bfin-uclinux__ then
you can do the following to build the `powerctrl` executable:
```
source setup_env.sh
make
```

Running `make install` will put the `powerctrl` executable in
__/var/lib/tftpboot__, so it can be downloaded to the Blackfin
via tftp:
```
tftp -g -r powerctrl <tftp_server_hostname>
```

## Testing 
A script, `make_sim.sh`, is provided to allow testing of the `powerctrl`
application without an actual Jungfrau power supply or Blackfin. It makes a
temporary directory which mimics the sysfs file/directory structure on the
Blackfin.

An example of how to run `powerctrl` using the test directory:
```
$ ./make_sim.sh 
Creating simulated Jungfrau power control in /tmp/tmp.0TRKtx1opt
$ ./powerctrl -p /tmp/tmp.0TRKtx1opt -l /tmp/tmp.0TRKtx1opt
```

## Running
The usage information for the `powerctrl` application:
```
./powerctrl -h
Usage: ./powerctrl [-v|--version] [-h|--help]
-p|--path <path> -l|--logdir <logdir> [-P|--port <port>]
[-c|--conn <connections>]
 Options:
    -p|--path     <path>                    the path to the power control scripts
    -l|--logdir   <logdir>                  the logdir of the power control scripts
    -P|--port     <port>                    port to use for the server (default: 32415)
    -c|--conn     <connections>             maximum number of connections (default: 3)
    -v|--version                            show file version
    -h|--help                               print this message and exit
```
The important parameters are __-p__ and __-l__, which tells the application
where the `power_control` scripts from PSI live, and where PSI scripts write
their logs and status files, respectively.

The `powerctrl` application is setup to start on boot via /etc/initab on the
Blackfin. To manually start if needed telnet to Blackfin and do the following:

```
/power_control_slac/powerctrl -p /power_control -l /var/log
```

## Protocol
The server expects commands as ASCII terminated with '\n'. The following is an
example EPICS StreamDevice protocol file for communicating with it:
```
#####
#
# Jungfrau powerctrl Protocol File
#
#####

InTerminator  = LF;
OutTerminator = LF;
LockTimeout   = 10000;
ReplyTimeout  = 3500;
ReadTimeout   = 100;
WriteTimeout  = 100;
ExtraInput    = Error;

GET_IDN       { out "*IDN?";      in "%39c"; }

# Power on/off state of the detector
GET_STATE     { out "STATE?";           in "%{OFF|ON|ERROR}"; }
# Set the power on/off state of the detector
SET_STATE     { out "STATE %{OFF|ON}";  in "%(\$1){OFF|ON}"; @init { GET_STATE; } }
# Get if the detector is blocked from powering on
GET_BLOCK     { out "BLOCK?"; in "%{NO|YES}"; }
# Set/Clear the detector blocked state
SET_BLOCK     { out "BLOCK %{CLEAR|SET}"; }
# The number of active modules in the detector
GET_MODULES   { out "MODULES?";         in "%d"; }
GET_CONDITION { out "INHIBITED?";       in "%{0|1}"; }
# Sets the sleep interval for enabling (in us)
# The interval to sleep (in us) between enabling each module
GET_INTERVAL  { out "INTERVAL?";  in "%d"; }
# If detector power is blocked
SET_INTERVAL  { out "INTERVAL %d"; }
# Position of the autostart dip switch
GET_AUTOSTART { out "AUTOSTART?"; in "%{1|0}"; }
# Position of the fan control dip switch
GET_FANCTRL   { out "FANCTRL?";   in "%{1|0}"; }
# Position of the flowmeter dip switch
GET_FLOWMETER { out "FLOWMETER?"; in "%{1|0}"; }
# Position of the inhibit dip switch
GET_INHIBIT   { out "INHIBIT?";   in "%{1|0}"; }
# Read the position of physical power switch
GET_SWITCH    { out "POWERSWITCH?"; in "%{0|1}"; }

###
# Macros for power on and off the detector:
#  these mimic the power_control_user scripts from PSI
###
MACRO_ON      { out "ON"; }
MACRO_OFF     { out "OFF"; }
MACRO_TOGGLE  { out "TOGGLE"; }

###
# Power Supply Commands
###
# The model name of the power supply
GET_PS_NAME   { out "PS\$1:NAME?";  in "%39c"; }
# The terminal voltage of the power supply
GET_PS_VOLT   { out "PS\$1:VOLT?";  in "%d"; }
# The current draw of the power supply
GET_PS_CURR   { out "PS\$1:CURR?";  in "%d"; }
# The temperature of the power supply
GET_PS_TEMP   { out "PS\$1:TEMP?";  in "%d"; }
# The on/off state of the 12Volt output of the power supply
GET_PS_POWER  { out "PS\$1:POWER?"; in "%{0|1}"; }
# Turn on/off the 12Volt output of the power supply
SET_PS_POWER  { out "PS\$1:POWER %{0|1}"; @init { GET_PS_POWER; } }

###
# GPIO Commands
###
# The enabled state of one of the GPIO outputs
GET_ENABLE { out "GPIO\$1:ENABLE\$2?"; in "%{0|1}"; }
# If the GPIO output will be enabled when powering on the system
GET_ACTIVE { out "GPIO\$1:ACTIVE\$2?"; in "%{0|1}"; }
# Enable/Disable the one of the GPIO outputs
SET_ENABLE { out "GPIO\$1:ENABLE\$2 %{0|1}"; @init { GET_ENABLE; } }
# Designate a GPIO output to be enabled when powering on the system
SET_ACTIVE { out "GPIO\$1:ACTIVE\$2 %{0|1}"; }
# Same behavior GET_ENABLE, but with a mask for all the outputs in one GPIO
GET_ENABLE_MASK { out "GPIO\$1:ENABLE?"; in "%d"; }
# Same behavior GET_ACTIVE, but with a mask for all the outputs in one GPIO
GET_ACTIVE_MASK { out "GPIO\$1:ACTIVE?"; in "%d"; }
# Same behavior SET_ENABLE, but with a mask for all the outputs in one GPIO
SET_ENABLE_MASK { out "GPIO\$1:ENABLE %d"; @init { GET_ENABLE_MASK; } }
# Same behavior SET_ACTIVE, but with a mask for all the outputs in one GPIO
SET_ACTIVE_MASK { out "GPIO\$1:ACTIVE %d"; }
# The power supply can be controlled via GPIO as well
GET_GPIO_POWER  { out "GPIO\$1:POWER?"; in "%{0|1}"; }
SET_GPIO_POWER  { out "GPIO\$1:POWER %{0|1}"; @init { GET_GPIO_POWER; } }

###
# Get the several warning states from the power supply
###
GET_WARN_TEMP   { out "GPIO\$1:WARN:TEMP?";  in "%{0|1}"; }
GET_WARN_AC     { out "GPIO\$1:WARN:AC?";    in "%{0|1}"; }
GET_WARN_DC     { out "GPIO\$1:WARN:DC?";    in "%{0|1}"; }

###
# Get the status of the LEDs on the chassis of the power supply
###
GET_LED_GREEN   { out "LED:GREEN?";   in "%{0|1}"; }
GET_LED_YELLOW  { out "LED:YELLOW?";  in "%{0|1}"; }
GET_LED_RED     { out "LED:RED?";     in "%{0|1}"; }
GET_LED_MASK    { out "LED:MASK?";    in "%d"; }
```
