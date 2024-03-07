from server import Server
from client import Client
from socket import timeout


def replace(originalChunk: bytes, original: bytes, replacer: bytes) -> bytes:
    '''
    Replaces the bytes `original` with the ones in `replacer`
    on replacing it tries updating any `Content-Length` in the same chunk
    '''
    if not originalChunk.startswith(b'HTTP/1.1 200 OK'):
        return originalChunk

    chunk: bytes = originalChunk.replace(original, replacer)
    sizeDiff: int = len(chunk) - len(originalChunk)
    if sizeDiff == 0:
        return chunk

    conLenHeader: bytes = b'Content-Length: '
    conLenStart: int = chunk.find(conLenHeader)
    if conLenStart == -1:
        return chunk
    # move to start of digit
    conLenStart += len(conLenHeader)
    conLenEnd: int = chunk.find(b'\r\n', conLenStart)
    conLen: int = int(chunk[conLenStart:conLenEnd])
    chunk = chunk.replace(
        conLenHeader + str(conLen).encode(),
        conLenHeader + str(conLen + sizeDiff).encode(),
    )
    return chunk


def getHostHeader(chunk: bytes):
    '''
    Finds the destination of an HTTP request in a chunk
    '''
    hostStart: int = chunk.find(b'GET http://')
    if hostStart == -1:
        raise ValueError("Invalid get request, or not first chunk in request")
    hostStart += 11  # Seek to host string
    hostEnd: int = chunk.find(b'/', hostStart)
    return chunk[hostStart:hostEnd]


def handleConversation(server: Server):
    '''
    Recieve a single chunk from the real client and send it to the real server
    then loop getting data from the real server, modify, and then forward it
    to the real client
    '''
    chunk: bytes = bytes()
    client = Client()
    while True:
        try:
            chunk = server.recv()
            if not client.isConnected():
                if not chunk.startswith(b'GET http://'):
                    continue
                host: bytes = getHostHeader(chunk)
                client.connect(host)
            client.send(chunk)
            while True:
                chunk = client.recv()
                chunk = replace(chunk, b'Smiley', b'Trolly')
                chunk = replace(chunk, b'smiley.jpg', b'trolly.jpg')
                chunk = replace(chunk, b' Stockholm', b' Link\xc3\xb6ping')
                server.send(chunk)
        except timeout:
            # Client disconnects in the destructor
            return


def main() -> int:
    '''
    Set up a socket for the server to recieve connections,
    then handle those connections in a loop
    '''
    server = Server(8080)
    try:
        while True:
            server.connect()
            handleConversation(server)
            server.disconnect()
    except KeyboardInterrupt:
        return 1
    return 0


if __name__ == '__main__':
    exit(main())
