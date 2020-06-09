#ifndef CONNECTIONCLIENT_H
#define CONNECTIONCLIENT_H

#include "connection.h"

class ConnectionClient : public Connection
{
public:
    ConnectionClient(int fd);
};

#endif // CONNECTIONCLIENT_H
