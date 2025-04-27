#include "Server.hh"
#include "Reader.hh"
#include "Simulator.hh"

#include <getopt.h>
#include <cstdlib>
#include <iostream>
#include <string>

using namespace Pds::Jungfrau;

static std::string JungfrauPowerControlVersion = "1.0";

static void showVersion(const char* p)
{
  std::cout << "Version:  " << p << "  Ver " << JungfrauPowerControlVersion << std::endl;
}

static void showUsage(const char* p)
{
  std::cout << "Usage: " << p << " [-v|--version] [-h|--help]" << std::endl
            << "-p|--path <path> -l|--logdir <logdir> [-P|--port <port>]" << std::endl
            << "[-c|--conn <connections>] [-b|--boards <nboards>]" << std::endl
            << "[-g|--gfms <ngfms>] [-f|--fans <nfans>] [--s|--sim]" << std::endl
            << "[-n|--name <name>]" << std::endl
            << " Options:" << std::endl
            << "    -p|--path     <path>                    the path to the power control scripts" << std::endl
            << "    -l|--logdir   <logdir>                  the logdir of the power control scripts" << std::endl
            << "    -n|--name     <name>                    the name of the device (default: JF4MD-CTRL)" << std::endl
            << "    -P|--port     <port>                    port to use for the server (default: 32415)" << std::endl
            << "    -c|--conn     <connections>             maximum number of connections (default: 3)" << std::endl
            << "    -b|--boards   <nboards>                 number of power supply/gpio boards (default: 1)" << std::endl
            << "    -g|--gfms     <ngfms>                   number of flow meters (default: 0)" << std::endl
            << "    -f|--fans     <nfans>                   number of fans (default: 1)" << std::endl
            << "    -s|--sim                                simulate extra sensors" << std::endl
            << "    -v|--version                            show file version" << std::endl
            << "    -h|--help                               print this message and exit" << std::endl;
}

int main(int argc, char *argv[])
{
  const char*         strOptions  = ":vhp:l:n:P:c:b:g:f:s";
  const struct option loOptions[] =
  {
    {"ver",         0, 0, 'v'},
    {"help",        0, 0, 'h'},
    {"path",        1, 0, 'p'},
    {"logdir",      1, 0, 'l'},
    {"name",        1, 0, 'n'},
    {"port",        1, 0, 'P'},
    {"conn",        1, 0, 'c'},
    {"boards",      1, 0, 'b'},
    {"gfms",        1, 0, 'g'},
    {"fans",        1, 0, 'f'},
    {"sim",         0, 0, 's'},
    {0,             0, 0,  0 }
  };

  bool lUsage = false;
  bool simulate = false;
  unsigned port  = 32415;
  unsigned conns = 3;
  unsigned boards = 1;
  unsigned gfms = 0;
  unsigned fans = 1;
  std::string path;
  std::string logdir;
  std::string name = "JF4MD-CTRL";

  int optionIndex  = 0;
  while ( int opt = getopt_long(argc, argv, strOptions, loOptions, &optionIndex ) ) {
    if ( opt == -1 ) break;

    switch(opt) {
      case 'h':               /* Print usage */
        showUsage(argv[0]);
        return 0;
      case 'v':               /* Print version */
        showVersion(argv[0]);
        return 0;
      case 'p':
        path = std::string(optarg);
        break;
      case 'l':
        logdir = std::string(optarg);
        break;
      case 'n':
        name = std::string(optarg);
        break;
      case 'P':
        port = std::strtoul(optarg, NULL, 0);
        break;
      case 'c':
        conns = std::strtoul(optarg, NULL, 0);
        break;
      case 'b':
        boards = std::strtoul(optarg, NULL, 0);
        break;
      case 'g':
        gfms = std::strtoul(optarg, NULL, 0);
        break;
      case 'f':
        fans = std::strtoul(optarg, NULL, 0);
        break;
      case 's':
        simulate = true;
        break;
      case '?':
        if (optopt)
          std::cout << argv[0] << ": Unknown option: " << static_cast<char>(optopt) << std::endl;
        else
          std::cout << argv[0] << ": Unknown option: " << argv[optind-1] << std::endl;
        lUsage = true;
        break;
      case ':':
        std::cout << argv[0] << ": Missing argument for " << static_cast<char>(optopt) << std::endl;
        lUsage = true;
        break;
      default:
        lUsage = true;
        break;
    }
  }

  if (path.empty()) {
    std::cout << argv[0] << ": path to the power control scripts is required" << std::endl;
    lUsage = true;
  }

  if (logdir.empty()) {
    std::cout << argv[0] << ": path to the logdir of the power control scripts is required" << std::endl;
    lUsage = true;
  }

  if (optind < argc) {
    std::cout << argv[0] << ": invalid argument -- " << argv[optind] << std::endl;
    lUsage = true;
  }

  if (lUsage) {
    showUsage(argv[0]);
    return 1;
  }

  if (simulate) {
    Simulator sim(logdir);
    Server srv(name, path, logdir, port, conns, &sim, boards, boards, gfms, fans);
    srv.run();
  } else {
    Server srv(name, path, logdir, port, conns, NULL, boards, boards, gfms, fans);
    srv.run();
  }

  return 0;
}
