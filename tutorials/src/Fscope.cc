#include <coroutine>
#include <seastar/core/app-template.hh>
#include <seastar/core/future.hh>
#include <seastar/core/sleep.hh>
#include <seastar/core/with_timeout.hh>

#include <seastar/core/timer.hh>

using namespace seastar;
using namespace std;
using namespace std::chrono_literals;
#include <chrono>

future<> init();
future<> keepRepeating();
future<> startAsyncLoop();
future<> checkStatus();
future<> repeatTillTimeout();
future<> delayedResult();

int main(int argc, char **argv) {
    app_template app;
    try {
        app.run(argc, argv, repeatTillTimeout);
    } catch (...) {
        cerr << "Couldn't start application: " << current_exception();
        return 1;
    }

    return 0;
}

future<> init() {
    return do_with(promise<int>{}, 5, [](promise<int> &p, int &testVar) {
        auto f = p.get_future().then([&testVar](int value) {
            cout << "Promise fulfilled" << value << endl;
            cout << "Test var = " << testVar << endl;
        });
        testVar++;
        p.set_value(30);

        return f;
    });
}

future<> startAsyncLoop() {
    keepRepeating();
    checkStatus();
    return make_ready_future<>();
}

future<> checkStatus() {
    return do_with(0, [](int &counter) {
        return seastar::repeat([&counter] {
            if (counter != 5) {
                cout << " Not available" << "\n";
            } else {
                cout << " Available" << "\n";
                return make_ready_future<stop_iteration>(stop_iteration::yes);
            }
            ++counter;
            sleep(std::chrono::milliseconds(0));
            return make_ready_future<stop_iteration>(stop_iteration::no);
        });
    });
}

future<> keepRepeating() {
    return do_with(0, [](int &i) {
        // It keeps repeating untill stop_iteration yes is met
        // This is async looping, so it doesn't block and keeps yeilding in
        // between
        return seastar::repeat([&i] {
            cout << "i = " << i << "\n";

            if (++i == 5) {
                return make_ready_future<stop_iteration>(stop_iteration::yes);
            }
            // yield here to demo interleaving
            // yield point (timer-based)
            return sleep(std::chrono::milliseconds(0)).then([] {
                return stop_iteration::no;
            });
        });
    });
}

future<> repeatTillTimeout() {
    auto deadline = std::chrono::steady_clock::now() + 5s;
    return seastar::with_timeout(deadline, delayedResult())
        .then([] { cout << "Task completed\n"; })
        .handle_exception([](std::exception_ptr e) {
            try {
                std::rethrow_exception(e);
            } catch (const timed_out_error &t) {
                cout << "System timed out before any result\n";
            }
        });
}

future<> delayedResult() {
    std::cout << "Started async work...\n";
    return seastar::sleep(10s).then(
        [] { std::cout << "Future fulfilled after 10 seconds\n"; });
}
