
#include "pcm_context.hpp"
#include "error_handling.hpp"
#include "hw_counter_json_reader.hpp"

#include <iostream>

#include <pcm/cpucounters.h>

using namespace pcm_wrapper;
using namespace std;

std::pair<std::string, HwCounter>
toCounterPair(HwCounter const& counter)
{
    return std::make_pair(counter.getIdentifier(), counter);
}

PcmContext::~PcmContext()
{
    if (m_pcm != nullptr)
    {
        m_pcm->cleanup();
    }
}

void
PcmContext::init(HwCounterJsonReader const& reader)
{
    m_pcm = PCM::getInstance();

    auto counters = reader.getCounters(m_pcm->getCPUModel());

    if (counters.empty())
    {
        FATAL_ERROR("No counters supported for this model");
    }

    for (const auto& counter : counters)
    {
        m_counters[counter.getIdentifier()] = counter;
    }

    setCounter<CounterRegister::ONE>("MEM_LOAD_RETIRED.L1_MISS");
    setCounter<CounterRegister::TWO>("MEM_LOAD_RETIRED.L2_MISS");
    setCounter<CounterRegister::THREE>("MEM_LOAD_RETIRED.L3_MISS");
    setCounter<CounterRegister::FOUR>("DTLB_LOAD_MISSES.WALK_COMPLETED");
}

void
PcmContext::startMonitoring()
{
    PCM::CustomCoreEventDescription description[4];

    for (size_t i = 0; i < 4; ++i)
    {
        HwCounter const& counter = m_counters[m_chosenEvents[i]];
        description[i] = {counter.getEventNumber(), counter.getUMaskValue()};
    }

    auto status
        = m_pcm->program(PCM::ProgramMode::CUSTOM_CORE_EVENTS, &description);

    if (status == PCM::Success)
    {
        cout << "OK" << endl;
    }
    else
    {
        handlePcmProgramError(status);
    }
}

void
PcmContext::handlePcmProgramError(PCM::ErrorCode ec)
{
    if (ec == PCM::MSRAccessDenied)
    {
        handleFatalError(
            "PcmContext::startMonitoring failed because access to PCM was "
            "denied");
    }
    else if (ec == PCM::PMUBusy)
    {
        if (m_resetBusyDevice)
        {
            m_pcm->resetPMU();

            handleFatalError(
                "PcmContext::startMontoring failed because PMU is busy, "
                "resetting device");
        }
        else
        {
            handleFatalError(
                "PcmContext::startMontoring failed because PMU is busy");
        }
    }
    else if (ec == PCM::UnknownError)
    {
        handleFatalError(
            "PcmContext::startMonitoring failed because of unknown error");
    }
}

CoreCountersHandle
PcmContext::getCoreHandle(std::uint32_t core) const
{
    return CoreCountersHandle(core, m_pcm);
}

SystemCountersHandle
PcmContext::getSystemHandle() const
{
    return SystemCountersHandle(m_pcm);
}

void
PcmContext::checkEventValidity(std::string const& eventName)
{
    auto it = m_counters.find(eventName);

    if (it == m_counters.end())
    {
        std::string errorMsg = "Event not found in passed db: " + eventName;

        FATAL_ERROR(errorMsg.c_str());
    }
}

void
CoreCountersHandle::onStart()
{
    m_startState = m_pcm->getCoreCounterState(m_core);
}

void
CoreCountersHandle::onEnd()
{
    m_endState = m_pcm->getCoreCounterState(m_core);
}

void
SystemCountersHandle::onStart()
{
    m_startState = m_pcm->getSystemCounterState();
}

void
SystemCountersHandle::onEnd()
{
    m_endState = m_pcm->getSystemCounterState();
}

CounterRecorder::CounterRecorder(
    std::size_t operationCount, CounterRegister counterCount)
    : m_operationCount(operationCount),
      m_counterCount(counterCount + 1),
      m_index(0)
{
    m_commonCounts = std::vector<std::uint64_t>(m_operationCount * 2);
    m_eventCounts
        = std::vector<std::uint64_t>(m_operationCount * m_counterCount);
}

void
CounterRecorder::print(std::ostream& oss) const
{
    for (size_t i = 0; i < m_index; i++)
    {
        printCommonCounters(oss, i);

        oss << c_csvDelim;

        printEventCounters(oss, i);

        oss << endl;
    }
}

void
CounterRecorder::printCommonCounters(
    std::ostream& oss, std::size_t const measurementIndx) const
{
    std::size_t indx = measurementIndx * c_commonCounterCount;

    oss << m_commonCounts[indx] << c_csvDelim << m_commonCounts[indx + 1];
}

void
CounterRecorder::printEventCounters(
    std::ostream& oss, std::size_t const measurementIndx) const
{
    std::size_t indx = measurementIndx * m_counterCount;

    for (size_t counter = 0; counter < m_counterCount - 1; ++counter)
    {
        oss << m_eventCounts[indx + counter] << c_csvDelim;
    }

    oss << m_eventCounts[indx + m_counterCount - 1];
}

void
__cpuid()
{
    unsigned a, b, c, d;

    __asm__ __volatile__("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d));
}
