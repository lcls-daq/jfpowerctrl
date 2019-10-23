#include "Reader.hh"

#include <sys/stat.h>
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

bool PowerControl::set_power(unsigned value)
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
