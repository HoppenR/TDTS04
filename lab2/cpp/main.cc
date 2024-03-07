#include "SocketTimeoutException.h"
#include "client.h"
#include "server.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <vector>

/*
 * Replaces the bytes `original` with the ones in `replacer`
 * on replacing it tries updating any `Content-Length` in the same chunk
 */
void replace(std::vector<uint8_t>& chunk,
             std::string const& original,
             std::string const& replacer) {
    // Return early if this chunk does not begin with "HTTP/1.1 200 OK"
    std::string resp_startstr{ "HTTP/1.1 200 OK" };
    std::vector<uint8_t>::iterator resp_start = std::search(
        chunk.begin(), chunk.end(), resp_startstr.begin(), resp_startstr.end());
    if (resp_start == chunk.end()) {
        return;
    }

    // Find the end of the header section to know where we can start modifying
    // normal data with search/replace
    std::string hdr_mark = "\r\n\r\n";
    std::vector<uint8_t>::iterator it = std::search(
        chunk.begin(), chunk.end(), hdr_mark.begin(), hdr_mark.end());
    if (it == chunk.end()) {
        return;
    }
    // Iterate from the start of body section and look for the next instance of
    // the `original` string in the `chunk` data and replace each instance with
    // `replacer`
    size_t replacements = 0;
    while (true) {
        it = std::search(it, chunk.end(), original.begin(), original.end());
        if (it == chunk.end()) {
            break;
        }
        it = chunk.erase(it, it + original.size());
        it = chunk.insert(it, replacer.begin(), replacer.end());
        replacements++;
    }
    if (replacements == 0) {
        // No need to modify Content-Length header, return early
        return;
    }
    size_t size_diff = replacements * (replacer.size() - original.size());

    // Find Content-Length in the chunk
    std::string conlen_hdr = "Content-Length: ";
    std::vector<uint8_t>::iterator conlen_start = std::search(
        chunk.begin(), chunk.end(), conlen_hdr.begin(), conlen_hdr.end());
    if (conlen_start == chunk.end()) {
        return;
    }
    conlen_start += conlen_hdr.length(); // Seek to content length digits

    // Find the end of Content-Length header
    std::string end_of_header = "\r\n";
    std::vector<uint8_t>::iterator conlen_end = std::search(
        conlen_start, chunk.end(), end_of_header.begin(), end_of_header.end());

    // Extract Content-Length digit
    std::string conlen_str = std::string{ conlen_start, conlen_end };
    size_t conlen = std::stoul(conlen_str);
    conlen_str = std::to_string(conlen + size_diff);

    // Replace old content length with new
    conlen_start = chunk.erase(conlen_start, conlen_end);
    chunk.insert(conlen_start, conlen_str.begin(), conlen_str.end());
}

/*
 * Finds the destination of an HTTP request in a chunk
 */
std::string get_host(std::vector<uint8_t> const& chunk) {
    std::string request_start{ "GET http://" };
    std::vector<uint8_t>::const_iterator host_start = std::search(
        chunk.begin(), chunk.end(), request_start.begin(), request_start.end());
    if (host_start == chunk.end()) {
        throw std::runtime_error{
            "Invalid get request, or not first chunk in request",
        };
    }

    // Seek to host string
    host_start += request_start.length();
    std::vector<uint8_t>::const_iterator host_end =
        std::find(host_start, chunk.end(), '/');
    return { host_start, host_end };
}

/*
 * Return whether a chunk begins with the bytes in `prefix`
 */
bool starts_with(std::vector<uint8_t> const& chunk, std::string const& prefix) {
    return std::equal(prefix.begin(), prefix.end(), chunk.begin());
}

/*
 * Recieve a single chunk from the real client and send it to the real server.
 * Then loop getting data from the real server, modify, and then forward it
 * to the real client. This repeats this until a socket times out.
 */
void handle_conversation(Server const& server) {
    std::vector<uint8_t> chunk;
    Client client;
    while (true) {
        try {
            chunk = server.recv();
            if (!starts_with(chunk, "GET http://")) {
                // Ignore request
                continue;
            }
            std::string host = get_host(chunk);
            client.connect(host);
            client.send(chunk);
            while (true) {
                chunk = client.recv();
                replace(chunk, "Smiley", "Trolly");
                replace(chunk, "smiley.jpg", "trolly.jpg");
                replace(chunk, " Stockholm", " Linköping");
                server.send(chunk);
            }
        } catch (SocketTimeoutException&) {
            // Client disconnects in the destructor
            return;
        }
    }
}

/*
 * Set up a socket for the server to recieve connections,
 * then handle those connections in a loop
 */
int main() {
    Server server{ 8080 };
    while (true) {
        server.connect();
        handle_conversation(server);
        server.disconnect();
    }
    return EXIT_SUCCESS;
}
