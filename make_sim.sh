#!/bin/bash
PWRDIR="$(mktemp -d)"
LOGDIR="$PWRDIR"
PSPATH="hwmon/ps0"
GFMPATH="hwmon/gfm0"
FANPATH="hwmon/fan0"
GPIOPATH="gpios"
GPIODIRS="0 1 2 3"

function make_file {
  echo -n "$3" > "$1/$2"
}

echo "Creating simulated Jungfrau power control in $PWRDIR"
cd "$PWRDIR"
# create power supply files
mkdir -p "$PSPATH"
make_file "$PSPATH" name cpfe1000fi
make_file "$PSPATH" set_power 0
make_file "$PSPATH" temp_input 22600
make_file "$PSPATH" volt_input 11980
make_file "$PSPATH" curr_input 1956
# create flow meter files
mkdir -p "$GFMPATH"
make_file "$GFMPATH" name gfm
make_file "$GFMPATH" flow_input 14000
make_file "$GFMPATH" temp_input 15200
# create fan files
mkdir -p "$FANPATH"
make_file "$FANPATH" name max6650
make_file "$FANPATH" fan1_input 30
make_file "$FANPATH" fan1_target 238125
make_file "$FANPATH" fan1_div 4
# create the gpio files
mkdir -p "$GPIOPATH"
make_file "$GPIOPATH" get_autostart_enable 1
make_file "$GPIOPATH" get_fanctrl_enable 1
make_file "$GPIOPATH" get_flowmeter_enable 1
make_file "$GPIOPATH" get_inhibit 0
make_file "$GPIOPATH" get_inhibit_enable 1
make_file "$GPIOPATH" get_powerswitch 1
make_file "$GPIOPATH" set_led_green 0
make_file "$GPIOPATH" set_led_red 0
make_file "$GPIOPATH" set_led_yellow 0
for g in $GPIODIRS; do
  mkdir "$GPIOPATH/$g"
  make_file "$GPIOPATH/$g" get_ac_warning 0
  make_file "$GPIOPATH/$g" get_dc_warning 1
  make_file "$GPIOPATH/$g" get_temp_warning 0
  make_file "$GPIOPATH/$g" set_power_supply_onoff 0
  for num in {1..12}; do
    make_file "$GPIOPATH/$g" "set_mcb${num}" 0
  done
done

# create the block file
make_file "$LOGDIR" block 0
