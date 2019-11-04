# SLAC Jungfrau Power Control Server
Simple server application written in C++ that runs on the Jungfrau power supply
Blackfin to facilitate controlling the power via an EPICS streamdevice IOC.

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
