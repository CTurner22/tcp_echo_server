// ********************************************************
// * A common set of system include files needed for socket() programming
// ********************************************************
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdarg.h>

#include <memory>

class Logger {
public:
    Logger(int verbosity);
    void info(const int level, const std::string &msg);
    void error(const int level, const std::string &msg);
    void info(const int level, const char *format , ... );
    void error(const int level, const char *format , ... );

private:
    const int VERBOSITY_LEVEL;

};

int processConnection(int sockFd, std::shared_ptr<Logger> logger);
