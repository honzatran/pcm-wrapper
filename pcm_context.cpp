
#include "pcm_context.hpp"
#include "hw_counter_json_reader.hpp"

#include <iostream>

#include <range/v3/core.hpp>
#include <range/v3/back.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/view/chunk.hpp>
#include <range/v3/algorithm/for_each_n.hpp>
#include <range/v3/algorithm/for_each.hpp>

#include <pcm/cpucounters.h>

using namespace PcmWrapper;
using namespace std;


auto toCounterPair(HwCounter const& counter) {
    return std::make_pair(counter.getIdentifier(), counter);
}

PcmContext::~PcmContext() {
    if (m_pcm != nullptr) {
        m_pcm->cleanup();
    }
}

void
PcmContext::init(HwCounterJsonReader const& reader) {
    m_pcm = PCM::getInstance();

    auto counters = reader.getCounters(m_pcm->getCPUModel());

    m_counters = counters | ranges::view::transform(toCounterPair);

    // for (const auto& counter : counters) {
    //     m_counters[counter.getIdentifier()] = counter;
    // }

    m_chosenEvents[0] = "MEM_LOAD_UOPS_RETIRED.L1_MISS";
    m_chosenEvents[1] = "MEM_LOAD_UOPS_RETIRED.L2_MISS";
    m_chosenEvents[2] = "MEM_LOAD_UOPS_RETIRED.L3_MISS";
    m_chosenEvents[3] = "DTLB_LOAD_MISSES.WALK_COMPLETED";
}

void 
PcmContext::startMonitoring() {
    PCM::CustomCoreEventDescription description[4];

    for (size_t i = 0; i < 4; ++i) {
        HwCounter const& counter = m_counters[m_chosenEvents[i]];
        description[i] = { counter.getEventNumber(), counter.getUMaskValue() };
    }

    auto status = m_pcm->program(PCM::ProgramMode::CUSTOM_CORE_EVENTS, &description);

    if (status == PCM::Success) {
        cout << "OK" << endl;
    }
}

CoreCountersHandle
PcmContext::getCoreHandle(std::uint32_t core) const {
    return CoreCountersHandle(core, m_pcm);
}

SystemCountersHandle
PcmContext::getSystemHandle() const {
    return SystemCountersHandle(m_pcm);
} 

void 
CoreCountersHandle::onStart() {
    m_startState = m_pcm->getCoreCounterState(m_core);
}

void 
CoreCountersHandle::onEnd() {
    m_endState = m_pcm->getCoreCounterState(m_core);
}

void 
SystemCountersHandle::onStart() {
    m_startState = m_pcm->getSystemCounterState();
}

void 
SystemCountersHandle::onEnd() {
    m_endState = m_pcm->getSystemCounterState();
}

CounterRecorder::CounterRecorder(std::size_t operationCount,
                                 CounterRegister counterCount)
    : m_operationCount(operationCount),
      m_counterCount(counterCount + 1),
      m_index(0) {

    m_eventCounts = std::vector<std::uint64_t>(m_operationCount * m_counterCount);

    cout << m_eventCounts.size() << endl;
}

// std::ostream&
// operator<<(std::ostream& oss, CounterRecorder const& recorder) {
//     auto& eventCounts = recorder.m_eventCounts;
//     size_t counterCount = recorder.m_counterCount;
//
//     for (size_t i = 0; i < eventCounts.size(); i += counterCount) {
//         for (size_t counter = 0; counter < counterCount - 1; ++counter) {
//             oss << eventCounts[i + counter] << c_csvDelim;
//         }
//
//         oss << eventCounts[i + counterCount - 1] << std::endl;
//     }
//     return oss;
// }
//
void
CounterRecorder::print(std::ostream& oss) const {
    auto rng = m_eventCounts | ranges::view::chunk(m_counterCount);

    ranges::for_each(rng, [&oss, this](auto const& quintet) {
        ranges::for_each_n(quintet, m_counterCount - 1, [&oss](auto const& v) {
            oss << v << ',';
        });

        oss << ranges::back(quintet) << endl;
    });
}
