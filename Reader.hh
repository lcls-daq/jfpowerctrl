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
      std::string read_raw_value(std::string cmd, int id=-1) const;
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

      bool set_power(unsigned value) const;
      int get_power() const;
      int get_temp() const;
      int get_voltage() const;
      int get_current() const;
      std::string get_name() const;

    private:
      const int _id;
    };

    class LedControl : public Control {
    public:
      LedControl(std::string path);
      virtual ~LedControl();

      bool set_led(unsigned mask) const;
      unsigned get_led() const;
      int get_led_green() const;
      int get_led_red() const;
      int get_led_yellow() const;
      bool set_led_green(unsigned value) const;
      bool set_led_red(unsigned value) const;
      bool set_led_yellow(unsigned value) const;
    };

    class MiscControl : public Control {
    public:
      MiscControl(std::string path);
      virtual ~MiscControl();

      int get_autostart_enable() const;
      int get_fanctrl_enable() const;
      int get_flowmeter_enable() const;
      int get_inhibit() const;
      int get_inhibit_enable() const;
      int get_powerswitch() const;
    };

    class GpioControl : public Control {
    public:
      GpioControl(std::string path, const int id=0);
      virtual ~GpioControl();

      int get_ac_warning() const;
      int get_dc_warning() const;
      int get_temp_warning() const;
      int get_power_supply_onoff() const;
      bool set_power_supply_onoff(unsigned value) const;

      int get_mcb(const int id) const;
      int get_mcb_mask() const;
      bool set_mcb(const int id, unsigned value) const;
      bool set_mcb_mask(unsigned value) const;
      static bool valid_mcb(const int id);

      static const int NUM_MCB = 12;

    private:
      std::string mcbcmd(int id) const;

    private:
      const int _id;
    };

    class CommandRunner {
    public:
      CommandRunner(std::string path, std::string block,
                    const unsigned num_ps, const unsigned num_gpios);
      ~CommandRunner();
      std::string run(const std::string& cmd) const;

    private:
      std::string int_to_reply(int value) const;
      std::string run_led(const std::string& cmd,
                          const std::string& value) const;
      std::string run_ps(const std::string& prefix,
                         const std::string& cmd,
                         const std::string& value) const;
      std::string run_gpios(const std::string& prefix,
                            const std::string& cmd,
                            const std::string& value) const;
      bool is_ps_cmd(const std::string& cmd) const;
      bool is_gpio_cmd(const std::string& cmd) const;
      bool is_led_cmd(const std::string& cmd) const;
      bool is_mcb_cmd(const std::string& cmd) const;
      bool is_warn_cmd(const std::string& cmd) const;
      bool check_cmd(const std::string& type, const std::string& cmd) const;
      int get_mcb_index(const std::string& cmd, const char match) const;

      static const std::string PSCMD;
      static const std::string GPIOCMD;
      static const std::string LEDCMD;
      static const std::string MCBCMD;
      static const std::string WARNCMD;

    private:
      const unsigned  _num_ps;
      const unsigned  _num_gpios;
      Block*          _block;
      LedControl*     _led;
      MiscControl*    _misc;
      PowerControl**  _ps;
      GpioControl**   _gpio;
    };
  }
}

#endif
