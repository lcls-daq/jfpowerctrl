#include "Reader.hh"

#include <sys/stat.h>
#include <sys/time.h>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <cerrno>
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

Logger::Logger(std::string path, std::string name, Level level) :
  File(path, name),
  _level(level)
{}

Logger::~Logger()
{}

void Logger::debug(const std::string& message) const
{
  std::cout << message << std::endl;
  if (_level >= DEBUG) log(message);
}

void Logger::info(const std::string& message) const
{
  std::cout << message << std::endl;
  if (_level >= INFO) log(message);
}

void Logger::error(const std::string& message) const
{
  std::cerr << message << std::endl;
  if (_level >= ERROR) log(message);
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

Lock::Lock(std::string path, std::string name) :
  File(path, name)
{}

Lock::~Lock()
{}

bool Lock::is_set() const
{
  struct stat buf;
  return (stat(_filename.c_str(), &buf) == 0);
}

bool Lock::set() const
{
  std::ofstream file(_filename.c_str());
  if (file.is_open()) {
    std::cerr << "Problem creating block file " << _filename << std::endl;
    file.close();
    return true;
  } else {
    return false;
  }
}

bool Lock::clear() const
{
  if (std::remove(_filename.c_str()) < 0) {
    if (errno != ENOENT) {
      std::cerr << "Problem removing block file " << _filename << ": "
                << std::strerror(errno) << std::endl;
      return false;
    }
  }

  return true;
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

bool Control::wait_value(int value, std::string cmd, unsigned long timeout, int id) const
{
  unsigned long long delta = 0;
  struct timeval start, end;
  gettimeofday(&start, NULL);
  do {
    if (read_value(cmd, id) == value) return true;
    gettimeofday(&end, NULL);
    delta = (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec;
  } while(delta < timeout);

  return false;
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

FlowMeterControl::FlowMeterControl(std::string path, const int id) :
  Control(path, "hwmon", "gfm"),
  _id(id)
{}

FlowMeterControl::~FlowMeterControl()
{}

int FlowMeterControl::get_temp() const
{
  return read_value("temp_input", _id);
}

int FlowMeterControl::get_flow() const
{
  return read_value("flow_input", _id);
}

std::string FlowMeterControl::get_name() const
{
  return read_raw_value("name", _id);
}

FanControl::FanControl(std::string path, const int id) :
  Control(path, "hwmon", "fan"),
  _id(id)
{}

FanControl::~FanControl()
{}

int FanControl::get_input() const
{
  return read_value("fan1_input", _id);
}

int FanControl::get_target() const
{
  return read_value("fan1_target", _id);
}

int FanControl::get_div() const
{
  return read_value("fan1_div", _id);
}

std::string FanControl::get_name() const
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

bool GpioControl::wait_ac_warning(int value, unsigned long timeout) const
{
  return wait_value(value, "get_ac_warning", timeout, _id);
}

bool GpioControl::wait_dc_warning(int value, unsigned long timeout) const
{
  return wait_value(value, "get_dc_warning", timeout, _id);
}

bool GpioControl::wait_temp_warning(int value, unsigned long timeout) const
{
  return wait_value(value, "get_temp_warning", timeout, _id);
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

int GpioControl::get_mcb_active(const int id) const
{
  return (_active>>(id - 1)) & 1;
}

int GpioControl::get_mcb_active_mask() const
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

void GpioControl::set_mcb_active(const int id, unsigned value)
{
  _active = (_active & ~(1U<<(id - 1))) | (value<<(id-1));
}

void GpioControl::set_mcb_active_mask(unsigned mask)
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
const std::string CommandRunner::GFMCMD = "GFM";
const std::string CommandRunner::FANCMD = "FMON";
const std::string CommandRunner::GPIOCMD = "GPIO";
const std::string CommandRunner::LEDCMD = "LED";
const std::string CommandRunner::WARNCMD = "WARN:";
const std::string CommandRunner::MCBCMDS[] = {"ENABLE", "ACTIVE", ""};

CommandRunner::CommandRunner(std::string name,
                             std::string path,
                             std::string logpath,
                             const unsigned num_ps,
                             const unsigned num_gpios,
                             const unsigned num_gfm,
                             const unsigned num_fan) :
  _num_ps(num_ps),
  _num_gpios(num_gpios),
  _num_gfm(num_gfm),
  _num_fan(num_fan),
  _name(name),
  _pause(0),
  _timeout(0),
  _state(new Flag(logpath, "state")),
  _block(new Lock(logpath, "block")),
  _logger(new Logger(logpath, "power_control.log")),
  _led(new LedControl(path)),
  _misc(new MiscControl(path)),
  _ps(num_ps > 0 ? new PowerControl*[num_ps] : NULL),
  _ps_temp(num_ps > 0 ? new Lock*[num_ps] : NULL),
  _gpio(num_gpios > 0 ? new GpioControl*[num_gpios] : NULL),
  _gfm(num_gfm > 0 ? new FlowMeterControl*[num_gfm] : NULL),
  _gfm_flow(num_gfm > 0 ? new Lock*[num_gfm] : NULL),
  _gfm_temp(num_gfm > 0 ? new Lock*[num_gfm] : NULL),
  _fan(num_fan > 0 ? new FanControl*[num_fan] : NULL),
  _fan_input(num_fan > 0 ? new Lock*[num_fan] : NULL)
{
  for (unsigned i=0; i<num_ps; i++) {
    std::string idx = int_to_str(i);
    _ps[i] = new PowerControl(path, i);
    _ps_temp[i] = new Lock(logpath, "lock_temp_ps" + idx);
  }
  for (unsigned j=0; j<num_gpios; j++) {
    _gpio[j] = new GpioControl(path, j);
  }
  for (unsigned k=0; k<num_gfm; k++) {
    std::string idx = int_to_str(k);
    _gfm[k] = new FlowMeterControl(path, k);
    _gfm_flow[k] = new Lock(logpath, "lock_wflow_gfm" + idx);
    _gfm_temp[k] = new Lock(logpath, "lock_temp_gfm" + idx);
  }
  for (unsigned l=0; l<num_fan; l++) {
    std::string idx = int_to_str(l);
    _fan[l] = new FanControl(path, l);
    _fan_input[l] = new Lock(logpath, "lock_fan" + idx);
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
  /* cleanup ps arrays */
  for (unsigned i=0; i<_num_ps; i++) {
    if (_ps && _ps[i]) delete _ps[i];
    if (_ps_temp && _ps_temp[i]) delete _ps_temp[i];
  }
  if (_ps) {
    delete[] _ps;
  }
  if (_ps_temp) {
    delete[] _ps_temp;
  }
  /* cleanup gpio arrays */
  if (_gpio) {
    for (unsigned j=0; j<_num_gpios; j++) {
      if (_gpio[j]) {
        delete _gpio[j];
      }
    }
    delete[] _gpio;
  }
  /* cleanup the gfm arrays */
  for (unsigned k=0; k<_num_gfm; k++) {
    if (_gfm && _gfm[k]) delete _gfm[k];
    if (_gfm_flow && _gfm_flow[k]) delete _gfm_flow[k];
    if (_gfm_temp && _gfm_temp[k]) delete _gfm_temp[k];
  }
  if (_gfm) {
    delete[] _gfm;
  }
  if (_gfm_flow) {
    delete[] _gfm_flow;
  }
  if (_gfm_temp) {
    delete[] _gfm_temp;
  }
  /* cleanup the fan arrays */
  for (unsigned l=0; l<_num_fan; l++) {
    if (_fan && _fan[l]) delete _fan[l];
    if (_fan_input && _fan_input[l]) delete _fan_input[l];
  }
  if (_fan) {
    delete[] _fan;
  }
  if (_fan_input) {
    delete[] _fan_input;
  }
}

std::string CommandRunner::on(bool verbose) const
{
  if (_block->is_set()) {
    _logger->error("Detector in an unsafe condition, don't start");
  } else {
    if (_state->is_set() && check_ps()) {
      _logger->error("Detector in inconsistent on state!");
      off();
    }

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
        if (!_gpio[j]->wait_dc_warning(0, _timeout)) {
          std::cerr << "Error: wait_dc_warning(0, " << _timeout << ") failed for GPIO " << j << std::endl;
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
  if (is_off()) {
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
        std::cerr << "Error: set_power(0) failed for power supply " << i << std::endl;
      }
    }

    // wait for the power supply to ramp down
    for (unsigned j=0; j<_num_gpios; j++) {
      if (!_gpio[j]->wait_dc_warning(1, _timeout)) {
        std::cerr << "Error: wait_dc_warning(1, " << _timeout << ") failed for GPIO " << j << std::endl;
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
  } else if (is_gfm_cmd(cmd)) {
    return run_gfm(prefix, suffix, value);
  } else if (is_fan_cmd(cmd)) {
    return run_fan(prefix, suffix, value);
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
        if(_ps[index]->get_power())
            return int_to_reply(_ps[index]->get_voltage());
        else
            return int_to_reply(0);
      } else if (!cmd.compare("CURR?")) {
        return int_to_reply(_ps[index]->get_current());
      } else if (!cmd.compare("POWER?")) {
        return int_to_reply(_ps[index]->get_power());
      } else if (!cmd.compare("LOCKTEMP?")) {
        return lock_to_reply(_ps_temp[index]);
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

std::string CommandRunner::run_gfm(const std::string& prefix,
                                   const std::string& cmd,
                                   const std::string& value) const
{
  char* end = NULL;
  unsigned index = std::strtoul(prefix.substr(GFMCMD.length()).c_str(), &end, 0);
  if (*end != '\0') {
    std::cerr << "Error: invalid flow meter prefix: " << prefix << std::endl;
  } else if (index < _num_gfm) {
    if (value.empty()) {
      if (!cmd.compare("NAME?")) {
        return _gfm[index]->get_name() + '\n';
      } else if (!cmd.compare("TEMP?")) {
        return int_to_reply(_gfm[index]->get_temp());
      } else if (!cmd.compare("FLOW?")) {
        return int_to_reply(_gfm[index]->get_flow());
      } else if (!cmd.compare("LOCKTEMP?")) {
        return lock_to_reply(_gfm_temp[index]);
      } else if (!cmd.compare("LOCKFLOW?")) {
        return lock_to_reply(_gfm_flow[index]);
      } else if (cmd.empty() || cmd[cmd.length() - 1] == '?') {
        std::cerr << "Error: invalid flow meter get command received: "
                  << cmd << std::endl;
      } else {
        std::cerr << "Error: received a flow meter set command without a value" << std::endl;
      }
    } else {
      //unsigned ivalue = std::strtoul(value.c_str(), &end, 0);
      if (*end != '\0') {
        std::cerr << "Error: invalid flow meter set command value: " << value << std::endl;
      } else if (cmd.empty() || cmd[cmd.length() - 1] != '?') {
        std::cerr << "Error: invalid flow meter set command received: "
                  << cmd  << std::endl;
      } else {
        std::cerr << "Error: received a flow meter get command with a value" << std::endl;
      }
    }
  } else {
    std::cerr << "Flow meter index out-of-range: " << index << std::endl;
  }
  return std::string("");
}

std::string CommandRunner::run_fan(const std::string& prefix,
                                   const std::string& cmd,
                                   const std::string& value) const
{
  char* end = NULL;
  unsigned index = std::strtoul(prefix.substr(FANCMD.length()).c_str(), &end, 0);
  if (*end != '\0') {
    std::cerr << "Error: invalid fan prefix: " << prefix << std::endl;
  } else if (index < _num_fan) {
    if (value.empty()) {
      if (!cmd.compare("NAME?")) {
        return _fan[index]->get_name() + '\n';
      } else if (!cmd.compare("INPUT?")) {
        return int_to_reply(_fan[index]->get_input());
      } else if (!cmd.compare("TARGET?")) {
        return int_to_reply(_fan[index]->get_target());
      } else if (!cmd.compare("DIV?")) {
        return int_to_reply(_fan[index]->get_div());
      } else if (!cmd.compare("LOCKINPUT?")) {
        return lock_to_reply(_fan_input[index]);
      } else if (cmd.empty() || cmd[cmd.length() - 1] == '?') {
        std::cerr << "Error: invalid fan get command received: "
                  << cmd << std::endl;
      } else {
        std::cerr << "Error: received a fan set command without a value" << std::endl;
      }
    } else {
      //unsigned ivalue = std::strtoul(value.c_str(), &end, 0);
      if (*end != '\0') {
        std::cerr << "Error: invalid fan set command value: " << value << std::endl;
      } else if (cmd.empty() || cmd[cmd.length() - 1] != '?') {
        std::cerr << "Error: invalid fan set command received: "
                  << cmd  << std::endl;
      } else {
        std::cerr << "Error: received a fan get command with a value" << std::endl;
      }
    }
  } else {
    std::cerr << "Fan index out-of-range: " << index << std::endl;
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
        return int_to_reply(_gpio[index]->get_mcb_active_mask());
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
        std::string prefix = get_mcb_prefix(cmd);
        int mcbidx = get_mcb_index(cmd, prefix, '?');
        if (mcbidx < 0) {
          if (cmd.empty() || cmd[cmd.length() - 1] == '?'){
            std::cerr << "Error: invalid mcb get prefix: " << cmd << std::endl;
          } else {
            std::cerr << "Error: received an GPIO set command without a value" << std::endl;
          }
        } else if (!prefix.compare("ENABLE")) {
          return int_to_reply(_gpio[index]->get_mcb(mcbidx));
        } else if (!prefix.compare("ACTIVE")) {
          return int_to_reply(_gpio[index]->get_mcb_active(mcbidx));
        } else {
          std::cerr << "Error: get command is not implement for mcb prefix: " << prefix << std::endl;
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
        _gpio[index]->set_mcb_active_mask(ivalue);
      } else if (is_mcb_cmd(cmd)) {
        std::string prefix = get_mcb_prefix(cmd);
        int mcbidx = get_mcb_index(cmd, prefix, '\0');
        if (mcbidx < 0) {
          if (cmd.empty() || cmd[cmd.length() - 1] != '?') {
            std::cerr << "Error: invalid mcb set prefix: " << cmd << std::endl;
          } else {
            std::cerr << "Error: received an GPIO get command with a value" << std::endl;
          }
        } else if (!prefix.compare("ENABLE")) {
          if (!_gpio[index]->set_mcb(mcbidx, ivalue)) {
            std::cerr << "Error: set_mcb(" << mcbidx << ", " << value << ") failed for GPIO "
                      << index << std::endl;
          }
        } else if (!prefix.compare("ACTIVE")) {
          _gpio[index]->set_mcb_active(mcbidx, ivalue);
        } else {
          std::cerr << "Error: set command is not implement for mcb prefix: " << prefix << std::endl;
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
      return std::string(_name + "\n");
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
    } else if (!cmd.compare("TIMEOUT?")) {
      return int_to_reply(_timeout);
    } else if (!cmd.compare("MODULES?")) {
      return int_to_reply(num_active_modules());
    } else if (!cmd.compare("STATE?")) {
      return state();
    } else if (!cmd.compare("BLOCK?")) {
      return lock_to_reply(_block);
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
    } else if (!cmd.compare("BLOCK")) {
      if (!set_lock(_block, value)) {
        std::cerr << "Error: invalid value for BLOCK command: " << value << std::endl;
      }
    } else if (*end != '\0') {
      std::cerr << "Error: invalid led set command value: " << value << std::endl;
    } else if (!cmd.compare("INTERVAL")) {
      _pause = ivalue;
    } else if (!cmd.compare("TIMEOUT")) {
      _timeout = ivalue;
    } else if (cmd.empty() || cmd[cmd.length() - 1] != '?') {
      std::cerr << "Error: invalid set command received: "
                << cmd  << std::endl;
    } else {
      std::cerr << "Error: received a get command with a value" << std::endl;
    }
  }

  return std::string("");
}

std::string CommandRunner::int_to_str(long value) const
{
  std::stringstream ss;
  ss << value;
  return ss.str();
}

std::string CommandRunner::int_to_reply(long value) const
{
  std::stringstream ss;
  ss << value << std::endl;;
  return ss.str();
}

std::string CommandRunner::lock_to_reply(const Lock* lock) const
{
  if (lock->is_set()) {
    return std::string("YES\n");
  } else {
    return std::string("NO\n");
  }
}

std::string CommandRunner::state() const
{
  if (check_ps() || check_enables()) {
    return std::string("ERROR\n");
  } else if (_state->is_set()) {
    return std::string("ON\n");
  } else {
    return std::string("OFF\n");
  }
}

bool CommandRunner::is_off() const
{
  return !(_state->is_set() || check_ps() || check_enables());
}

bool CommandRunner::is_on() const
{
  return !(!_state->is_set() || check_ps() || check_enables());
}

bool CommandRunner::check_enables() const
{
  for (unsigned i=0; i<_num_gpios; i++) {
    if (_state->is_set()) {
      if (_gpio[i]->get_mcb_active_mask() != _gpio[i]->get_mcb_mask())
        return true;
    } else {
      if(_gpio[i]->get_mcb_mask())
        return true;
    }
  }

  return false;
}

bool CommandRunner::check_ps() const
{
  int expected = _state->is_set() ? 1 : 0;
  for (unsigned i=0; i<_num_ps; i++) {
    if (_ps[i]->get_power() != expected)
      return true;
  }

  return false;
}

bool CommandRunner::set_lock(const Lock* lock, const std::string& value) const
{
  if (!value.compare("SET")) { 
    if (!lock->set()) {
      std::stringstream msg;
      msg << "Failed to create " << lock->filename() << " file!";
      _logger->error(msg.str());
    }
    return true;
  } else if (!value.compare("CLEAR")) {
     if (!_block->clear()) {
       std::stringstream msg;
       msg << "Failed to remove " << lock->filename() << " file!";
      _logger->error(msg.str());
     }
    return true;
  } else {
    return false;
  }
}

bool CommandRunner::is_ps_cmd(const std::string& cmd) const
{
  return check_cmd(PSCMD, cmd);
}

bool CommandRunner::is_gfm_cmd(const std::string& cmd) const
{
  return check_cmd(GFMCMD, cmd);
}

bool CommandRunner::is_fan_cmd(const std::string& cmd) const
{
  return check_cmd(FANCMD, cmd);
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
  unsigned idx = 0;
  while (!MCBCMDS[idx].empty()) {
    if (check_cmd(MCBCMDS[idx], cmd)) {
      return true;
    }
    idx++;
  }

  return false;
}

bool CommandRunner::is_warn_cmd(const std::string& cmd) const
{
  return check_cmd(WARNCMD, cmd);
}

bool CommandRunner::check_cmd(const std::string& type, const std::string& cmd) const
{
  return !cmd.compare(0, type.length(), type, 0, type.length());
}

std::string CommandRunner::get_mcb_prefix(const std::string& cmd) const
{
  unsigned idx = 0;
  while (!MCBCMDS[idx].empty()) {
    if (check_cmd(MCBCMDS[idx], cmd)) {
      return MCBCMDS[idx];
    }
    idx++;
  }

  return std::string("");
}

int CommandRunner::get_mcb_index(const std::string& cmd,
                                 const std::string& prefix,
                                 const char match) const
{
  char* end = NULL;
  int index = -1;

  if (cmd.length() > (prefix.length() + (match == '\0' ? 0 : 1))) {
    index = std::strtol(cmd.substr(prefix.length()).c_str(), &end, 0);
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
