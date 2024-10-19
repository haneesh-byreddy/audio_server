#include "client.h"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr<<"Usage: "<<argv[0]<<" <IP> <PORT>"<<std::endl;
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
            client.SendMessage("call");
            client.SendStream();
        }
        else std::cout << "Invalid cmd supported : quit, call" << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
    close(client.client_socket);
    std::cout << "Closed socket" << std::endl;
    return 0;
}
