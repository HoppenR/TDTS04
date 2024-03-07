#pragma once

#include <cstdint>
#include <netinet/in.h>
#include <vector>

class Server {
public:
    Server(uint16_t);
    ~Server();
    void connect();
    void disconnect();
    void send(std::vector<uint8_t> const&) const;
    std::vector<uint8_t> recv() const;

private:
    int socketfd;
    int client_socket;
    sockaddr_in bind_data;
};
