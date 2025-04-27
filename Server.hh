#ifndef Pds_Jungfrau_Server_hh
#define Pds_Jungfrau_Server_hh

#include <poll.h>
#include <string>

namespace Pds {
  namespace Jungfrau {
    class Simulator;
    class CommandRunner;

    class Connection {
    public:
      Connection(int fd, CommandRunner* cmd, const unsigned bufsz=1024);
      ~Connection();
      void shutdown();
      bool closed() const;
      bool process();

    private:
      std::string buffer_to_str(char* buffer) const;
      bool reply(std::string cmd);
      bool parse();

    private:
      const unsigned _bufsz;
      bool           _overflow;
      int            _fd;
      char*          _wpos;
      char*          _buf;
      CommandRunner* _cmd;
    };

    class Server {
    public:
      Server(std::string name,
             std::string path,
             std::string block,
             const unsigned port,
             const unsigned max_conns,
             Simulator* sim = NULL,
             const unsigned num_ps=1,
             const unsigned num_gpios=1,
             const unsigned num_gfm=0,
             const unsigned num_fan=0);
      ~Server();
      void run();

    private:
      void add(unsigned idx, int fd);
      void remove(unsigned idx);
      void prune();
      bool accept();

    private:
      const unsigned _max_conns;
      const unsigned _server_idx;
      const unsigned _conn_idx;
      bool           _up;
      nfds_t         _nfds;
      unsigned       _nconns;
      int            _server_fd;
      Simulator*     _sim;
      CommandRunner* _cmd;
      Connection**   _conns;
      pollfd*        _pfds;
      pollfd*        _conn_pfds;
    };
  }
}

#endif
