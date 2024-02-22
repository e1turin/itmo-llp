#pragma once

#include <optional>
#include <string>

#include "tcp_socket.hpp"

class Server {
 public:
  Server(const char* addr, uint16_t port);

  bool accept();

  bool send(const std::string& msg);
  std::optional<std::string> receive();

 private:
  TcpSocket socket;
};