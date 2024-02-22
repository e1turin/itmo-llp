#pragma once

#include <string>

class TcpSocket {
 public:
  TcpSocket();
  ~TcpSocket();

  int64_t sendBuffer(const void* buffer, size_t size);
  int64_t receiveBuffer(void* buffer, size_t size);

  bool connectTo(const char* addr, uint16_t port);

  bool listenAt(const char* addr, uint16_t port);
  bool acceptConn();

 private:
  void* socket_fd_ = nullptr;
  void* client_fd_ = nullptr;
};