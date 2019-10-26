#include "Server.hh"
#include "Reader.hh"

#include <cstdio>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace Pds::Jungfrau;

Connection::Connection(int fd, CommandRunner* cmd, const unsigned bufsz) :
  _bufsz(bufsz),
  _overflow(false),
  _fd(fd),
  _wpos(NULL),
  _buf(new char[bufsz]),
  _cmd(cmd)
{
  _wpos = _buf;
}

Connection::~Connection()
{
  shutdown();
  if (_buf) {
    delete[] _buf;
  }
}

void Connection::shutdown()
{
  if (_fd >= 0) {
    ::close(_fd);
    _fd = -1;
  }
}

bool Connection::closed() const
{
  return _fd < 0;
}

bool Connection::process()
{
  int nread = ::recv(_fd, _wpos, _bufsz - (_wpos - _buf) - 1, 0);
  if(nread > 0) {
    // null-terminated the buffer
    _wpos[nread] = '\0';
    if (!parse())
      return false;
    if ((_wpos - _buf) == nread) {
      _wpos = _buf;
      _overflow = true;
    }
    return true;
  } else {
    if (nread < 0)
      std::perror("Error: socket recv failed!");
    return false;
  }
}

std::string Connection::buffer_to_str(char* buffer) const
{
  return std::string(buffer, strlen(buffer));
}

bool Connection::reply(std::string cmd)
{
  if (_cmd) {
    std::string reply = _cmd->run(cmd);
    if (::send(_fd, reply.c_str(), reply.length(), 0) < 0) {
      std::perror("Error: socket send failed!");
      return false;
    } else {
      return true;
    }
  } else {
    return false;
  }
}

bool Connection::parse()
{
  bool partial = _buf[strlen(_buf) - 1] != '\n';
  char* last = NULL;
  char* cmd = strtok (_buf, "\r\n");
  while (cmd != NULL) {
    if (last) {
      if (_overflow) {
        _overflow = false;
      } else {
        if (!reply(buffer_to_str(last)))
          return false;
      }
    }
    last = cmd;
    cmd = strtok (NULL, "\r\n");
  }
  if(partial) {
    std::memmove(_buf, last, strlen(last));
    _wpos = _buf + strlen(last);
  } else {
    _wpos = _buf;
    if (last) {
      if (_overflow) {
        _overflow = false;
      } else {
         if (!reply(buffer_to_str(last)))
          return false;
      }
    }
  }

  return true;
}


Server::Server(std::string path, std::string block,
               const unsigned port, const unsigned max_conns,
               const unsigned num_ps, const unsigned num_gpios) :
  _max_conns(max_conns),
  _server_idx(0),
  _conn_idx(1),
  _up(false),
  _nfds(max_conns + 1),
  _nconns(0),
  _server_fd(-1),
  _cmd(new CommandRunner(path, block, num_ps, num_gpios)),
  _conns(new Connection*[max_conns]),
  _pfds(new pollfd[max_conns + 1]),
  _conn_pfds(NULL)
{
  struct sockaddr_in address;
  int opt = 1;

  // set the conn pfds pointer
  _conn_pfds = _pfds + _conn_idx;
  // initialize the poller array
  for (unsigned i=0; i<_nfds; i++) {
    _pfds[i].fd       = -1;
    _pfds[i].events   = POLLIN;
    _pfds[i].revents  = 0;
  }

  // setup the address
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(port);

  _server_fd = ::socket(AF_INET, SOCK_STREAM, 0);
  if (_server_fd < 0) {
    std::perror("Error: server socket creation failed");
  } else {
    if (::setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
      std::perror("Error: setsockopt failed on server socket");
    } else {
      if (::bind(_server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        std::perror("Error: bind failed for server socket"); 
      } else {
        if (::listen(_server_fd, _max_conns) < 0) {
          std::perror("Error: listen failed for server socket");
        } else {
          // add server fd to poller
          _pfds[_server_idx].fd = _server_fd;
          _up = true;
        }
      }
    }
  }
}

Server::~Server()
{
  _up = false;
  if (_server_fd >= 0) {
    ::close(_server_fd);
    _server_fd = -1;
  }
  if (_cmd) {
    delete _cmd;
  }
  if (_conns) {
    for (unsigned i=0; i<_max_conns; i++) {
      if (_conns[i]) {
        delete _conns[i];
      }
    }
    delete[] _conns;
  }
  if (_pfds) {
    delete[] _pfds;
  }
}

bool Server::accept()
{
  struct sockaddr_in address;
  int addrlen = sizeof(address);
  int fd = ::accept(_server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);

  if (fd < 0) {
    std::perror("Error: connection accept failed");
    return false;
  } else {
    if (_nconns < _max_conns) {
      unsigned new_idx = _max_conns;
      for (unsigned i=0; i<_max_conns; i++) {
        if (_conn_pfds[i].fd < 0) {
          new_idx = i;
          break;
        }
      }
      if (new_idx == _max_conns) {
        std::cerr << "Error: poller data structure unexpectedly full - server is fubar!!" << std::endl;
        return false;
      } else if (_conns[new_idx]) {
        std::cerr << "Error: poller and connection data structures don't match - server is fubar!!" << std::endl;
        return false;
      }

      add(new_idx, fd);
    } else {
      ::close(fd);
    }
    return true;
  }
}

void Server::add(unsigned idx, int fd)
{
  _conn_pfds[idx].fd = fd;
  _conns[idx] = new Connection(fd, _cmd);
  _nconns++;
}

void Server::remove(unsigned idx)
{
  if (_conn_pfds[idx].fd >= 0) {
    _conn_pfds[idx].fd = -1;
    if (_conns[idx]) {
      delete _conns[idx];
      _conns[idx] = NULL;
    }
    _nconns--;
  }
}

void Server::prune()
{
  for (unsigned i=0; i<_max_conns; i++) {
    if (_conn_pfds[i].fd >= 0) {
      if (_conns[i]) {
        if (_conns[i]->closed()) {
          remove(i);
        }
      } else {
        remove(i);
      }
    }
  }
}

void Server::run()
{
  while(_up) {
    // prune dead connections
    prune();

    int npoll = ::poll(_pfds, (nfds_t) _nfds, -1);
    if (npoll < 0) {
      _up = false;
      std::perror("Error: server poller failed");
    } else {
      for (unsigned i=0; i<_max_conns; i++) {
        if (_conn_pfds[i].revents & POLLIN) {
          if (!_conns[i]->process()) remove(i);
        }
      }

      if (_pfds[_server_idx].revents & POLLIN) {
        _up = accept();
      }
    }
  }
}

