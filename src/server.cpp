#include "server.h"

namespace fs = std::filesystem;

int Server::Start(int port, int index) {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))<0){
        std::cout << "Cannot reuse port" << std::endl;
        return 0;
    };
    sockaddr_in socket_addr;

    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons(port);
    socket_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_socket, (struct sockaddr*)&socket_addr, sizeof(socket_addr));
    std::cout << "Binded to port " << port << std::endl;
    listen(server_socket, 5);
    std::cout << "Listening for client : "<< index << std::endl;

    return server_socket;
}

int Server::Accept(int server_socket, int index) {
    int accepted_fd = accept(server_socket, nullptr, nullptr);
    std::cout << "Accepted client : " << index << std::endl;
    return accepted_fd;
}

std::string Server::ReceiveMessage(int accepted_fd, int index) {
    std::string buffer = "";
    ssize_t curr_size = 0, total_size = 0, remaining = 0;
    char chunk[1024];
    recv(accepted_fd, &total_size, sizeof(total_size), 0);
    remaining = total_size;
    if (total_size <= 0) {
        std::cerr << "Error: Invalid message size received." << std::endl;
        return "";
    }
    float prev_percent = 0.0f, percent_threshold = 1.0f;
    do {
        if (remaining == 0) break;
        memset(chunk, 0, sizeof(chunk));
        curr_size = recv(accepted_fd, chunk, sizeof(chunk), 0);
        if (curr_size > 0) buffer.append(chunk, curr_size);
        remaining-=curr_size;
        float curr_percent = ((float)(total_size-remaining)/total_size)*100;
        if (curr_percent-prev_percent >= percent_threshold) {
            prev_percent = curr_percent;
            std::cout<<"\rPercentage: "<<(int)curr_percent<<"%"<<std::flush;
        }
    } while (remaining > 0);
    std::cout<<"\r"<<std::string(30, ' ')<<"\r";
    std::cout << "Received size : " << buffer.length() << std::endl;
    std::string ack = "ACK";
    send(accepted_fd, ack.c_str(), ack.length(), 0);
    return buffer;
}

int Server::SendMessage(int accepted_fd, std::string message, int index) {
    ssize_t n;
    size_t total_sent = 0, bytes_left = message.length();
    std::cout << "Sending client " << index <<" Size : " << bytes_left << std::endl;
    if (send(accepted_fd, &bytes_left, sizeof(bytes_left), 0) < 0) return -1;
    while (total_sent < message.length()) {
        n = send(accepted_fd, message.c_str()+total_sent, bytes_left, 0);
        if (n < 0) {
            std::cout << "Sending failed" << std::endl;
            return -1;
        }
        total_sent+=n;
        bytes_left-=n;
    }
    char buffer[3];
    if (recv(accepted_fd, buffer, sizeof(buffer), 0)<0) return -1;
    return 0;
}

int Server::RecordAudio() {
    pa_simple *s = nullptr;
    int error;
    static const pa_sample_spec ss = {
        .format = PA_SAMPLE_S16LE,
        .rate = 44100,
        .channels = 2
    };
    s = pa_simple_new(NULL, "MyRecorder", PA_STREAM_RECORD, NULL, "record", &ss, NULL, NULL, &error);
    if (!s) {
        std::cerr << "pa_simple_new() failed: " << pa_strerror(error) << std::endl;
        return 1;
    }
    std::vector<uint8_t> buffer(1024);
    while(true) {
        if (pa_simple_read(s, buffer.data(), buffer.size(), &error) < 0) {
            std::cerr << "pa_simple_read() failed: " << pa_strerror(error) << std::endl;
            break;
        }
        audioData.append(reinterpret_cast<char*>(buffer.data()), buffer.size());
    }
    if (s) pa_simple_free(s);
}

int Server::SendStream(int accepted_fd, int index) {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "Recorded " << audioData.size() << " bytes of audio data." << std::endl;
    if (Server::SendMessage(accepted_fd, audioData, index)<0) std::cerr << "Error sending audio data" << std::endl;
    audioData = "";
    ReceiveStream(accepted_fd, index);
    return 0;
}

void playAudioNonBlocking(const std::string& command) {
    system(command.c_str());
}

int Server::ReceiveStream(int accepted_fd, int index) {
    std::string ReceivedData = Server::ReceiveMessage(accepted_fd, index);
    
    WAVHeader header;
    header.byterate = header.sample_rate * header.channels * (header.bits_per_sample / 8);
    header.block_align = header.channels * (header.bits_per_sample / 8);
    header.data_size = ReceivedData.size();
    header.overall_size = header.data_size + sizeof(WAVHeader) - 8;

    std::ofstream outFile("recorded_audio_server.wav", std::ios::binary);
    if (outFile) {
        outFile.write(reinterpret_cast<const char*>(&header), sizeof(WAVHeader));
        
        outFile.write(ReceivedData.data(), ReceivedData.size());
        std::cout << "Audio data saved to 'recorded_audio.wav'" << std::endl;
    } else {
        std::cerr << "Failed to save audio data to file." << std::endl;
    }
    std::string command = "aplay recorded_audio_server.wav";
    std::thread audioThread(playAudioNonBlocking, command);
    audioThread.detach();
    Server::SendStream(accepted_fd, index);
    return 0;
}

int Server::ReceiveCommand(int accepted_fd, int index, Server& server) {
    Server::SendMessage(accepted_fd, "Server Accepted", index);
    while(true) {
        std::string received = Server::ReceiveMessage(accepted_fd, index);
        if (received.compare("")==0) break;
        if (received.compare("call")==0) {
            std::thread record(&Server::RecordAudio);
            Server::ReceiveStream(accepted_fd, index);
        }
    }
    return 0;
}