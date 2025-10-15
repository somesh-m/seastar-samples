#include "service.hh"
#include <seastar/core/coroutine.hh>
#include <seastar/core/reactor.hh>
#include <seastar/net/api.hh>

using namespace seastar;

static logger s_logger{"service"};

seastar::future<> service::start() {
    _running = true;
    const auto sid = seastar::this_shard_id();
    const auto port = uint16_t(_port + sid);
    s_logger.info("Starting service on port {} at shard {}", port, sid);
    seastar::listen_options lo;
    lo.reuse_address = false;
seastar:
    server_socket listener =
        seastar::listen(seastar::make_ipv4_address({"0.0.0.0", port}), lo);
    try {
        while (true) {
            seastar::accept_result res = co_await listener.accept();
            auto peer = res.remote_address;
            s_logger.info("Accepted connection from remote {} at shard", peer,
                          sid);
            auto out = seastar::output_stream<char>(res.connection.output());
            // auto in = seastar::input_stream<char>(res.connection.input());
            co_await out.write("Received connection\r\n");
            co_await out.flush();
            co_await out.close();
        }
    } catch (...) {
        std::cerr << "Error occurred in accept loop: "
                  << std::current_exception() << "\n";
        co_return;
    }
    co_return;
}

seastar::future<> service::stop() {
    if (_running) {
        _running = {}; // destroy server_socket to stop accepting
        _running = false;
    }
    co_return;
}
