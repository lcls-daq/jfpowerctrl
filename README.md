# SLAC Jungfrau Power Control Server
Simple server application written in C++ that runs on the Jungfrau power supply
Blackfin to facilitate controlling the power via an EPICS streamdevice IOC.

## Build/Install

## 

## Running on the Blackfin
The `powerctrl` application is setup to start on boot via /etc/initab on the
Blackfin. To manually start if needed do the following:

```
telnet pwr-jungfrau4m
/power_control_slac/powerctrl -p /power_control -l /var/log
```
