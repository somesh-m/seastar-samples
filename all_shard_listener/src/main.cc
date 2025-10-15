#include "service.hh"
#include <chrono>
#include <seastar/core/abort_source.hh>
#include <seastar/core/app-template.hh>
#include <seastar/core/coroutine.hh>
#include <seastar/core/reactor.hh>
#include <seastar/core/sharded.hh>
#include <seastar/core/sleep.hh>
#include <seastar/net/api.hh>

namespace ss = seastar;
using namespace seastar;

static ss::logger m_logger{"main"};

ss::future<> init();

int main(int argc, char **argv) {
    ss::app_template app;
    try {
        app.run(argc, argv, init);
    } catch (...) {
        std::cerr << "Couldn't start application: " << std::current_exception()
                  << "\n";
        return 1;
    }

    return 0;
}

seastar::future<> accept_forever(seastar::server_socket listener, uint16_t port,
                                 seastar::abort_source &as) {
    try {
        while (!as.abort_requested()) {
            auto ar = co_await listener.accept();
            m_logger.info("Accepted on port {} -> shard {} from {}", port,
                          seastar::this_shard_id(), ar.remote_address);
            // ... handle ar.connection ...
        }
    } catch (const seastar::abort_requested_exception &) {
        m_logger.info("Accept loop on port {} aborted", port);
    } catch (const std::system_error &se) {
        m_logger.error("accept() failed on port {}: {} ({})", port,
                       se.code().message(), se.code().value());
    }
    co_return;
}

seastar::future<> init() {
    // This logic should run only on shard 0
    if (ss::this_shard_id() != 0) {
        co_return;
    }

    const uint16_t base_port = 50110;
    const unsigned shard_count = ss::smp::count;
    static seastar::abort_source as;
    static seastar::gate g;

    m_logger.info("Starting listener on {} shards", shard_count);

    // Create a sharded copy of the service we want to run on each logical core
    static ss::sharded<service> service;

    /**
     * Listen to all the ports in shard 0.
     * This is needed because by default all connection is sent to shard 0
     * Shard 0 based on the load balancing algorithm distributes the connection
     *to other shards. In this case we are using fixed algorithm, so we
     *basically are binding connections to a particular port.
     **/
    for (unsigned cpu = 0; cpu < shard_count; ++cpu) {
        m_logger.info("Making main thread listen to all sockets");
        const uint16_t port = base_port + cpu;
        seastar::listen_options lo;
        lo.reuse_address = true;
        lo.lba = server_socket::load_balancing_algorithm::fixed;
        lo.fixed_cpu = cpu;

        auto listener =
            seastar::listen(make_ipv4_address({"0.0.0.0", port}), lo);
        // Run each accept loop as its own task; gate gives you a clean join.
        (void)seastar::with_gate(g, [l = std::move(listener), port]() mutable {
            return accept_forever(std::move(l), port, as);
        });
        // while (true) {
        //     try {
        //         auto result = co_await server_socket.accept();
        //         m_logger.info("Accepted on port {}", result.remote_address);
        //     } catch (...) {
        //         std::cerr << "Error while starting listeners on shard 0: "
        //                   << std::current_exception() << "\n";
        //     }
        // }
        // (void)seastar::keep_doing(
        //     [server_socket = std::move(server_socket), port]() mutable {
        //         return server_socket.accept()
        //             .then([port](seastar::accept_result ar) {
        //                 auto sid = this_shard_id(); // destination shard id
        //                 std::cout << "Accepted on port " << port << " ->shard
        //                 "
        //                           << sid << "\n";
        //                 auto peer = ar.remote_address;
        //                 std::cout << "Remote address: " << peer << "\n";
        //                 // return store.local().handle_session(
        //                 //     std::move(ar.connection), peer, port);
        //             })
        //             .handle_exception([](std::exception_ptr e) {
        //                 std::cout << "Accept failed: " << e << "\n";
        //             });
        //     });
    }

    m_logger.info("Starting service");
    co_await service.start(base_port);
    co_await service.invoke_on_others(&service::start);
    m_logger.info("Service started");

    // co_await seastar::sleep(std::chrono::hours(24));
}
