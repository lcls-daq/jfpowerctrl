#include "Simulator.hh"
#include "Reader.hh"

#include <cstring>
#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <termios.h>
#include <stdlib.h>

using namespace Pds::Jungfrau;


Simulator::Simulator(std::string path) :
  _bmefd(-1)
{
  // Setup the BME.
#ifdef _GNU_SOURCE
  //_bmefd = ::getpt();
  _bmefd = posix_openpt(O_RDWR | O_NOCTTY);
  grantpt(_bmefd);
  unlockpt(_bmefd);
  {
      struct termios t;
      char buf[1024];
      tcgetattr(_bmefd, &t);
      t.c_lflag |= ~ECHO;
      tcsetattr(_bmefd, TCSANOW, &t);
      if (!ptsname_r(_bmefd, buf, sizeof(buf))) {
          ::unlink(File(path, "BME").filename().c_str());
          ::symlink(buf, File(path, "BME").filename().c_str());
      }
  }
#endif
  _bme_temp = File(path, "BME_temperature").filename();
  _bme_humid = File(path, "BME_humidity").filename();
  _bme_press = File(path, "BME_pressure").filename();
  _bme_alt = File(path, "BME_altitude").filename();
}

Simulator::~Simulator()
{
  if (_bmefd >= 0) {
    ::close(_bmefd);
    _bmefd = -1;
  }
}

double Simulator::readFloat(std::string filename) const
{
  double result = 0.0;
  std::ifstream file(filename.c_str());
  if (file.is_open()) {
    file >> result;
    file.close();
  }
  ::unlink(filename.c_str());
  return result;
}

void Simulator::writeBME(std::string format, double value) const
{
    char buf[1024];
    ::sprintf(buf, format.c_str(), value);
    write(_bmefd, buf, strlen(buf));
}

void Simulator::checkBME() const
{
    struct stat buf;
    double v;
    if (stat(_bme_temp.c_str(), &buf) == 0) {
        v = readFloat(_bme_temp);
        writeBME("Temperature = %f *C\r\n", v);
    }
    if (stat(_bme_humid.c_str(), &buf) == 0) {
        v = readFloat(_bme_humid);
        writeBME("Humidity = %f \%\r\n", v);
    }
    if (stat(_bme_press.c_str(), &buf) == 0) {
        v = readFloat(_bme_press);
        writeBME("Pressure = %f hPa\r\n", v);
    }
    if (stat(_bme_alt.c_str(), &buf) == 0) {
        v = readFloat(_bme_alt);
        writeBME("Approx. Altitude = %f m\r\n", v);
    }
}
