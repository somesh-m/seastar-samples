#include <chrono>
#include <seastar/core/app-template.hh>
#include <seastar/core/memory.hh>
#include <seastar/core/sleep.hh>

using namespace std::chrono_literals;

seastar::future<> init(); // Explain briefly what future does
seastar::future<> run();
seastar::future<> run_others();
seastar::future<> run_on_particular();
seastar::future<> print_stats();

int local_counter = 0;

int main(int argc, char **argv) {
    seastar::app_template app;
    try {
        app.run(argc, argv, run_on_particular);
    } catch (...) {
        std::cerr << "Couldn't start application: " << std::current_exception();
        return 1;
    }

    return 0;
}

// This is a simple hello world function
seastar::future<> init() {
    std::cout << "Hello from seastar" << std::endl;
    return seastar::make_ready_future<>(); // Resolves the future immediately
}

seastar::future<> run() {
    return seastar::smp::invoke_on_all([] {
        std::cout << "\n Shard " << seastar::this_shard_id()
                  << " Says hi ... \n"
                  << std::flush;
    });
}

seastar::future<> run_others() {
    return seastar::smp::invoke_on_others(2, [] {
        std::cout << "\nHi from shard " << seastar::this_shard_id()
                  << std::flush;
    });
}

seastar::future<> run_on_particular() {
    return seastar::smp::submit_to(5, [] {
        std::cout << "\n Hi from shard " << seastar::this_shard_id()
                  << std::flush;
    });
}
