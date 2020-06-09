#include "server.h"

Server::Server(unsigned short port, ServerType type)
    : m_portListenClients(port), m_serverType(type)
{
    //Initial preparing
#ifdef _WIN32
    WSADATA WSAData;
    if (WSAStartup (MAKEWORD(1,1), &WSAData)!=0)
    {
        std::cout << "WSAStartup faild. Error:" << WSAGetLastError();
        m_ok = false;
        return;
    }
#endif
    if ((m_srvSock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cout <<"socket failed\n";
        m_ok = false;
        return;
    }

    m_addrListenClients.sin_family = AF_INET;
    m_addrListenClients.sin_port = htons(m_portListenClients);
    m_addrListenClients.sin_addr.s_addr = INADDR_ANY;

    //Bind
    if (bind(m_srvSock, reinterpret_cast<sockaddr*>(&m_addrListenClients), sizeof(m_addrListenClients)) != 0) {
        printf("Bind error #%d\n", errno);
        m_ok = false;
        return;
    }

    m_ok = true;
}

Server::~Server()
{
    std::cout << "Server shutdown" << std::endl;
    std::lock_guard<std::mutex> guard(m_mtxClientsMap);
    m_clients.clear();
    if (m_srvSock)
        close(m_srvSock);
}

void Server::acceptClients() {

    //Listen
    if (listen(m_srvSock, 2) != 0) {
        printf("ListenError #%d\n", errno);
        m_ok = false;
        return;
    }

    struct sockaddr_in from;
#ifdef __linux__
    socklen_t fromlen = sizeof (from);
#elif _WIN32
    int fromlen = sizeof(from);
#endif

    //Infinite loop of accepts untill stop is set
    while(true) {
        //Select
        int selRes;
        struct timeval tv;
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(m_srvSock, &rfds);
        tv.tv_sec = 15;
        tv.tv_usec = 0;
        selRes = select(m_srvSock + 1, &rfds, (fd_set *) 0, (fd_set *) 0, &tv);

        //Accept
        if(selRes > 0) {
            unsigned short client = accept(m_srvSock, reinterpret_cast<sockaddr*>(&from), &fromlen);
            std::thread th(&Server::addClient, this, client);
            th.detach();
        }

        //Stop
        if (stop) {
            return;
        }
    }
}

void Server::acceptManagers()
{

}

void Server::addClient(unsigned short clientSocket)
{
    if (serverType == ServerType::remoteEmulatorServer)
        Connection* client = new ConnectionRemoteEmulator(clientSocket);
    else
        Connection* client = new Connection(clientSocket);

    //Insert client to map
    m_mtxClientsMap.lock();
    m_clients[clientSocket] = client;
    printClientInfo(clientSocket);
    m_mtxClientsMap.unlock();
    //END insert

    //While client is alive, it communicate
    client->communicate();

    //Client disconnected, delete it
    m_mtxClientsMap.lock();
    delete client;
    m_clients.erase(clientSocket);
    m_mtxClientsMap.unlock();
}

bool Server::interactServer()
{
    std::string help =  "\t help - help\n"
                        "\t list - client list\n"
                        "\t connect <id> - connect to client\n";
    std::string command = "";
    std::cout << "Enter command (help for help)." << std::endl;
    while (command != "stop") {
        std::cout << "server#";
        std::cin >> command;
        if (command == "help") {
            std::cout << help << std::endl;
            continue;
        } else if (command == "list") {
            m_mtxClientsMap.lock();
            for (auto it : m_clients) {
                std::cout << it.second->id() << std::endl;
            }
            m_mtxClientsMap.unlock();
            continue;
        } else if (command == "connect") {
            std::string clientId;
            std::cin >> clientId;
            unsigned int sock = 0;

            m_mtxClientsMap.lock();
            for (auto it : m_clients) {
                if (clientId == it.second->id()) {
                    sock = it.first;
                    break;
                }
            }
            m_mtxClientsMap.unlock();

            if (sock) {
                if (!interactClient(sock, clientId))
                    std::cout << "Client unexpectedly disconnected from server\n";
            } else {
                std::cout << "Client not found!\n";
            }
            continue;
        } else {
            std::cout << "Enter command (help for help).\n";
        }
    }
    std::cout << "Exit interact mode" << std::endl;
    return true;
}

bool Server::interactClient(unsigned int clientSocket, const std::string& id)
{
    std::string command;
    while(true) {
        std::cout << "remote@" << id << "#";
        std::cin >> command;
        if (command == "exit")
            return true;

        m_mtxClientsMap.lock();
        if (m_clients.find(clientSocket) != m_clients.end()) {
            m_clients[clientSocket]->pushCommand(command);
        } else {
            m_mtxClientsMap.unlock();
            return false;
        }
        m_mtxClientsMap.unlock();
    }
}

bool Server::isOk()
{
    return m_ok;
}

void Server::printClientInfo(unsigned int socket)
{
    struct sockaddr_in from;
#ifdef __linux__
    socklen_t fromlen = sizeof (from);
#elif _WIN32
    int fromlen = sizeof(from);
#endif

    getpeername(socket, reinterpret_cast<sockaddr*>(&from), &fromlen);
    char *connected_ip= inet_ntoa(from.sin_addr);
    int port = ntohs(from.sin_port);
    std::cout << "\nNew connection from:\t" << connected_ip << ":" << port << std::endl;
}
