#include "client.h"
#include "SocketTimeoutException.h"

#include <cstdint>
#include <cstring>
#include <netdb.h>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

/*
 * Creates a socket for a future server connection
 */
Client::Client() {
    this->socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->socketfd == -1) {
        throw std::runtime_error{
            std::string{ "Client: bad fd: " } + strerror(errno),
        };
    }
}

/*
 * Closes the socket to the server
 */
Client::~Client() {
    close(this->socketfd);
}

/*
 * Connect to the server by resolving its IP-address through the `host` string
 */
void Client::connect(std::string const& host) {
    // Get information about the real server
    addrinfo hints{ 0, AF_INET, SOCK_STREAM, 0, 0, 0, nullptr, nullptr };
    addrinfo* res;

    // Get address information
    int addrinfo_ret = getaddrinfo(host.c_str(), nullptr, &hints, &res);
    if (addrinfo_ret != 0) {
        throw std::runtime_error{
            std::string{ "Client: getaddrinfo error: " } +
                gai_strerror(addrinfo_ret),
        };
    }
    // NOTE: There can be multiple results, but we only care about the first
    //       one?
    if (res->ai_family != AF_INET) {
        throw std::runtime_error{ "Client: bad getaddr info type" };
    }
    this->bind_data = *reinterpret_cast<sockaddr_in*>(res->ai_addr);
    this->bind_data.sin_port = htons(80);

    int connect_ret = ::connect(this->socketfd,
                                reinterpret_cast<sockaddr*>(&this->bind_data),
                                sizeof(this->bind_data));
    if (connect_ret == -1) {
        throw std::runtime_error{
            std::string{ "Client: invalid connect call: " } + strerror(errno),
        };
    }

    // Set timeout to 0.5 seconds
    timeval timeout{ 0, 500000 };
    int sockopt_ret = setsockopt(
        socketfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    if (sockopt_ret == -1) {
        throw std::runtime_error{
            std::string{ "Client: error setting timeout: " } + strerror(errno),
        };
    }
    freeaddrinfo(res);
}

/*
 * Send data in `chunk` to the currently connected server
 */
void Client::send(std::vector<uint8_t> const& chunk) {
    ssize_t send_ret = ::send(this->socketfd, chunk.data(), chunk.size(), 0);
    if (send_ret == -1) {
        throw std::runtime_error{
            std::string{ "Client: invalid send call: " } + strerror(errno),
        };
    }
}

/*
 * Recieve data from the currently connected server
 * If the socket times out, throw a SocketTimeoutException
 */
std::vector<uint8_t> Client::recv() {
    std::vector<uint8_t> chunk(8192);
    ssize_t bytes_read = ::recv(this->socketfd, chunk.data(), chunk.size(), 0);
    if (bytes_read == -1) {
        if (errno == EWOULDBLOCK) {
            // Timed out
            throw SocketTimeoutException{};
        }
        throw std::runtime_error{
            std::string{ "Client: error recv data: " } + strerror(errno),
        };
    }
    chunk.resize(bytes_read);
    return chunk;
}
