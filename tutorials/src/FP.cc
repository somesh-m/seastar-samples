#include <seastar/core/app-template.hh>

seastar::future<> init();
int main(int argc, char **argv) {
    seastar::app_template app;
    try {
        app.run(argc, argv, init);
    } catch (...) {
        std::cerr << "Failed to start application: " << std::current_exception()
                  << "\n";
        return 1;
    }
    return 0;
}

seastar::future<> init() {
    seastar::promise<int> p;
    auto f = p.get_future();
    f.then([](int value) {
        // this is a different context
        std::cout << "Promise resolved " << value << std::endl;
    });
    std::cout << " Waiting for the result" << std::endl;
    p.set_value(42);
    return seastar::make_ready_future<>();
}
