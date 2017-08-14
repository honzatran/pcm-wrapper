#ifndef PCM_CONTEXT_HPP 
#define PCM_CONTEXT_HPP

#include "hw_counter_json_reader.hpp"
#include "hw_counter.hpp"

#include <array>
#include <unordered_map>
#include <vector>
#include <cstdint>

#include <pcm/cpucounters.h>

constexpr char c_csvDelim = ',';


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
protected:
    CounterState(PCM* pcm) : m_pcm(pcm) {}
    STATE m_startState;
    STATE m_endState;

    PCM* m_pcm;
};
}

class CoreCountersHandle;
class SystemCountersHandle;

struct EventHeader
{ std::array<std::string, 4> chosenEvents; };

inline std::ostream&
operator<<(std::ostream& oss, EventHeader const& eventHeader) {
    auto &eventsName = eventHeader.chosenEvents;

    return oss << eventsName[0] << "," << eventsName[1] << "," << eventsName[2]
               << "," << eventsName[3];
}


class PcmContext
{
public:
    ~PcmContext();

    void init(HwCounterJsonReader const& reader);

    void startMonitoring();

    void onBenchmarkStart();
    void onBenchmarkEnd();

    template <CounterRegister order>
    void setCounter(std::string const& eventName) {
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

    const HwCounter& getHwCounter(std::string const& eventName) const;
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

        m_index += m_counterCount;

        std::cout << m_index << std::endl;
    }

    friend std::ostream& operator<<(std::ostream& oss,
                                    CounterRecorder const& recorder);

    void reset() { m_index = 0; }
private:
    std::size_t const m_operationCount;
    std::size_t const m_counterCount;
    std::vector<std::uint64_t> m_eventCounts;

    std::size_t m_index;

    template <typename COUNTER_STATE, CounterRegister COUNTER_REGISTER>
    void storeCounterDifference(COUNTER_STATE const& state) {
        if (COUNTER_REGISTER < m_counterCount) {
            m_eventCounts[m_index + COUNTER_REGISTER] =
                state.template getEventCounts<COUNTER_REGISTER>();
        }
    }

    void print(std::ostream& oss) const;
};

inline std::ostream&
operator<<(std::ostream& oss, CounterRecorder const& recorder) {
    recorder.print(oss);
    return oss;
}

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
