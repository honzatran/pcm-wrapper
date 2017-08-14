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


}

#endif