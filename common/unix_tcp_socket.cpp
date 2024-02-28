#ifdef __unix__

#include "tcp_socket.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdexcept>

TcpSocket::TcpSocket() : socket_fd_(new int(socket(AF_INET, SOCK_STREAM, 0))) {
  if (*reinterpret_cast<int*>(socket_fd_) < 0) {
    throw std::runtime_error("Unable to create socket, error: " + std::to_string(*reinterpret_cast<int*>(socket_fd_)));
  }

  const int reuseaddr = 1;
  if (setsockopt(*reinterpret_cast<int*>(socket_fd_), SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(int)) < 0) {
    throw std::runtime_error("setsockopt(SO_REUSEADDR) failed");
  }

  const int reuseport = 1;
  if (setsockopt(*reinterpret_cast<int*>(socket_fd_), SOL_SOCKET, SO_REUSEPORT, &reuseport, sizeof(int)) < 0) {
    throw std::runtime_error("setsockopt(SO_REUSEPORT) failed");
  }
}

TcpSocket::~TcpSocket() {
  delete reinterpret_cast<int*>(socket_fd_);
  delete reinterpret_cast<int*>(client_fd_);
}

sockaddr_in convertAddr(const char* addr, uint16_t port) {
  return sockaddr_in{
      .sin_family = AF_INET, .sin_port = static_cast<in_port_t>(port), .sin_addr = {.s_addr = inet_addr(addr)}};
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
  return send(*reinterpret_cast<int*>(client_fd_), buffer, size, 0);
}

int64_t TcpSocket::receiveBuffer(void* buffer, size_t size) {
  return recv(*reinterpret_cast<int*>(client_fd_), buffer, size, 0);
}
#endif /* __unix__ */