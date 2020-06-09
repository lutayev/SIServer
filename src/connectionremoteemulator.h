#ifndef CONNECTIONREMOTEEMULATOR_H
#define CONNECTIONREMOTEEMULATOR_H
#include "connection.h"

class ConnectionRemoteEmulator : public Connection
{
public:
    ConnectionRemoteEmulator(int fd);
protected:
    virtual void processMessage(const std::string& msg);
};

#endif // CONNECTIONREMOTEEMULATOR_H
