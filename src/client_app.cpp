#include "client.h"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        LOG(LOGGER_LEVEL_INFO, "Usage: ./%s <IP> <PORT>", argv[0]);
        return 1;
    }
    std::string ip = argv[1];
    int port = std::stoi(argv[2]);
    std::string message;
    Client client;
    
    client.Connect(ip, port);
    client.IsAccepted();
    while(true) {
        std::cout<<"\033[32m->\033[0m ";
        getline(std::cin, message);
        if (message.compare("quit")==0) break;
        else if (message.compare(0,4,"call")==0) {
            std::thread record(&Client::RecordAudio, &client);
            record.detach();
            client.SendMessage("call");
            client.SendStream();
        }
        LOG(LOGGER_LEVEL_INFO, "Invalid cmd, supported : quit, call");
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
    close(client.client_socket);
    LOG(LOGGER_LEVEL_INFO, "Closed socket");
    return 0;
}
