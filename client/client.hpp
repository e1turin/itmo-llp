#pragma once

#include <optional>
#include <string>

#include "tcp_socket.hpp"

class Client {
 public:
  bool connect(const char* addr, uint16_t port);

  bool send(const std::string& msg);
  std::optional<std::string> receive();

 private:
  TcpSocket socket;
};