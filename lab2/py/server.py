from socket import (
    socket, AF_INET, SOCK_STREAM,
    SOL_SOCKET, SO_REUSEADDR, SO_REUSEPORT,
    error as SocketError
)


class Server():
    def __init__(self, port: int):
        '''
        Associates socket with port at localhost and starts listening
        for connections
        '''
        self._port: int = port
        self._socket: socket = socket(AF_INET, SOCK_STREAM)
        self._clientSocket: socket
        if self._socket == -1:
            raise SocketError('can not create socket on port %d' % self._port)
        self._socket.setsockopt(SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 1)
        self._socket.bind(('', self._port))
        self._socket.listen(5)

    def __del__(self):
        self._clientSocket.close()
        self._socket.close()

    def disconnect(self):
        self._clientSocket.close()

    def connect(self):
        '''
        connect to the client
        '''
        self._clientSocket, _ = self._socket.accept()
        self._clientSocket.settimeout(0.5)

    def recv(self) -> bytes:
        return self._clientSocket.recv(8192)

    def send(self, chunk: bytes):
        self._clientSocket.send(chunk)
