#ifndef HW_TYPE_COUNTERS_HPP 
#define HW_TYPE_COUNTERS_HPP

#include <cstdint>
#include <string>
#include <mutex>
#include <algorithm>
#include <type_traits>


#include <pcm/types.h>
#include <pcm/cpucounters.h>

namespace PcmWrapper {

class HwCounter;

namespace detail 
{
    static std::mutex counter_mtx;

    struct RegistredCounters 
    {
        static std::vector<HwCounter> counters;
    };

    template <typename T, typename U>
    struct DecayEquiv : std::is_same<T, typename std::decay_t<U>>::type
    {};
}


class HwCounter
{
public:
    HwCounter()
        : m_eventNumber(0), m_umaskValue(0), m_name("") {};

    HwCounter(std::int32_t eventNumber,
              std::int32_t umaskValue,
              std::string const& name,
              std::vector<int> const& supportedCpuModels)
        : m_eventNumber(eventNumber),
          m_umaskValue(umaskValue),
          m_name(name),
          m_supportedCpuModels(supportedCpuModels) {}

    std::int32_t getEventNumber() const { return m_eventNumber; }
    std::int32_t getUMaskValue() const { return m_umaskValue; }

    std::vector<int> const& getSupportedModels() const {
        return m_supportedCpuModels;
    }

    std::string const& getIdentifier() const { return m_name; }

    void addSupportedCpuModel(int cpuModel) {
        m_supportedCpuModels.push_back(cpuModel);
    }

    bool supports(int cpuModel) const {
        return std::find(m_supportedCpuModels.begin(),
                         m_supportedCpuModels.end(),
                         cpuModel) != m_supportedCpuModels.end();
    }

private:
    std::int32_t m_eventNumber;
    std::int32_t m_umaskValue;

    std::string m_name;
    std::vector<int> m_supportedCpuModels;
};


std::vector<HwCounter> getRegistredHwCounters();

void registerHwCounter(const HwCounter& HwCounter);

template <typename ITERATOR,
          typename = typename std::enable_if_t<detail::DecayEquiv<
              HwCounter,
              typename std::iterator_traits<ITERATOR>::value_type>::value>>
void
registerHwCounters(ITERATOR&& begin, ITERATOR&& end) {
    std::lock_guard<std::mutex> lockGuard(detail::counter_mtx);
    std::copy(begin, end, std::back_inserter(detail::RegistredCounters::counters));
}


}

#endif
