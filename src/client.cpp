#include "client.h"

namespace fs = std::filesystem;

int Client::Connect(std::string ipaddr, int port) {
    socket_addr.sin_family = AF_INET;
    socket_addr.sin_port = htons(port);
    socket_addr.sin_addr.s_addr = inet_addr(ipaddr.c_str());

    if (connect(client_socket, (struct sockaddr*)&socket_addr, sizeof(socket_addr))==0){
        LOG(LOGGER_LEVEL_INFO, "Connected");
    }
    return 0;
}

int Client::IsAccepted() {
    char chunk[1024] = {0};
    std::cout<<"Waiting for server to Accept"<<std::flush;
    std::string buffer = ReceiveMessage();
    std::cout<<"\r"<<std::string(30,' ')<<"\r"<<buffer<<std::endl;
}

std::string Client::ReceiveMessage() {
    std::string buffer = "";
    ssize_t curr_size = 0, total_size = 0, remaining = 0;
    char chunk[1024];

    recv(client_socket, &total_size, sizeof(total_size), 0);
    remaining = total_size;
    if (total_size <= 0) {
        std::cerr << "Error: Invalid message size received." << std::endl;
        return "";
    }
    do {
        if (remaining == 0) break;
        memset(chunk, 0, sizeof(chunk));
        curr_size = recv(client_socket, chunk, sizeof(chunk), 0);
        if (curr_size > 0) buffer.append(chunk, curr_size);
        remaining-=curr_size;
    } while (remaining > 0);

    LOG(LOGGER_LEVEL_INFO, "Received Size : %d", buffer.length());
    std::string ack = "ACK";
    send(client_socket, ack.c_str(), ack.length(), 0);
    return buffer;
}

int Client::SendMessage(std::string message) {
    ssize_t n;
    size_t total_sent = 0, bytes_left = message.length();
    LOG(LOGGER_LEVEL_INFO, "Sending Server Size : %d", bytes_left);
    if (send(client_socket, &bytes_left, sizeof(bytes_left), 0) < 0) return -1;
    while (total_sent < message.length()) {
        n = send(client_socket, message.c_str()+total_sent, bytes_left, 0);
        if (n < 0) {
            LOG(LOGGER_LEVEL_INFO, "Sending failed");
            return -1;
        }
        total_sent+=n;
        bytes_left-=n;
    }
    char buffer[3];
    if (recv(client_socket, buffer, sizeof(buffer), 0)<0) return -1;
    return 0;
}

int Client::RecordAudio() {
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

int Client::SendStream() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    LOG(LOGGER_LEVEL_INFO, "Recorded %d bytes of audio data", audioData.size());
    if (SendMessage(audioData)<0) std::cerr << "Error sending audio data" << std::endl;
    audioData = "";
    ReceiveStream();
    return 0;
}

void playAudioNonBlocking(const std::string& command) {
    system(command.c_str());
}

int Client::ReceiveStream() {
    std::string ReceivedData = ReceiveMessage();
    
    WAVHeader header;
    header.byterate = header.sample_rate * header.channels * (header.bits_per_sample / 8);
    header.block_align = header.channels * (header.bits_per_sample / 8);
    header.data_size = ReceivedData.size();
    header.overall_size = header.data_size + sizeof(WAVHeader) - 8;

    std::ofstream outFile("recorded_audio_client.wav", std::ios::binary);
    if (outFile) {
        outFile.write(reinterpret_cast<const char*>(&header), sizeof(WAVHeader));
        
        outFile.write(ReceivedData.data(), ReceivedData.size());
        LOG(LOGGER_LEVEL_INFO, "Audio data saved to 'recorded_audio_client.wav'");
    } else {
        LOG(LOGGER_LEVEL_INFO, "Failed to save audio data to file.");
    }
    std::string command = "aplay recorded_audio_client.wav";
    std::thread audioThread(playAudioNonBlocking, command);
    audioThread.detach();
    SendStream();
    return 0;
}