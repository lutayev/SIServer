#include "server.h"

int main(int argc, char** argv) {
    Server server(8889);
    if (!server.isOk()) {
        std::cout << "Error bind or listen" << std::endl;
        return EXIT_FAILURE;
    }
    std::thread srvThread(&Server::acceptClients, &server);
    server.interactServer();

    sleep(30);
    server.stop = true;

    srvThread.join();
}
