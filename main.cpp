#include <iostream> 
#include <vector>
#include <random>
#include <chrono>

#include <pcm/cpucounters.h>
#include <pcm/utils.h>

#include "hw_counter.hpp"
#include "hw_counter_json_reader.hpp"
#include "pcm_context.hpp"



using namespace std;

#define SIZE 1024 * 1024 * 512 
#define MAX 1024

int
main(int argc, char** argv) {
    std::mt19937 gen;
    uniform_int_distribution<int> dist(0, MAX);

    uniform_int_distribution<int> index_dist(1, SIZE);

    PcmWrapper::HwCounterJsonReader reader;

    reader.loadFromDirectory(argv[1]);

    PcmWrapper::PcmContext context;
    context.init(reader);

    context.startMonitoring();

    auto coreHandle = context.getSystemHandle();

    auto mixin =
        PcmWrapper::CounterHandleRecorder<PcmWrapper::SystemCountersHandle>(
            2, PcmWrapper::FOUR, std::move(coreHandle));

    set_signal_handlers();

    std::vector<int> values;
    values.reserve(SIZE);

    cout << "FILL START" << endl;
    for (std::size_t i = 0; i < SIZE; i++) {
        values.push_back(dist(gen));
    }
    cout << "FILL END" << endl;


    int sum = 0;

    mixin.onStart();
    for (std::size_t i = 0; i < SIZE; i++) {
        sum += values[i];
    }
    mixin.onEnd();

    vector<int> indices;
    indices.reserve(1000000);
    for (std::size_t i = 0; i < SIZE; i++) {
        indices.push_back(dist(gen));
    }

    cout  << "RANDOM" << endl;

    mixin.onStart();
    for (std::size_t i = 0; i < SIZE; i++) {
        sum += values[indices[i]];
    }
    mixin.onEnd();

    cout << sum << endl;

    cout << context.getEventHeader() << endl;
    cout << mixin << endl;

    exit_cleanup();

    return 0;
}
