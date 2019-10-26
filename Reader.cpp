#include "Reader.hh"

#include <sys/stat.h>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <fstream>

using namespace Pds::Jungfrau;

Block::Block(std::string block) :
  _block(block)
{}

Block::~Block()
{}

bool Block::is_active() const
{
  struct stat buf;
  return (stat(_block.c_str(), &buf) == 0);
}

Control::Control(std::string path, std::string type, std::string dev) :
  _sep('/'),
  _path(path),
  _type(type),
  _dev(dev)
{}

Control::~Control()
{}

std::string Control::read_raw_value(std::string cmd, int id) const
{
  std::string result;

  // read the file
  std::ifstream file(filename(cmd, id).c_str());
  if (file.is_open()) {
    file >> result;
    file.close();
  }

  return result;
}

int Control::read_value(std::string cmd, int id) const
{
  int result = -1;
  
  // read the file
  std::ifstream file(filename(cmd, id).c_str());
  if (file.is_open()) {
    file >> result;
    file.close();
  }

  return result;
}

bool Control::write_value(unsigned value, std::string cmd, int id) const
{
  std::ofstream file(filename(cmd, id).c_str());
  if (file.is_open()) {
    file << value;
    file.close();
    return true;
  } else {
    return false;
  }
}

std::string Control::filename(std::string cmd, int id) const
{
  std::stringstream fname;
  // construct the file name
  fname << _path << _sep << _type << _sep << _dev;
  // if the id is negative don't use it
  if (id >= 0) fname << id;
  fname << _sep << cmd;

  return fname.str();
}

PowerControl::PowerControl(std::string path, const int id) :
  Control(path, "hwmon", "ps"),
  _id(id)
{}

PowerControl::~PowerControl()
{}

bool PowerControl::set_power(unsigned value) const
{
  return write_value(value, "set_power", _id);
}

int PowerControl::get_power() const
{
  return read_value("set_power", _id);
}

int PowerControl::get_temp() const
{
  return read_value("temp_input", _id);
}

int PowerControl::get_voltage() const
{
  return read_value("volt_input", _id);
}

int PowerControl::get_current() const
{
  return read_value("curr_input", _id);
}

std::string PowerControl::get_name() const
{
  return read_raw_value("name", _id);
}

LedControl::LedControl(std::string path) :
  Control(path, "gpios", "")
{}

LedControl::~LedControl()
{}

bool LedControl::set_led(unsigned mask) const
{
  return set_led_green(mask&1) && set_led_yellow((mask&2)>>1) && set_led_red((mask&4)>>2);
}

unsigned LedControl::get_led() const
{
  return (get_led_green()<<0) | (get_led_yellow()<<1) | (get_led_red()<<2);
}

int LedControl::get_led_green() const
{
  return read_value("set_led_green");
}

int LedControl::get_led_red() const
{
  return read_value("set_led_red");
}

int LedControl::get_led_yellow() const
{
  return read_value("set_led_yellow");
}

bool LedControl::set_led_green(unsigned value) const
{
  return write_value(value, "set_led_green");
}

bool LedControl::set_led_red(unsigned value) const
{
  return write_value(value, "set_led_red");
}

bool LedControl::LedControl::set_led_yellow(unsigned value) const
{
  return write_value(value, "set_led_yellow");
}

MiscControl::MiscControl(std::string path) :
  Control(path, "gpios", "")
{}

MiscControl::~MiscControl()
{}

int MiscControl::get_autostart_enable() const
{
  return read_value("get_autostart_enable");
}

int MiscControl::get_fanctrl_enable() const
{
  return read_value("get_fanctrl_enable");
}

int MiscControl::get_flowmeter_enable() const
{
  return read_value("get_flowmeter_enable");
}

int MiscControl::get_inhibit() const
{
  return read_value("get_inhibit");
}

int MiscControl::get_inhibit_enable() const
{
  return read_value("get_inhibit_enable");
}

int MiscControl::get_powerswitch() const
{
  return read_value("get_powerswitch");
}

const std::string CommandRunner::PSCMD = "PS";
const std::string CommandRunner::GPIOCMD = "GPIO";
const std::string CommandRunner::LEDCMD = "LED";

CommandRunner::CommandRunner(std::string path, std::string block,
                             const unsigned num_ps, const unsigned num_gpios) :
  _num_ps(num_ps),
  _num_gpios(num_gpios),
  _block(new Block(block)),
  _led(new LedControl(path)),
  _misc(new MiscControl(path)),
  _ps(new PowerControl*[num_ps])
{
  for (unsigned i=0; i<num_ps; i++) {
    _ps[i] = new PowerControl(path, i);
  }
}

CommandRunner::~CommandRunner()
{
  if (_block) {
    delete _block;
  }
  if (_led) {
    delete _led;
  }
  if (_misc) {
    delete _misc;
  }
  if (_ps) {
    for (unsigned i=0; i<_num_ps; i++) {
      if (_ps[i]) {
        delete _ps[i];
      }
    }
    delete[] _ps;
  }
}

std::string CommandRunner::run(const std::string& cmd) const
{
  size_t cpos = cmd.find(":");
  size_t vpos = cmd.find(" ", cpos+1);
  std::string prefix = cmd.substr(0, cpos);
  std::string suffix = cmd.substr(cpos == std::string::npos ? 0 : cpos+1, vpos - (cpos+1));
  std::string value = cmd.substr(vpos == std::string::npos ? 0 : vpos+1,
                                 vpos == std::string::npos ? 0 : std::string::npos);
  if (!cmd.compare("*IDN?")) {
    return std::string("JF4MD-CTRL\n");
  } else if (!cmd.compare("AUTOSTART?")) {
    return int_to_reply(_misc->get_autostart_enable());
  } else if (!cmd.compare("FANCTRL?")) {
    return int_to_reply(_misc->get_fanctrl_enable());
  } else if (!cmd.compare("FLOWMETER?")) {
    return int_to_reply(_misc->get_flowmeter_enable());
  } else if (!cmd.compare("INHIBIT?")) {
    return int_to_reply(_misc->get_inhibit_enable());
  } else if (!cmd.compare("INHIBITED?")) {
    return int_to_reply(_misc->get_inhibit());
  } else if (!cmd.compare("POWERSWITCH?")) {
    return int_to_reply(_misc->get_powerswitch());
  } else if (is_ps_cmd(cmd)) {
    return run_ps(prefix, suffix, value);
  } else if (is_gpio_cmd(cmd)) {
    return std::string("");
  } else if (is_led_cmd(cmd)) {
    return run_led(suffix, value);
  } else {
    std::cerr << "Error: invalid command received: " << cmd << std::endl;
    return std::string("");
  }
}

std::string CommandRunner::run_led(const std::string& cmd,
                                   const std::string& value) const
{
  if (value.empty()) {
    if (!cmd.compare("MASK?")) {
      return int_to_reply(_led->get_led());
    } else if (!cmd.compare("GREEN?")) {
      return int_to_reply(_led->get_led_green());
    } else if (!cmd.compare("YELLOW?")) {
      return int_to_reply(_led->get_led_green());
    } else if (!cmd.compare("RED?")) {
      return int_to_reply(_led->get_led_green());
    } else if (cmd.empty() || cmd[cmd.length() - 1] == '?') {
      std::cerr << "Error: invalid led get command received: "
                << cmd << std::endl;
    } else {
      std::cerr << "Error: received an led set command without a value" << std::endl;
    }
  } else {
    char* end = NULL;
    unsigned ivalue = std::strtoul(value.c_str(), &end, 0);
    if (*end != '\0') {
      std::cerr << "Error: invalid led set command value: " << value << std::endl;
    } else if (!cmd.compare("MASK")) {
      if (!_led->set_led(ivalue)) {
        std::cerr << "Error: set_led(" << value << ") failed" << std::endl;
      }
    } else if (!cmd.compare("GREEN")) {
      if (!_led->set_led_green(ivalue)) {
        std::cerr << "Error: set_green_led(" << value << ") failed" << std::endl;
      }
    } else if (!cmd.compare("YELLOW")) {
      if (!_led->set_led_yellow(ivalue)) {
        std::cerr << "Error: set_yellow_led(" << value << ") failed" << std::endl;
      }
    } else if (!cmd.compare("RED")) {
      if (!_led->set_led_red(ivalue)) {
        std::cerr << "Error: set_red_led(" << value << ") failed" << std::endl;
      }
    } else if (cmd.empty() || cmd[cmd.length() - 1] != '?'){
      std::cerr << "Error: invalid led set command received: "
                << cmd  << std::endl;
    } else {
      std::cerr << "Error: received an led get command with a value" << std::endl;
    }
  }

  return std::string("");
}

std::string CommandRunner::run_ps(const std::string& prefix,
                                  const std::string& cmd,
                                  const std::string& value) const
{
  char* end = NULL;
  unsigned index = std::strtoul(prefix.substr(PSCMD.length()).c_str(), &end, 0);
  if (*end != '\0') {
    std::cerr << "Error: invalid power supply prefix: " << prefix << std::endl;
  } else if (index < _num_ps) {
    if (value.empty()) {
      if (!cmd.compare("NAME?")) {
        return _ps[index]->get_name() + '\n';
      } else if (!cmd.compare("TEMP?")) {
        return int_to_reply(_ps[index]->get_temp());
      } else if (!cmd.compare("VOLT?")) {
        return int_to_reply(_ps[index]->get_voltage());
      } else if (!cmd.compare("CURR?")) {
        return int_to_reply(_ps[index]->get_current());
      } else if (!cmd.compare("POWER?")) {
        return int_to_reply(_ps[index]->get_power());
      } else {
          std::cerr << "Error: invalid power supply get command received: "
                    << cmd << std::endl;
      }
    } else {
      unsigned ivalue = std::strtoul(value.c_str(), &end, 0);
      if (*end != '\0') {
        std::cerr << "Error: invalid power supply set command value: " << value << std::endl;
      } else if (!cmd.compare("POWER")) {
        if (!_ps[index]->set_power(ivalue)) {
          std::cerr << "Error: set_power(" << value << ") failed for power supply "
                    << index << std::endl;
        }
      } else {
        std::cerr << "Error: invalid power supply set command received: "
                  << cmd  << std::endl;
      }
    }
  } else {
    std::cerr << "Power supply index out-of-range: " << index << std::endl;
  }

  return std::string("");
}

std::string CommandRunner::int_to_reply(int value) const
{
  std::stringstream ss;
  ss << value << std::endl;;
  return ss.str();
}

bool CommandRunner::is_ps_cmd(const std::string& cmd) const
{
  return check_cmd(PSCMD, cmd);
}

bool CommandRunner::is_gpio_cmd(const std::string& cmd) const
{
  return check_cmd(GPIOCMD, cmd);
}

bool CommandRunner::is_led_cmd(const std::string& cmd) const
{
  return check_cmd(LEDCMD, cmd);
}

bool CommandRunner::check_cmd(const std::string& type, const std::string& cmd) const
{
  return !cmd.compare(0, type.length(), type, 0, type.length());
}
