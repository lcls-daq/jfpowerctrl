#include "Reader.hh"

#include <sys/stat.h>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <fstream>

using namespace Pds::Jungfrau;

File::File(std::string path, std::string name, const char sep)
{
  std::stringstream fname;
  fname << path << sep << name;
  _filename = fname.str();
}

File::~File()
{}

std::string File::filename() const
{
  return _filename;
}

Logger::Logger(std::string path, std::string name) :
  File(path, name)
{}

Logger::~Logger()
{}

void Logger::info(const std::string& message) const
{
  std::cout << message << std::endl;
  log(message);
}

void Logger::error(const std::string& message) const
{
  std::cerr << message << std::endl;
  log(message);
}

void Logger::log(const std::string& message) const
{
  std::ofstream file(_filename.c_str(), std::ofstream::out | std::ofstream::app);
  if (file.is_open()) {
    file << datetime() << " " << message << std::endl;
    file.close();
  }
}

std::string Logger::datetime() const
{
  time_t now = time(0);
  struct tm* dt = localtime(&now);
  char buffer[64];

  strftime(buffer, sizeof(buffer), "%a %b %d %T %Z %Y", dt);

  return std::string(buffer);
}

Block::Block(std::string path) :
  File(path, "block")
{}

Block::~Block()
{}

bool Block::is_active() const
{
  struct stat buf;
  return (stat(_filename.c_str(), &buf) == 0);
}

Flag::Flag(std::string path, std::string name) :
  File(path, name)
{}

Flag::~Flag()
{}

bool Flag::is_set() const
{
  return read_flag();
}

bool Flag::set() const
{
  return write_flag(true);
}

bool Flag::clear() const
{
  return write_flag(false);
}

bool Flag::read_flag() const
{
  bool result = false;

  // read the file
  std::ifstream file(_filename.c_str());
  if (file.is_open()) {
    file >> result;
    file.close();
  }

  return result;
}

bool Flag::write_flag(bool flag) const
{
  std::ofstream file(_filename.c_str());
  if (file.is_open()) {
    file << flag;
    file.close();
    return true;
  } else {
    return false;
  }
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

GpioControl::GpioControl(std::string path, const int id) :
  Control(path, "gpios", ""),
  _id(id),
  _active(ALL_ON)
{}

GpioControl::~GpioControl()
{}

int GpioControl::get_ac_warning() const
{
  return read_value("get_ac_warning", _id);
}

int GpioControl::get_dc_warning() const
{
  return read_value("get_dc_warning", _id);
}

int GpioControl::get_temp_warning() const
{
  return read_value("get_temp_warning", _id);
}

int GpioControl::get_power_supply_onoff() const
{
  return read_value("set_power_supply_onoff", _id);
}

bool GpioControl::set_power_supply_onoff(unsigned value) const
{
  return write_value(value, "set_power_supply_onoff", _id);
}

unsigned GpioControl::num_mcb_active() const
{
  unsigned nactive = 0;
  for (int i=0; i<NUM_MCB; i++) {
    nactive += (_active>>i)&1;
  }
  return nactive;
}

int GpioControl::get_mcb(const int id) const
{
  return read_value(mcbcmd(id), _id);
}

bool GpioControl::set_mcb(const int id, unsigned value) const
{
  return write_value(value, mcbcmd(id), _id);
}

int GpioControl::get_mcb_mask() const
{
  int mask = 0;
  for (int i=0; i<NUM_MCB; i++) {
    mask |= (get_mcb(i+1)<<i);
  }
  return mask;
}

int GpioControl::get_mcb_active() const
{
  return _active;
}

bool GpioControl::set_mcb_mask(unsigned mask, unsigned long pause) const
{
  time_t sec = pause / 1000000;
  long nsec = (pause % 1000000) * 1000;
  struct timespec pt = {sec, nsec};
  for (int i=0; i<NUM_MCB; i++) {
    if(!set_mcb(i+1, (mask>>i)&1))
      return false;
    else {
      nanosleep(&pt, NULL);
    }
  }
  return true;
}

void GpioControl::set_mcb_active(unsigned mask)
{
  _active = mask & ALL_ON;
}

bool GpioControl::set_mcb_on(unsigned long pause) const
{
  return set_mcb_mask(_active, pause);
}

bool GpioControl::set_mcb_off(unsigned long pause) const
{
  return set_mcb_mask(0, pause);
}

bool GpioControl::valid_mcb(const int id)
{
  return id > 0 && id <= NUM_MCB;
}

std::string GpioControl::mcbcmd(int id) const
{
  std::stringstream fname;
  fname << "set_mcb" << id;

  return fname.str();
}

const std::string CommandRunner::PSCMD = "PS";
const std::string CommandRunner::GPIOCMD = "GPIO";
const std::string CommandRunner::LEDCMD = "LED";
const std::string CommandRunner::MCBCMD = "ENABLE";
const std::string CommandRunner::WARNCMD = "WARN:";

CommandRunner::CommandRunner(std::string path, std::string logpath,
                             const unsigned num_ps, const unsigned num_gpios) :
  _num_ps(num_ps),
  _num_gpios(num_gpios),
  _pause(0),
  _state(new Flag(logpath, "state")),
  _block(new Block(logpath)),
  _logger(new Logger(logpath, "power_control.log")),
  _led(new LedControl(path)),
  _misc(new MiscControl(path)),
  _ps(new PowerControl*[num_ps]),
  _gpio(new GpioControl*[num_gpios])
{
  for (unsigned i=0; i<num_ps; i++) {
    _ps[i] = new PowerControl(path, i);
  }
  for (unsigned j=0; j<num_gpios; j++) {
    _gpio[j] = new GpioControl(path, j);
  }
}

CommandRunner::~CommandRunner()
{
  if (_state) {
    delete _state;
  }
  if (_block) {
    delete _block;
  }
  if (_logger) {
    delete _logger;
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
  if (_gpio) {
    for (unsigned j=0; j<_num_gpios; j++) {
      if (_gpio[j]) {
        delete _gpio[j];
      }
    }
    delete[] _gpio;
  }
}

std::string CommandRunner::on(bool verbose) const
{
  if (_block->is_active()) {
    _logger->error("Detector in an unsafe condition, don't start");
  } else {
    if (_state->is_set()) {
      if (check_enables()) {
        // update the state of the enables
        for (unsigned j=0; j<_num_gpios; j++) {
          if (!_gpio[j]->set_mcb_on(_pause)) {
            std::cerr << "Error: set_mcb_on(" << _pause << ") failed for GPIO " << j << std::endl;
          }
        }
        _logger->info("Detector enables updated");
      } else {
        _logger->error("Detector already on!");
      }
    } else {
      _led->set_led(3);

      // power on the supply
      for (unsigned i=0; i<_num_ps; i++) {
        if (!_ps[i]->set_power(1)) {
          std::cerr << "Error: set_power(1) failed for power supply " << i << std::endl;
        }
      }

      // turn on the enables
      for (unsigned j=0; j<_num_gpios; j++) {
        if (!_gpio[j]->set_mcb_on(_pause)) {
          std::cerr << "Error: set_mcb_on(" << _pause << ") failed for GPIO " << j << std::endl;
        }
      }

      _led->set_led_yellow(0);

      // set the state flag
      _state->set();
    }
  }

  if (verbose) {
    return state();
  } else {
    return std::string("");
  }
}

std::string CommandRunner::off(bool verbose) const
{
  if (!_state->is_set()) {
    _logger->error("Detector already off!");
  } else {
    _led->set_led(3);

    // turn off the enables
    for (unsigned j=0; j<_num_gpios; j++) {
      if (!_gpio[j]->set_mcb_off(_pause)) {
        std::cerr << "Error: set_mcb_off(" << _pause << ") failed for GPIO " << j << std::endl;
      }
    }

    // power off the supply
    for (unsigned i=0; i<_num_ps; i++) {
      if (!_ps[i]->set_power(0)) {
        std::cerr << "Error: set_power(1) failed for power supply " << i << std::endl;
      }
    }

    _led->set_led_green(0);

    // clear the state flag
    _state->clear();
  }

  if (verbose) {
    return state();
  } else {
    return std::string("");
  }
}

std::string CommandRunner::toggle() const
{
  if (_state->is_set()) {
    return off();
  } else {
    return on();
  }
}

std::string CommandRunner::run(const std::string& cmd)
{
  size_t cpos = cmd.find(":");
  size_t vpos = cmd.find(" ", cpos+1);
  std::string prefix = cmd.substr(0, cpos);
  std::string suffix = cmd.substr(cpos == std::string::npos ? 0 : cpos+1, vpos - (cpos+1));
  std::string value = cmd.substr(vpos == std::string::npos ? 0 : vpos+1,
                                 vpos == std::string::npos ? 0 : std::string::npos);

  if (is_ps_cmd(cmd)) {
    return run_ps(prefix, suffix, value);
  } else if (is_gpio_cmd(cmd)) {
    return run_gpios(prefix, suffix, value);
  } else if (is_led_cmd(cmd)) {
    return run_led(suffix, value);
  } else {
    return run_base(suffix, value);
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
    } else if (cmd.empty() || cmd[cmd.length() - 1] != '?') {
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
      } else if (cmd.empty() || cmd[cmd.length() - 1] == '?') {
          std::cerr << "Error: invalid power supply get command received: "
                    << cmd << std::endl;
      } else {
        std::cerr << "Error: received a power supply set command without a value" << std::endl;
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
      } else if (cmd.empty() || cmd[cmd.length() - 1] != '?') {
        std::cerr << "Error: invalid power supply set command received: "
                  << cmd  << std::endl;
      } else {
        std::cerr << "Error: received a power supply get command with a value" << std::endl;
      }
    }
  } else {
    std::cerr << "Power supply index out-of-range: " << index << std::endl;
  }

  return std::string("");
}

std::string CommandRunner::run_gpios(const std::string& prefix,
                                     const std::string& cmd,
                                     const std::string& value) const
{
  char* end = NULL;
  unsigned index = std::strtoul(prefix.substr(GPIOCMD.length()).c_str(), &end, 0);
  if (*end != '\0') {
    std::cerr << "Error: invalid GPIO prefix: " << prefix << std::endl;
  } else if (index < _num_gpios) {
    if (value.empty()) {
      if (!cmd.compare("POWER?")) {
        return int_to_reply(_gpio[index]->get_power_supply_onoff());
      } else if (!cmd.compare("ENABLE?")) {
        return int_to_reply(_gpio[index]->get_mcb_mask());
      } else if (!cmd.compare("ACTIVE?")) {
        return int_to_reply(_gpio[index]->get_mcb_active());
      } else if (is_warn_cmd(cmd)) {
        std::string warncmd = cmd.substr(WARNCMD.length());
        if (!warncmd.compare("AC?")) {
          return int_to_reply(_gpio[index]->get_ac_warning());
        } else if (!warncmd.compare("DC?")) {
          return int_to_reply(_gpio[index]->get_dc_warning());
        } else if (!warncmd.compare("TEMP?")) {
          return int_to_reply(_gpio[index]->get_temp_warning());
        } else if (warncmd.empty() || warncmd[warncmd.length() - 1] == '?') {
          std::cerr << "Error: invalid gpio get command received: "
                    << warncmd  << std::endl;
        } else {
          std::cerr << "Error: received an GPIO set command without a value" << std::endl;
        }
      } else if (is_mcb_cmd(cmd)) {
        int mcbidx = get_mcb_index(cmd, '?');
        if (mcbidx < 0) {
          if (cmd.empty() || cmd[cmd.length() - 1] == '?'){
            std::cerr << "Error: invalid mcb get prefix: " << cmd << std::endl;
          } else {
            std::cerr << "Error: received an GPIO set command without a value" << std::endl;
          }
        } else {
          return int_to_reply(_gpio[index]->get_mcb(mcbidx));
        }
      } else if (cmd.empty() || cmd[cmd.length() - 1] == '?'){
        std::cerr << "Error: invalid gpio get command received: "
                  << cmd  << std::endl;
      } else {
        std::cerr << "Error: received an GPIO set command without a value" << std::endl;
      }
    } else {
      unsigned ivalue = std::strtoul(value.c_str(), &end, 0);
      if (*end != '\0') {
        std::cerr << "Error: invalid GPIO set command value: " << value << std::endl;
      } else if (!cmd.compare("POWER")) {
        if (!_gpio[index]->set_power_supply_onoff(ivalue)) {
          std::cerr << "Error: set_power_supply_onoff(" << value << ") failed for GPIO "
                    << index << std::endl;
        }
      } else if (!cmd.compare("ENABLE")) {
        if (!_gpio[index]->set_mcb_mask(ivalue)) {
          std::cerr << "Error: set_mcb_mask(" << value << ") failed for GPIO "
                    << index << std::endl;
        }
      } else if (!cmd.compare("ACTIVE")) {
        _gpio[index]->set_mcb_active(ivalue);
      } else if (is_mcb_cmd(cmd)) {
        int mcbidx = get_mcb_index(cmd, '\0');
        if (mcbidx < 0) {
          if (cmd.empty() || cmd[cmd.length() - 1] != '?') {
            std::cerr << "Error: invalid mcb set prefix: " << cmd << std::endl;
          } else {
            std::cerr << "Error: received an GPIO get command with a value" << std::endl;
          }
        } else {
          if (!_gpio[index]->set_mcb(mcbidx, ivalue)) {
            std::cerr << "Error: set_mcb(" << mcbidx << ", " << value << ") failed for GPIO "
                      << index << std::endl;
          }
        }
      } else {
        std::cerr << "Error: invalid GPIO set command received: "
                  << cmd  << std::endl;
      }
    }
  } else {
    std::cerr << "GPIO index out-of-range: " << index << std::endl;
  }

  return std::string("");
}

std::string CommandRunner::run_base(const std::string& cmd,
                                    const std::string& value)
{
  if (value.empty()) {
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
    } else if (!cmd.compare("INTERVAL?")) {
      return int_to_reply(_pause);
    } else if (!cmd.compare("MODULES?")) {
      return int_to_reply(num_active_modules());
    } else if (!cmd.compare("STATE?")) {
      return state();
    } else if (!cmd.compare("ON")) {
      return on();
    } else if (!cmd.compare("OFF")) {
      return off();
    } else if (!cmd.compare("TOGGLE")) {
      return toggle();
    } else if (cmd.empty() || cmd[cmd.length() - 1] == '?') {
      std::cerr << "Error: invalid get command received: "
                << cmd << std::endl;
    } else {
      std::cerr << "Error: received a set command without a value" << std::endl;
    }
  } else {
    char* end = NULL;
    unsigned long ivalue = std::strtoul(value.c_str(), &end, 0);
    if (!cmd.compare("STATE")) {
      if (!value.compare("ON")) {
        return on(true);
      } else if (!value.compare("OFF")) {
        return off(true);
      } else {
        std::cerr << "Error: invalid value for STATE command: " << value << std::endl;
      }
    } else if (*end != '\0') {
      std::cerr << "Error: invalid led set command value: " << value << std::endl;
    } else if (!cmd.compare("INTERVAL")) {
      _pause = ivalue;
    } else if (cmd.empty() || cmd[cmd.length() - 1] != '?') {
      std::cerr << "Error: invalid set command received: "
                << cmd  << std::endl;
    } else {
      std::cerr << "Error: received a get command with a value" << std::endl;
    }
  }

  return std::string("");
}

std::string CommandRunner::int_to_reply(int value) const
{
  std::stringstream ss;
  ss << value << std::endl;;
  return ss.str();
}

std::string CommandRunner::state() const
{
  if (_state->is_set()) {
    return std::string("ON\n");
  } else {
    return std::string("OFF\n");
  }
}

bool CommandRunner::check_enables() const
{
  for (unsigned i=0; i<_num_gpios; i++) {
    if (_gpio[i]->get_mcb_active() != _gpio[i]->get_mcb_mask())
      return true;
  }

  return false;
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

bool CommandRunner::is_mcb_cmd(const std::string& cmd) const
{
  return check_cmd(MCBCMD, cmd);
}

bool CommandRunner::is_warn_cmd(const std::string& cmd) const
{
  return check_cmd(WARNCMD, cmd);
}

bool CommandRunner::check_cmd(const std::string& type, const std::string& cmd) const
{
  return !cmd.compare(0, type.length(), type, 0, type.length());
}

int CommandRunner::get_mcb_index(const std::string& cmd, const char match) const
{
  char* end = NULL;
  int index = -1;

  if (cmd.length() > (MCBCMD.length() + (match == '\0' ? 0 : 1))) {
    index = std::strtol(cmd.substr(MCBCMD.length()).c_str(), &end, 0);
    if (*end != match || !GpioControl::valid_mcb(index)) {
      index = -1;
    }
  }

  return index;
}

unsigned CommandRunner::num_active_modules() const
{
  unsigned num_active = 0;
  for (unsigned i=0; i<_num_gpios; i++) {
    num_active += _gpio[i]->num_mcb_active();
  }

  return num_active;
}
