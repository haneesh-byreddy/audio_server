#include "server.h"

int count = 0;

int Listen(int client, int index, Server* server) {
    Server::ReceiveCommand(client, index, *server);
    LOG(LOGGER_LEVEL_INFO, "Ended client : %d", index);
    close(client);
    count--;
}

int main(int argc, char* argv[]) {
    Server server;
    if (argc < 2) {
        LOG(LOGGER_LEVEL_INFO, "Usage: ./%s <NumberOfAllowedClients>", argv[0]);
        return 1;
    }
    int client_socket = Server::Start(9000, count);
    while(true) {
        while (count >= std::stoi(argv[1])) std::this_thread::sleep_for(std::chrono::seconds(1));
        int client = Server::Accept(client_socket, count);
        std::thread t1(Listen, client, count, &server);
        t1.detach();
        count++;
    }
    return 0;
}