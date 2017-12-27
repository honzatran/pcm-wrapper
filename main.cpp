#include <chrono>
#include <iostream>
#include <random>
#include <vector>

#include <pcm/cpucounters.h>
#include <pcm/utils.h>

#include "hw_counter.hpp"
#include "hw_counter_json_reader.hpp"
#include "pcm_context.hpp"

#include <mutex>
#include <thread>

using namespace std;

#define SIZE 1024 * 1024 * 512
#define MAX 1024

int
main(int argc, char **argv)
{
    std::mt19937 gen;
    uniform_int_distribution<int> dist(1, MAX);

    uniform_int_distribution<int> index_dist(1, SIZE);

    ASSERT(argc == 2, "AA");

    PcmWrapper::HwCounterJsonReader reader;

    reader.loadFromDirectory(argv[1]);

    PcmWrapper::PcmContext context;
    context.init(reader);
    context.resetMsrIfBusy();

    context.startMonitoring();

    auto handle = context.getCoreHandle(2);

    auto mixin
        = PcmWrapper::CounterHandleRecorder<std::decay<decltype(handle)>::type>(
            2, PcmWrapper::FOUR, std::move(handle));

    set_signal_handlers();

    std::vector<int> values;
    values.reserve(SIZE);

    cout << "FILL START" << endl;
    for (std::size_t i = 0; i < SIZE; i++)
    {
        values.push_back(dist(gen));
    }
    cout << "FILL END" << endl;

    int sum = 0;

    mixin.onStart();
    for (std::size_t i = 0; i < 100; i++)
    {
        sum += values[i];
    }
    mixin.onEnd();

    vector<int> indices;
    indices.reserve(1000000);
    for (std::size_t i = 0; i < SIZE; i++)
    {
        indices.push_back(dist(gen));
    }

    cout << "RANDOM" << endl;

    mixin.onStart();
    for (std::size_t i = 0; i < 100; i++)
    {
        sum += values[indices[i]];
    }
    mixin.onEnd();

    cout << context.getEventHeader() << endl;
    cout << mixin << endl;

    exit_cleanup();

    return 0;
}
