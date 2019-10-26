#include "Server.hh"
#include "Reader.hh"

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
            << "[-c|--conn <connections>]" << std::endl
            << " Options:" << std::endl
            << "    -p|--path     <path>                    the path to the power control scripts" << std::endl
            << "    -l|--logdir   <logdir>                  the logdir of the power control scripts" << std::endl
            << "    -P|--port     <port>                    port to use for the server (default: 32415)" << std::endl
            << "    -c|--conn     <connections>             maximum number of connections (default: 3)" << std::endl
            << "    -v|--version                            show file version" << std::endl
            << "    -h|--help                               print this message and exit" << std::endl;
}

int main(int argc, char *argv[])
{
  const char*         strOptions  = ":vhp:l:P:c:";
  const struct option loOptions[] =
  {
    {"ver",         0, 0, 'v'},
    {"help",        0, 0, 'h'},
    {"path",        1, 0, 'p'},
    {"logdir",      1, 0, 'l'},
    {"port",        1, 0, 'P'},
    {"conn",        1, 0, 'c'},
    {0,             0, 0,  0 }
  };

  bool lUsage = false;
  unsigned port  = 32415;
  unsigned conns = 3;
  std::string path;
  std::string logdir;

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
      case 'P':
        port = std::strtoul(optarg, NULL, 0);
        break;
      case 'c':
        conns = std::strtoul(optarg, NULL, 0);
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

  Server srv(path, logdir, port, conns);
  srv.run();

  return 0;
}
