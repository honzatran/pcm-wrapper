
#include "pcm_context.hpp"
#include "hw_counter_json_reader.hpp"

#include <iostream>

#include <pcm/cpucounters.h>

using namespace PcmWrapper;
using namespace std;


std::pair<std::string, HwCounter>
toCounterPair(HwCounter const& counter) {
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

    for (const auto& counter : counters) {
        m_counters[counter.getIdentifier()] = counter;
    }

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

void
CounterRecorder::print(std::ostream& oss) const {
    for (size_t i = 0; i < m_eventCounts.size(); i += m_counterCount) {

        for (size_t counter = 0; counter < m_counterCount - 1; ++counter) {
            oss << m_eventCounts[i + counter] << c_csvDelim;
        }

        oss << m_eventCounts[i + m_counterCount - 1] << std::endl;
    }
}
