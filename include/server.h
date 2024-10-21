#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <unistd.h>
#include <thread>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>
#include <pulse/simple.h>
#include <pulse/error.h>


static std::string audioData;

class Server {
    public:
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
            uint16_t bits_per_sample = 16;  // Assuming 16-bit audio
            char data_chunk_header[4] = {'d', 'a', 't', 'a'};
            uint32_t data_size;
        };

        static int Start(int port, int index);
        static int Accept(int server_socket, int index);
        static int SendMessage(int fd, std::string message, int index);
        static std::string ReceiveMessage(int fd, int index);
        static int SendStream(int fd, int index);
        static int ReceiveStream(int fd, int index);
        static int ReceiveCommand(int fd, int index, Server& Server);
        static int RecordAudio();
};