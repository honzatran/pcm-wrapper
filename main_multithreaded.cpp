#include <vector>
#include <random>
#include <chrono>
#include <thread>

#include <pcm/cpucounters.h>
#include <pcm/utils.h>

#include "hw_counter.hpp"
#include "hw_counter_json_reader.hpp"
#include "pcm_context.hpp"

#include <mutex>

#ifdef __linux__

#include <sched.h>

#endif

std::mutex mtx;


using namespace std;
using namespace PcmWrapper;

#define SIZE 1024 * 1024 * 512 
#define MAX 1024

class AffinityGuard
{
public:
#ifdef __linux__
    cpu_set_t oldstate;

    explicit AffinityGuard(std::uint32_t core) {
        pthread_getaffinity_np(
            std::this_thread::get_id(), sizeof(cpu_set_t), &oldstate);

        cpu_set_t newstate;
        CPU_ZERO(&newstate);
        CPU_SET(core, &newstate);

        pthread_setaffinity_np(
            std::this_thread::get_id(), sizeof(cpu_set_t), &newstate);
    }

    ~AffinityGuard() {
        pthread_setaffinity_np(
            std::this_thread::get_id(), sizeof(cpu_set_t), &oldstate);
    }
#else

    explicit AffinityGuard(std::uint32_t core) {}
#endif
};

void
work(PcmContext const& context, int core) {
    std::mt19937 gen;
    uniform_int_distribution<int> dist(0, MAX);

    uniform_int_distribution<int> index_dist(1, SIZE);

    AffinityGuard affinityGuard(core);

    auto handle = context.getCoreHandle(core);

    auto mixin =
        PcmWrapper::CounterHandleRecorder<std::decay<decltype(handle)>::type>(
            2, PcmWrapper::FOUR, std::move(handle));

    std::vector<int> values;
    values.reserve(SIZE);

    for (std::size_t i = 0; i < SIZE; i++) {
        values.push_back(dist(gen));
    }

    int sum = 0;

    mixin.onStart();
    for (std::size_t i = 0; i < 100; i++) {
        sum += values[i];
    }
    mixin.onEnd();

    vector<int> indices;
    indices.reserve(1000000);
    for (std::size_t i = 0; i < SIZE; i++) {
        indices.push_back(dist(gen));
    }

    mixin.onStart();
    for (std::size_t i = 0; i < 100; i++) {
        sum += values[indices[i]];
    }
    mixin.onEnd();

    {
        std::lock_guard<std::mutex> lock_guard(mutex);

        cout << "core: " << core << endl;
        cout << context.getEventHeader() << endl;
        cout << mixin << endl;
    }
}

int
main(int argc, char** argv) {

    PcmWrapper::HwCounterJsonReader reader;

    reader.loadFromDirectory(argv[1]);

    PcmWrapper::PcmContext context;
    context.init(reader);
    context.resetMsrIfBusy();

    context.startMonitoring();

    set_signal_handlers();

    std::vector<std::thread> workers;

    for (size_t i = 0; i < 4; ++i) {
        workers.push_back(std::thread([i, &context] { work(context, i); }));
    }

    for (auto & t : workers) {
        t.join();
    }

    exit_cleanup();

    return 0;
}
