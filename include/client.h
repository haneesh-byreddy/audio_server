#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <filesystem>
#include <arpa/inet.h>
#include <thread>
#include <cstring>
#include <cctype>
#include <vector>
#include <pulse/simple.h>
#include <pulse/error.h>

class Client {
    public:
        int client_socket = socket(AF_INET, SOCK_STREAM, 0);
        sockaddr_in socket_addr;
        struct WAVHeader {
            char riff[4] = {'R', 'I', 'F', 'F'};
            uint32_t overall_size;
            char wave[4] = {'W', 'A', 'V', 'E'};
            char fmt_chunk_marker[4] = {'f', 'm', 't', ' '};
            uint32_t length_of_fmt = 16;
            uint16_t format_type = 1;
            uint16_t channels = 2;
            uint32_t sample_rate = 44100;
            uint32_t byterate;
            uint16_t block_align;
            uint16_t bits_per_sample = 16;
            char data_chunk_header[4] = {'d', 'a', 't', 'a'};
            uint32_t data_size;
        };
        std::string audioData;

        int Connect(std::string ip_addr, int port);
        int IsAccepted();
        int SendMessage(std::string message);
        std::string ReceiveMessage();
        int SendStream();
        int ReceiveStream();
        int RecordAudio();
};