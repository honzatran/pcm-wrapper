#ifndef PCM_CONTEXT_HPP 
#define PCM_CONTEXT_HPP

#include "hw_counter_json_reader.hpp"
#include "hw_counter.hpp"
#include "error_handling.hpp"

#include <array>
#include <unordered_map>
#include <vector>
#include <cstdint>

#include <pcm/cpucounters.h>

constexpr char c_csvDelim = ',';
constexpr char c_commonCounterHeaders[] = "INSTRUCTIONS,CYCLES";

namespace PcmWrapper {

enum CounterRegister : int { ONE = 0, TWO = 1, THREE = 2, FOUR = 3 };

namespace detail {

template <typename STATE>
class CounterState
{
public:
    template <CounterRegister order>
    std::uint64_t getEventCounts() const {
        return getNumberOfCustomEvents(order, m_startState, m_endState);
    }

    std::uint64_t getExecutedInstructions() const {
        return getInstructionsRetired(m_startState, m_endState);
    }

    std::uint64_t getExecutedCycles() const {
        return getCycles(m_startState, m_endState);
    }

protected:
    CounterState(PCM* pcm) : m_pcm(pcm) {}
    STATE m_startState;
    STATE m_endState;

    PCM* m_pcm;
};
}

class RDPMCCountersHandle;
class CoreCountersHandle;
class SystemCountersHandle;

struct EventHeader
{ std::array<std::string, 4> chosenEvents; };

inline std::ostream&
operator<<(std::ostream& oss, EventHeader const& eventHeader) {
    auto &eventsName = eventHeader.chosenEvents;

    oss << c_commonCounterHeaders << c_csvDelim;

    return oss << eventsName[0] << c_csvDelim << eventsName[1] << c_csvDelim
               << eventsName[2] << c_csvDelim << eventsName[3];
}


class PcmContext
{
public:
    ~PcmContext();

    void init(HwCounterJsonReader const& reader);

    void startMonitoring();

    void onBenchmarkStart();
    void onBenchmarkEnd();

    void resetMsrIfBusy() {
        m_resetBusyDevice = true;
    }

    template <CounterRegister order>
    void setCounter(std::string const& eventName) {
        checkEventValidity(eventName);
        m_chosenEvents[order] = eventName;
    }

    template <CounterRegister order>
    const std::string& getEventName() const {
        return m_chosenEvents[order];
    }

    CoreCountersHandle getCoreHandle(std::uint32_t core) const;
    SystemCountersHandle getSystemHandle() const;

    EventHeader getEventHeader() const {
        return { m_chosenEvents };
    }

private:
    PCM* m_pcm;
    std::unordered_map<std::string, HwCounter> m_counters;
    std::array<std::string, 4> m_chosenEvents;
    bool m_resetBusyDevice = false;

    const HwCounter& getHwCounter(std::string const& eventName) const;

    void handlePcmProgramError(PCM::ErrorCode ec);

    void checkEventValidity(std::string const& eventName);
};

class CoreCountersHandle : public detail::CounterState<CoreCounterState>
{
public:
    void onStart();
    void onEnd();
private:
    std::uint32_t m_core;

    CoreCountersHandle(std::uint32_t core, PCM* pcm)
        : detail::CounterState<CoreCounterState>(pcm), m_core(core) {}

    friend PcmContext;
};

class SystemCountersHandle : public detail::CounterState<SystemCounterState> 
{
public:
    void onStart();
    void onEnd();
private:
    SystemCountersHandle(PCM *pcm) : detail::CounterState<SystemCounterState>(pcm) {};

    friend PcmContext;
};

class CounterRecorder
{
public:
    CounterRecorder(std::size_t operationCount, CounterRegister counterCount);

    template <typename COUNTER_STATE>
    void recordCounter(COUNTER_STATE const& state) {
        storeCounterDifference<COUNTER_STATE, CounterRegister::ONE>(state);
        storeCounterDifference<COUNTER_STATE, CounterRegister::TWO>(state);
        storeCounterDifference<COUNTER_STATE, CounterRegister::THREE>(state);
        storeCounterDifference<COUNTER_STATE, CounterRegister::FOUR>(state);

        storeCommonCounters(state);

        m_index += 1;
    }

    friend std::ostream& operator<<(std::ostream& oss,
                                    CounterRecorder const& recorder);

    void reset() { m_index = 0; }
private:
    static constexpr std::size_t c_commonCounterCount = 2;


    std::size_t const m_operationCount;
    std::size_t const m_counterCount;

    // user common counters, instruction and cycles
    std::vector<std::uint64_t> m_commonCounts;
    // user defined counters
    std::vector<std::uint64_t> m_eventCounts;


    std::size_t m_index;

    template <typename COUNTER_STATE, CounterRegister COUNTER_REGISTER>
    void storeCounterDifference(COUNTER_STATE const& state) {
        if (COUNTER_REGISTER < m_counterCount) {
            std::size_t const indx = m_index * m_counterCount;

            if (indx + COUNTER_REGISTER < m_eventCounts.size()) {
                m_eventCounts[indx + COUNTER_REGISTER] =
                    state.template getEventCounts<COUNTER_REGISTER>();
            } else {
                FATAL_ERROR(
                    "Storing event count outside the eventCounts structure");
            }
        } 
    }

    template <typename COUNTER_STATE>
    void storeCommonCounters(COUNTER_STATE const& state) {
        std::size_t const indx = m_index * c_commonCounterCount;

        if (m_index + 1 < m_commonCounts.size()) {
            m_commonCounts[indx]     = state.getExecutedInstructions();
            m_commonCounts[indx + 1] = state.getExecutedCycles();
        } else {
            FATAL_ERROR("Storing common event outside commonCounts structure");
        }
    }

    void print(std::ostream& oss) const;

    void printCommonCounters(std::ostream& oss, std::size_t const measurementIndx) const;

    void printEventCounters(std::ostream& oss, std::size_t const measurementIndx) const;
};

inline std::ostream&
operator<<(std::ostream& oss, CounterRecorder const& recorder) {
    recorder.print(oss);
    return oss;
}

class RDPMCCountersHandle
{
public:
    void onStart();
    void onEnd();

    template <CounterRegister order>
    std::uint64_t getEventCounts() const {
        return m_endState.p[order] - m_startState.p[order];
    }

    std::uint64_t getExecutedInstructions() const {
        return m_endState.instructions - m_startState.instructions;
    }

    std::uint64_t getExecutedCycles() const {
        return m_endState.cycles - m_startState.cycles;
    }
private:
    struct CounterState
    {
        std::uint64_t instructions;
        std::uint64_t cycles;

        std::uint64_t p[4];
    };

    CounterState m_startState, m_endState;
};

template <typename T>
class CounterHandleRecorder : public T, public CounterRecorder
{
public:
    template <typename... ARGS>
    CounterHandleRecorder(std::size_t operationCount,
                          CounterRegister counterCount,
                          ARGS&&... args)
        : T(std::forward<ARGS>(args)...),
          CounterRecorder(operationCount, counterCount) {}

    void onEnd() {
        T::onEnd();
        recordCounter(*this);
    }
};


}

#endif
