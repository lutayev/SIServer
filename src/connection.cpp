#include "connection.h"
#include "powrprof.h"

Connection::Connection(unsigned short fd)
{
    m_sock = fd;
	done = false;
	m_error = SUCCESS;

    m_logFile = "log_" + m_id + ".txt";

	m_log.open(m_logFile.c_str(), 
			std::ios_base::out | std::ios_base::ate | std::ios_base::app);

	if (!m_log.good())
		m_error = LOG_NOT_OPENED;

}

void Connection::communicate()
{
    std::string message;
    for(;;)
    {
        //cout << "listening for incoming message...\n";
        message = readMessage();
        if (!message.size()) {
            break;
        }
        std::cout << "Message: " << message << std::endl;
        processMessage(message);
    }
}

std::string Connection::id()
{
    return m_id;
}

void Connection::pushCommand(const std::string &command)
{
    m_mtxCmd.lock();            //LOCK
    m_commands.push(command);
    m_mtxCmd.unlock();          //UNLOCK
}

std::string Connection::readMessage()
{
    char buf[Protocol::BUFSIZE];
    m_error = SUCCESS;
    std::string message = "";
    int result = recv(m_sock, buf, Protocol::BUFSIZE, 0);
    //Check read
    if (result == 0 || result == SOCKET_ERROR) {
        std::cout << "Erorr communication" << std::endl;
        m_error = WRONG_COMMUNICATION;
        return "";
    }

    //Check protocol
    if (buf[0] != Protocol::SOH) {
        std::cout << "Wrong message" << std::endl;
        m_error = WRONG_MESSAGE;
        return "";
    }

    message = std::string(buf, 0, result);

    return message;
}

bool Connection::writeMessage(const std::string &msg)
{
    int result = send(m_sock, msg.c_str(), msg.size(), 0);
    if (result == 0 || result == SOCKET_ERROR) {
        return false;
    }
    return true;
}

void Connection::processMessage(const std::string &msg)
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
    case Protocol::USR_MUTE:
        //SetSuspendState(true, true, true);
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

    default:
        std::cout << "Unknown message" << std::endl;
        response = Protocol::createRequest(Protocol::NAK);
        writeMessage(response);
    }
}

std::string Connection::popCommand()
{
    std::string command;
    m_mtxCmd.lock();        //LOCK
    if (!m_commands.empty()) {
        command = m_commands.front();
        m_commands.pop();
    }
    m_mtxCmd.unlock();      //UNLOCK
    return command;
}

Connection::~Connection()
{
    std::cout << "Client " << m_id << " disconnected" << std::endl;
	if (m_log.good())
		m_log.close();

    if (m_sock != -1)
        close(m_sock);
}
