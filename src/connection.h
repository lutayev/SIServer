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

#include "protocol.h"

class Connection
{
public:
    Connection(unsigned short fd);
    virtual ~Connection();

    void communicate();
    std::string id();

    void pushCommand(const std::string& command);

protected:
    virtual void processMessage(const std::pair<uint8_t, std::string>& message);
    std::string popCommand();

    std::string     m_id;
    std::string     m_logFile;
    std::ofstream   m_log;
    std::mutex      m_mtxCmd;

    std::queue<std::string> m_commands;

    unsigned short      m_sock;
    int                 m_error;
    bool                done;
};

#endif //CONNECTION_H
