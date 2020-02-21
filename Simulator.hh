#ifndef Pds_Jungfrau_Simulator_hh
#define Pds_Jungfrau_Simulator_hh

#include <string>

namespace Pds {
  namespace Jungfrau {
    class Simulator {
    public:
      Simulator(std::string path);
      virtual ~Simulator();

      double readFloat(std::string filename) const;
      void writeBME(std::string format, double value) const;
      void checkBME() const;

    private:
      int            _bmefd;
      std::string    _bme_temp;
      std::string    _bme_humid;
      std::string    _bme_press;
      std::string    _bme_alt;
    };
  }
}

#endif
