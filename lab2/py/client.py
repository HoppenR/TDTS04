from socket import (socket, getaddrinfo, AF_INET, SOCK_STREAM)


class Client():
    def __init__(self):
        self._socket: socket = socket(AF_INET, SOCK_STREAM)
        self._isConnected: bool = False

    def __del__(self):
        self._socket.close()

    def connect(self, host: bytes):
        '''
        connect to the host
        '''
        addressInfoList: list[tuple] = getaddrinfo(host, "http")
        # Gets the first addressInfo (0) the (address, port) (4)
        self._socket.connect(addressInfoList[0][4])
        self._socket.settimeout(0.5)
        self._isConnected = True

    def isConnected(self) -> bool:
        return self._isConnected

    def send(self, chunk: bytes):
        self._socket.send(chunk)

    def recv(self) -> bytes:
        return self._socket.recv(8192)
