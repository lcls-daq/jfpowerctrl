#ifndef Pds_Jungfrau_Server_hh
#define Pds_Jungfrau_Server_hh

#include <poll.h>

namespace Pds {
  namespace Jungfrau {
    class Connection {
    public:
      Connection(int fd, const unsigned bufsz=1024);
      ~Connection();
      void shutdown();
      bool closed() const;
      bool process();
    private:
      void parse();
    private:
      const unsigned _bufsz;
      bool           _overflow;
      int            _fd;
      char*          _wpos;
      char*          _buf;
    };

    class Server {
    public:
      Server(const unsigned port, const unsigned max_conns);
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
      Connection**   _conns;
      pollfd*        _pfds;
      pollfd*        _conn_pfds;
    };
  }
}

#endif
