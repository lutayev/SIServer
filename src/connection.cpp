#include "connection.h"

Connection::Connection(unsigned short fd)
{
    m_sock = fd;
	done = false;
    //m_error = ;

    m_logFile = "log_" + m_id + ".txt";

	m_log.open(m_logFile.c_str(), 
			std::ios_base::out | std::ios_base::ate | std::ios_base::app);

    //if (!m_log.good())
    //	m_error = LOG_NOT_OPENED;

}

void Connection::communicate()
{
    std::pair<uint8_t, std::string> message;
    for(;;)
    {
        message.first = 0;
        message.second.clear();
        //cout << "listening for incoming message...\n";
        Protocol::ERRORS err = Protocol::readMessage(m_sock, message);
        if (err == Protocol::ERRORS::WRONG_COMMUNICATION) {
            std::cout << "Wrong communication" << std::endl;
            break;
        } else if (err == Protocol::ERRORS::WRONG_MESSAGE) {
            std::cout << "Wrong message" << std::endl;
        }

        std::cout << "Message: " << message.second << std::endl;
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

void Connection::processMessage(const std::pair<uint8_t, std::string>& message)
{

    uint8_t command         = message.first;
    std::string text        = message.second;

    std::string response;

    switch (command) {
    case Protocol::CL_REQ_COMMAND:
        std::cout << "Client requested command" << std::endl;
        response = popCommand();
        if (!response.size())
            response = "wait";
        Protocol::writeMessage(m_sock, Protocol::SRV_SND_COMMAND, response);
        break;
    case Protocol::CL_SND_ID:
        std::cout << "Client sent ID " << text << std::endl;
        m_id = text;
        Protocol::writeMessage(m_sock, Protocol::ACK);
        break;
    case Protocol::CL_REQ_ID:
        std::cout << "Client requested ID" << std::endl;
        Protocol::writeMessage(m_sock, Protocol::SRV_SND_TEXT, "testId");
        break;
    case Protocol::CL_REQ_FILE:
        std::cout << "Client requested file" << std::endl;
        Protocol::writeMessage(m_sock, Protocol::NAK);
        break;
    case Protocol::CL_SND_PRINT:
        std::cout << "Client sent printable info" << std::endl;
        std::cout << text << std::endl;
        Protocol::writeMessage(m_sock, Protocol::ACK);
        break;

    default:
        std::cout << "Unknown message" << std::endl;
        Protocol::writeMessage(m_sock, Protocol::NAK);
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
