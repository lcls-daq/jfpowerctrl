#ifndef Pds_Jungfrau_Reader_hh
#define Pds_Jungfrau_Reader_hh

#include <string>

namespace Pds {
  namespace Jungfrau {
    class File {
    public:
      File(std::string path, std::string name, const char sep='/');
      ~File();
      std::string filename() const;

    protected:
      std::string _filename;
    };

    class Logger : public File {
    public:
      enum Level { DEBUG, INFO, ERROR };
      Logger(std::string path, std::string name, Level level=INFO);
      ~Logger();
      void debug(const std::string& message) const;
      void info(const std::string& message) const;
      void error(const std::string& message) const;
    private:
      void log(const std::string& message) const;
      std::string datetime() const;
    private:
      const Level _level;
    };

    class Lock : public File {
    public:
      Lock(std::string path, std::string name);
      ~Lock();
      bool is_set() const;
      bool set() const;
      bool clear() const;
    };

    class Flag : public File {
    public:
      Flag(std::string path, std::string name);
      ~Flag();
      bool is_set() const;
      bool set() const;
      bool clear() const;

    private:
      bool read_flag() const;
      bool write_flag(bool flag) const;
    };

    class Control {
    protected:
      Control(std::string path, std::string type, std::string dev);
      virtual ~Control();
      std::string read_raw_value(std::string cmd, int id=-1) const;
      int read_value(std::string cmd, int id=-1) const;
      bool wait_value(int value, std::string cmd, unsigned long timeout, int id=-1) const;
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

    class FlowMeterControl : public Control {
    public:
      FlowMeterControl(std::string path, const int id=0);
      virtual ~FlowMeterControl();

      int get_temp() const;
      int get_flow() const;
      std::string get_name() const;

    private:
      const int _id;
    };

    class FanControl : public Control {
    public:
      FanControl(std::string path, const int id=0);
      virtual ~FanControl();

      int get_input() const;
      int get_target() const;
      int get_div() const;
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
      bool wait_ac_warning(int value, unsigned long timeout) const;
      bool wait_dc_warning(int value, unsigned long timeout) const;
      bool wait_temp_warning(int value, unsigned long timeout) const;
      int get_power_supply_onoff() const;
      bool set_power_supply_onoff(unsigned value) const;

      unsigned num_mcb_active() const;
      int get_mcb(const int id) const;
      int get_mcb_mask() const;
      int get_mcb_active(const int id) const;
      int get_mcb_active_mask() const;
      bool set_mcb(const int id, unsigned value) const;
      bool set_mcb_mask(unsigned mask, unsigned long pause=0) const;
      void set_mcb_active(const int id, unsigned value);
      void set_mcb_active_mask(unsigned mask);
      bool set_mcb_on(unsigned long pause=0) const;
      bool set_mcb_off(unsigned long pause=0) const;
      static bool valid_mcb(const int id);

      static const int NUM_MCB = 12;
      static const int ALL_ON = (1<<NUM_MCB) - 1;

    private:
      std::string mcbcmd(int id) const;

    private:
      const int _id;
      unsigned  _active;
    };

    class CommandRunner {
    public:
      CommandRunner(std::string name,
                    std::string path,
                    std::string logpath,
                    const unsigned num_ps,
                    const unsigned num_gpios,
                    const unsigned num_gfm,
                    const unsigned num_fan);
      ~CommandRunner();
      std::string run(const std::string& cmd);

    private:
      std::string on(bool verbose=false) const;
      std::string off(bool verbose=false) const;
      std::string toggle() const;
      std::string int_to_str(long value) const;
      std::string int_to_reply(long value) const;
      std::string lock_to_reply(const Lock* lock) const;
      std::string state() const;
      std::string run_led(const std::string& cmd,
                          const std::string& value) const;
      std::string run_ps(const std::string& prefix,
                         const std::string& cmd,
                         const std::string& value) const;
      std::string run_gfm(const std::string& prefix,
                           const std::string& cmd,
                           const std::string& value) const;
      std::string run_fan(const std::string& prefix,
                           const std::string& cmd,
                           const std::string& value) const;
      std::string run_gpios(const std::string& prefix,
                            const std::string& cmd,
                            const std::string& value) const;
      std::string run_base(const std::string& cmd,
                           const std::string& value);
      bool is_off() const;
      bool is_on() const;
      bool check_enables() const;
      bool check_ps() const;
      bool set_lock(const Lock* lock, const std::string& value) const;
      bool is_ps_cmd(const std::string& cmd) const;
      bool is_gfm_cmd(const std::string& cmd) const;
      bool is_fan_cmd(const std::string& cmd) const;
      bool is_gpio_cmd(const std::string& cmd) const;
      bool is_led_cmd(const std::string& cmd) const;
      bool is_mcb_cmd(const std::string& cmd) const;
      bool is_warn_cmd(const std::string& cmd) const;
      bool check_cmd(const std::string& type, const std::string& cmd) const;
      std::string get_mcb_prefix(const std::string& cmd) const;
      int get_mcb_index(const std::string& cmd,
                        const std::string& prefix,
                        const char match) const;
      unsigned num_active_modules() const;

      static const std::string PSCMD;
      static const std::string GFMCMD;
      static const std::string FANCMD;
      static const std::string GPIOCMD;
      static const std::string LEDCMD;
      static const std::string WARNCMD;
      static const std::string MCBCMDS[];

    private:
      const unsigned     _num_ps;
      const unsigned     _num_gpios;
      const unsigned     _num_gfm;
      const unsigned     _num_fan;
      std::string        _name;
      unsigned long      _pause;
      unsigned long      _timeout;
      Flag*              _state;
      Lock*              _block;
      Logger*            _logger;
      LedControl*        _led;
      MiscControl*       _misc;
      PowerControl**     _ps;
      Lock**             _ps_temp;
      GpioControl**      _gpio;
      FlowMeterControl** _gfm;
      Lock**             _gfm_flow;
      Lock**             _gfm_temp;
      FanControl**       _fan;
      Lock**             _fan_input;
    };
  }
}

#endif
