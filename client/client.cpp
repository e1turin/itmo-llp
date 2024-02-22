#include "client.hpp"

bool Client::connect(const char* addr, uint16_t port) {
  return socket.connectTo(addr, port);
}

bool Client::send(const std::string& msg) {
  uint32_t len = msg.size();
  if (sizeof(len) != socket.sendBuffer(&len, sizeof(len))) {
    return false;
  }
  return (socket.sendBuffer(msg.c_str(), msg.size()) == msg.size());
}

std::optional<std::string> Client::receive() {
  uint32_t len = 0;
  if (sizeof(len) != socket.receiveBuffer(&len, sizeof(len))) {
    return std::nullopt;
  }

  std::string res;
  res.resize(len);

  if (len != socket.receiveBuffer(res.data(), len)) {
    return std::nullopt;
  }

  return res;
}