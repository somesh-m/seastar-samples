#pragma once
#include <seastar/core/sharded.hh>

class service {
  public:
    explicit service(std::uint16_t port) : _port(port) {}

    seastar::future<> start();
    seastar::future<> stop();

  private:
    bool _running{false};
    std::uint16_t _port{0} ;
};
