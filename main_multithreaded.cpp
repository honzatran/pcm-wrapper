#include <chrono>
#include <random>
#include <thread>
#include <vector>

#include <pcm/cpucounters.h>
#include <pcm/utils.h>

#include "hw_counter_json_reader.hpp"
#include "hw_event.hpp"
#include "pcm_context.hpp"
#include "rdmpc_counter_handle.hpp"

#include <mutex>

#ifdef __linux__

#include <sched.h>

#endif

std::mutex mtx;

using namespace std;
using namespace pcm_wrapper;

#define SIZE 1024 * 1024 * 512
#define MAX 1024

class AffinityGuard
{
public:
#ifdef __linux__
    cpu_set_t oldstate;

    explicit AffinityGuard(std::uint32_t core)
    {
        pthread_getaffinity_np(pthread_self(), sizeof(cpu_set_t), &oldstate);

        cpu_set_t newstate;
        CPU_ZERO(&newstate);
        CPU_SET(core, &newstate);

        pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &newstate);
    }

    ~AffinityGuard()
    {
        pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &oldstate);
    }
#else

    explicit AffinityGuard(std::uint32_t core) {}
#endif
};

void
work(PcmContext const& context, int core)
{
    std::mt19937 gen;
    uniform_int_distribution<int> dist(0, MAX);

    uniform_int_distribution<int> index_dist(1, SIZE);

    AffinityGuard affinityGuard(core);

    auto handle = context.getCoreHandle(core);

    auto mixin = pcm_wrapper::CounterHandleRecorder<std::decay<decltype(
        handle)>::type>(1000, pcm_wrapper::FOUR, std::move(handle));

    for (int i = 0; i < 1000; ++i)
    {
        mixin.onStart();
        mixin.onEnd();
    }

    // std::vector<int> values;
    // values.reserve(SIZE);
    //
    // for (std::size_t i = 0; i < SIZE; i++) {
    //     values.push_back(dist(gen));
    // }
    //
    // int sum = 0;
    //
    // mixin.onStart();
    // for (std::size_t i = 0; i < 100; i++) {
    //     sum += values[i];
    // }
    // mixin.onEnd();

    // vector<int> indices;
    // indices.reserve(1000000);
    // for (std::size_t i = 0; i < SIZE; i++) {
    //     indices.push_back(dist(gen));
    // }

    // mixin.onStart();
    // for (std::size_t i = 0; i < 100; i++) {
    //     sum += values[indices[i]];
    // }
    // mixin.onEnd();

    {
        std::lock_guard<std::mutex> lock_guard(mutex);

        cout << "core: " << core << endl;
        cout << context.getEventHeader() << endl;
        cout << mixin << endl;
    }
}

int
main(int argc, char** argv)
{
    pcm_wrapper::HwCounterJsonReader reader;

    reader.loadFromDirectory(argv[1]);

    pcm_wrapper::PcmContext context;
    context.init(reader);
    context.resetMsrIfBusy();

    context.startMonitoring();

    set_signal_handlers();

#if PCM_USE_PERF
    cout << "USING PERF" << endl;
#endif

    std::vector<std::thread> workers;

    for (size_t i = 4; i < 9; ++i)
    {
        workers.push_back(std::thread([i, &context] { work(context, i); }));
    }

    for (auto& t : workers)
    {
        t.join();
    }

    exit_cleanup();

    return 0;
}
