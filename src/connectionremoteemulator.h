#ifndef CONNECTIONREMOTEEMULATOR_H
#define CONNECTIONREMOTEEMULATOR_H
#include "connection.h"
#include "powrprof.h"

class ConnectionRemoteEmulator : public Connection
{
public:
    ConnectionRemoteEmulator(int fd);
protected:
    virtual void processMessage(const std::pair<uint8_t, std::string>& message);
    bool changeAudioVolume(float level);
    bool shutdown();
    bool mouseMove(int x, int y);
};

#endif // CONNECTIONREMOTEEMULATOR_H
