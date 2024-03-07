#pragma once

#include <cstdint>
#include <netinet/in.h>
#include <string>
#include <vector>

class Client {
public:
    Client();
    ~Client();
    void connect(std::string const&);
    void send(std::vector<uint8_t> const&);
    std::vector<uint8_t> recv();

private:
    int socketfd;
    sockaddr_in bind_data;
};
