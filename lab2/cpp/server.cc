#include "server.h"
#include "SocketTimeoutException.h"

#include <arpa/inet.h>
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

/*
 * Associates socket with port at localhost and starts listening
 * for connections
 */
Server::Server(uint16_t port) {
    this->socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->socketfd == -1) {
        throw std::runtime_error{
            std::string{ "Server: bad fd: " } + strerror(errno),
        };
    }

    int opt = 1;
    setsockopt(this->socketfd,
               SOL_SOCKET,
               SO_REUSEADDR | SO_REUSEPORT,
               &opt,
               sizeof(opt));

    this->bind_data.sin_family = AF_INET;
    this->bind_data.sin_addr.s_addr = INADDR_ANY;
    this->bind_data.sin_port = htons(port);

    if (bind(this->socketfd,
             reinterpret_cast<sockaddr*>(&bind_data),
             sizeof(bind_data)) == -1) {
        throw std::runtime_error{
            std::string{ "Server: failed bind call: " } + strerror(errno),
        };
    }

    if (listen(socketfd, 5) == -1) {
        throw std::runtime_error{
            std::string{ "Server: failed listen call: " } + strerror(errno),
        };
    }
}

/*
 * Closes the socket to the client as well as the socket for recieving new
 * connections
 */
Server::~Server() {
    close(this->client_socket);
    close(this->socketfd);
}

/*
 * Connect to the client (web browser)
 */
void Server::connect() {
    unsigned int addrlen = sizeof(this->bind_data);
    this->client_socket = accept(
        socketfd, reinterpret_cast<sockaddr*>(&this->bind_data), &addrlen);
    if (client_socket == -1) {
        throw std::runtime_error{
            std::string{ "Server: failed accept call: " } + strerror(errno),
        };
    }

    // Set timeout to 0.5 seconds
    timeval timeout{ 0, 500000 };
    int sockopt_ret = setsockopt(
        client_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    if (sockopt_ret == -1) {
        throw std::runtime_error{
            std::string{ "Server: error setting timeout: " } + strerror(errno),
        };
    }
}

/*
 * Closes the socket to the client
 */
void Server::disconnect() {
    close(this->client_socket);
}

/*
 * Send data in `chunk` to the currently connected client
 */
void Server::send(std::vector<uint8_t> const& chunk) const {
    ssize_t send_ret =
        ::send(this->client_socket, chunk.data(), chunk.size(), 0);
    if (send_ret == -1) {
        throw std::runtime_error{
            std::string{ "Server: invalid send call " } + strerror(errno),
        };
    }
}

/*
 * Recieve data from the currently connected client
 * If the socket times out, throw a SocketTimeoutException
 */
std::vector<uint8_t> Server::recv() const {
    std::vector<uint8_t> chunk(8192);
    ssize_t bytes_read =
        ::recv(this->client_socket, chunk.data(), chunk.size(), 0);
    if (bytes_read == -1) {
        if (errno == EWOULDBLOCK) {
            // Timed out
            throw SocketTimeoutException{};
        }
        throw std::runtime_error{
            std::string{ "Server: error recv data: " } + strerror(errno),
        };
    }
    chunk.resize(bytes_read);
    return chunk;
}
