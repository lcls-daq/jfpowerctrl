#ifndef Pds_Jungfrau_Reader_hh
#define Pds_Jungfrau_Reader_hh

#include <string>

namespace Pds {
  namespace Jungfrau {
    class Block {
    public:
      Block(std::string block);
      ~Block();
      bool is_active() const;
    private:
      std::string _block;
    };

    class Control {
    protected:
      Control(std::string path, std::string type, std::string dev);
      virtual ~Control();
      int read_value(std::string cmd, int id=-1) const;
      bool write_value(unsigned value, std::string cmd, int id=-1) const;

    private:
      std::string filename(std::string cmd, int id) const;

    private:
      const char  _sep;
      std::string _path;
      std::string _type;
      std::string _dev;
    };

    class PowerControl : public Control {
    public:
      PowerControl(std::string path, const int id=0);
      virtual ~PowerControl();

      bool set_power(unsigned value);
      int get_power() const;
      int get_temp() const;
      int get_voltage() const;
      int get_current() const;

    private:
      const int _id;
    };
  }
}

#endif
