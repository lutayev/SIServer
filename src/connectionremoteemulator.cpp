#include "connectionremoteemulator.h"

ConnectionRemoteEmulator::ConnectionRemoteEmulator(int fd) : Connection(fd)
{

}

void ConnectionRemoteEmulator::processMessage(const std::string &msg)
{
    //Parse incoming message
    std::pair<char, std::string> parsed;
    if (!Protocol::parseResponse(msg, parsed)) {
        std::cout << "Can't parse client's request" << std::endl;
        writeMessage(Protocol::createRequest(Protocol::NAK));
        return;
    }

    char command         = parsed.first;
    std::string text     = parsed.second;

    std::string response;

    switch (command) {
    case Protocol::CL_REQ_COMMAND:
        std::cout << "Client requested command" << std::endl;
        response = popCommand();
        if (!response.size())
            response = "wait";
        response = Protocol::createRequest(Protocol::SRV_SND_COMMAND, response);
        writeMessage(response);
        break;
    case Protocol::CL_SND_ID:
        std::cout << "Client sent ID " << text << std::endl;
        m_id = text;
        response = Protocol::createRequest(Protocol::ACK);
        writeMessage(response);
        break;
    case Protocol::CL_REQ_ID:
        std::cout << "Client requested ID" << std::endl;
        response = Protocol::createRequest(Protocol::SRV_SND_TEXT, "testId");
        writeMessage(response);
        break;
    case Protocol::CL_REQ_FILE:
        std::cout << "Client requested file" << std::endl;
        response = Protocol::createRequest(Protocol::NAK);
        writeMessage(response);
        break;
    case Protocol::CL_SND_PRINT:
        std::cout << "Client sent printable info" << std::endl;
        std::cout << parsed.second << std::endl;
        response = Protocol::createRequest(Protocol::ACK);
        writeMessage(response);
        break;
    case Protocol::USR_CMD:
        if (parsed.second == "mute") {
            INPUT input;
            input.type = INPUT_KEYBOARD;
            input.ki.dwFlags = 0;
            input.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
            input.ki.wScan=MapVirtualKey(VK_VOLUME_MUTE,0);
            input.ki.time = 0;
            input.ki.dwExtraInfo = 0;
            input.ki.wVk = VK_VOLUME_MUTE;
            SendInput(1,&input,sizeof(input));
            break;
        } else if (parsed.second == "sleep") {
            SetSuspendState(true, true, true);
        }

    default:
        std::cout << "Unknown message" << std::endl;
        response = Protocol::createRequest(Protocol::NAK);
        writeMessage(response);
    }
}
