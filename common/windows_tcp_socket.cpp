#ifdef _WIN32

#include "tcp_socket.hpp"

#include <stdexcept>

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

TcpSocket::TcpSocket() {
  WSADATA wsaData;

  int iResult;

  // Initialize Winsock
  iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (iResult != 0) {
    throw std::runtime_error("WSAStartup failed\n");
  }

  SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (sock == INVALID_SOCKET) {
    WSACleanup();
    throw std::runtime_error("Unable to create socket");
  }

  socket_fd_ = new SOCKET(sock);
}

TcpSocket::~TcpSocket() {
  delete reinterpret_cast<SOCKET*>(socket_fd_);
  delete reinterpret_cast<SOCKET*>(client_fd_);

  WSACleanup();
}

sockaddr_in convertAddr(const char* addr, uint16_t port) {
  IN_ADDR in_addr = {};
  inet_pton(AF_INET, addr, &in_addr);

  return sockaddr_in{.sin_family = AF_INET, .sin_port = static_cast<u_short>(port), .sin_addr = in_addr};
}

bool TcpSocket::listenAt(const char* addr, uint16_t port) {
  sockaddr_in host = convertAddr(addr, port);

  if (bind(*reinterpret_cast<int*>(socket_fd_), reinterpret_cast<sockaddr*>(&host), sizeof(host)) < 0) {
    return false;
  }

  return (listen(*reinterpret_cast<int*>(socket_fd_), 1) == 0);
}

bool TcpSocket::acceptConn() {
  client_fd_ = new int(accept(*reinterpret_cast<int*>(socket_fd_), nullptr, nullptr));
  return (*reinterpret_cast<int*>(client_fd_) > 0);
}

bool TcpSocket::connectTo(const char* addr, uint16_t port) {
  sockaddr_in dst = convertAddr(addr, port);

  auto res = (connect(*reinterpret_cast<int*>(socket_fd_), reinterpret_cast<sockaddr*>(&dst), sizeof(dst)) == 0);

  client_fd_ = socket_fd_;
  socket_fd_ = nullptr;

  return res;
}

int64_t TcpSocket::sendBuffer(const void* buffer, size_t size) {
  return send(*reinterpret_cast<int*>(client_fd_), reinterpret_cast<const char*>(buffer), size, 0);
}

int64_t TcpSocket::receiveBuffer(void* buffer, size_t size) {
  return recv(*reinterpret_cast<int*>(client_fd_), reinterpret_cast<char*>(buffer), size, 0);
}

#endif /* _WIN32 */