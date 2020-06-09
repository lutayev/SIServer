#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <queue>
#include <atomic>
#include <mutex>

#include <string.h>
#include <unistd.h>
#include <stdio.h>

#ifdef __linux__
#include <dlfcn.h>
#include <sys/wait.h>
#include <sys/prctl.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#elif _WIN32
#include <winsock2.h>
#include <mswsock.h>
#endif

#include "protocol.h"

#ifdef __linux__
#define SOCKET_ERROR (-1)
#endif

class Connection
{
public:
    Connection(unsigned short fd);
    virtual ~Connection();

    void communicate();
    std::string id();

    void pushCommand(const std::string& command);
    virtual std::string readMessage();
    virtual bool writeMessage(const std::string& msg);

    enum ERRORS {SUCCESS = 0,
                 LOG_NOT_OPENED,
                 DATABASE_NOT_OPEN,
                 WRONG_COMMUNICATION,
                 WRONG_MESSAGE
                };

protected:
    virtual void processMessage(const std::string& msg);
    std::string popCommand();

    std::string     m_id;
    std::string     m_logFile;
    std::ofstream   m_log;
    std::mutex      m_mtxCmd;

    std::queue<std::string> m_commands;

    unsigned short  m_sock;
    int             m_error;
    bool            done;
};

#endif //CONNECTION_H
