// **************************************************************************************
// * Echo Strings (echo_s.cc)
// * -- Accepts TCP connections and then echos back each string sent.
// **************************************************************************************
#include "echo_s.h"


#define CLOSE_CONNECTION 0
#define QUITE_PROGRAM 1
#define BUFFER_SIZE 2048

int main (int argc, char *argv[]) {

  // ********************************************************************
  // * Process the command line arguments
  // ********************************************************************

    const int DEFAULT_VERBOSITY = 1;
    std::shared_ptr<Logger> logger;

    int opt;
    while ((opt = getopt(argc,argv,"v")) != -1) {
        switch (opt) {
        case 'v':
            logger = std::make_shared<Logger>(2);
            break;
        case ':':
        case '?':
        default:
          std::cout << "useage: " << argv[0] << " -v" << std::endl;
          exit(-1);
        }
    }

    if(!logger)
        logger = std::make_shared<Logger>(DEFAULT_VERBOSITY);

  // *******************************************************************
  // * Creating the inital socket is the same as in a client.
  // ********************************************************************
    int listenFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenFd == -1) {
        logger->error(1, "Failed to open socket: %s\n", strerror(errno));
        return -4;
    }

  logger->info(2,   "Calling Socket() assigned file descriptor %i\n", listenFd);

  // handle bind
  struct sockaddr_in servaddr;
  srand(time(NULL));
  int port = (rand() % 10000) + 1024;
  bzero(&servaddr, sizeof(servaddr));

  servaddr.sin_family = PF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(port);

  logger->info(2, "Calling bind(%i, %p, %i).\n", listenFd, &servaddr, sizeof(servaddr) );
  while(bind(listenFd, (sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
      if(errno == EADDRINUSE) {
          port = (rand() % 10000) + 1024;
          servaddr.sin_port = htons(port);
      } else {
          logger->error(1, "Failed to bind socket: %s\n", strerror(errno));
          exit(-1);
      }
  }

  logger->info(1, "Using port %i\n", port);


  // ********************************************************************
  // * Setting the socket to the listening state is the second step
  // * needed to being accepting connections.  This creates a queue for
  // * connections and starts the kernel listening for connections.
  // ********************************************************************
  int listenQueueLength = 1;
  logger->info(2, "Calling listen( %i, %i)\n", listenFd, listenQueueLength);

  if( listen(listenFd, listenQueueLength) < 0) {
      logger->error(1, "Failed to listen: %s\n", strerror(errno));
      exit(-1);
  }

    int quitProgram = 0;
    while (!quitProgram) {

        logger->info(2, "Calling accept( %i, NULL, NULL)\n", listenFd);
        int connFd = accept(listenFd, nullptr, nullptr);
        if(connFd < 0) {
            logger->error(1, "Error with accept: %s\n", strerror(errno));
            exit(-1);
        }

        logger->info(2, "We have received a connection on %i\n", connFd);
        quitProgram = processConnection(connFd, logger);
        close(connFd);
    }

  close(listenFd);
}
int processConnection(int sockFd, std::shared_ptr<Logger> logger) {

    char buffer[BUFFER_SIZE];
    while (true) {

        logger->info(2, "Calling read( %i, %p, %i)\n", sockFd, buffer, sizeof(buffer));
        // get next message
        memset(buffer, '\0', sizeof(buffer));
        size_t bytes = read(sockFd, buffer,sizeof(buffer) -1);
        if(bytes < 0) {
            logger->error(1, "Error with read: %s\n", strerror(errno));
            exit(-1);
        }

        logger->info(2, "Received %i bytes: %s\n", bytes, buffer);

        // parse for commands
        if(strncmp(buffer, "CLOSE", 5) == 0) {
            logger->info(2, "Received CLOSE command, closing connection...\n");
            return CLOSE_CONNECTION;
        }

        if(strncmp(buffer, "QUIT", 4) == 0) {
            logger->info(2, "Received QUITE command, closing connection and exiting ...\n");
            return QUITE_PROGRAM;
        }

        // perform echo
        size_t bytes_trans = 0;
        do {
            size_t temp = write(sockFd, buffer + bytes_trans, bytes - bytes_trans);
            if (temp < 0) {
                logger->error(1, "Error with read: %s\n", strerror(errno));
                exit(-1);
            }
            bytes_trans += temp;
        } while( bytes_trans < bytes);
    }
}

void Logger::error(const int level, const char *format, ...) {
    if(level <= VERBOSITY_LEVEL) {
        va_list arglist;
        va_start(arglist, format);
        vfprintf(stderr, format, arglist);
        va_end(arglist);
    }
}

void Logger::error(const int level, const std::string &msg) {
    if(level <= VERBOSITY_LEVEL)
        std::cerr << msg << std::endl;

}

void Logger::info(const int level, const char *format, ...) {
    if(level <= VERBOSITY_LEVEL) {
        va_list arglist;
        va_start(arglist, format);
        vprintf(format, arglist);
        va_end(arglist);
    }
}

void Logger::info(const int level, const std::string &msg) {
    if(level <= VERBOSITY_LEVEL)
        std::cout << msg << std::endl;

}
Logger::Logger(int verbosity) : VERBOSITY_LEVEL{verbosity} {}