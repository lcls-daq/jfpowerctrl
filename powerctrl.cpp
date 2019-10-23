#include <iostream>

#include "Server.hh"
#include "Reader.hh"

using namespace Pds::Jungfrau;

int main(int argc, char *argv[])
{
  Server srv(8888, 3);
  srv.run();
  /*PowerControl ps("/power_control");
  std::cout << "temp: " << ps.get_temp() << " voltage: " << ps.get_voltage() << " current: " << ps.get_current() << std::endl;

  Block b1("/var/log/block");
  Block b2("/var/log/power_control.log");
  std::cout << "blocks: " << b1.is_active() << " " << b2.is_active() << std::endl;*/

  return 0;
}
