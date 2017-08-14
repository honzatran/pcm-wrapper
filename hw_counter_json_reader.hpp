
#ifndef HW_COUNTER_JSON_READER_HPP
#define HW_COUNTER_JSON_READER_HPP

#include <json/json.hpp>

#include "hw_counters_type.hpp"
#include <unordered_map>

namespace PcmWrapper {

class HwCounterJsonReader
{
public:
    void loadCounters(std::string const& jsonDbPath, int cpumodel);

    void loadFromDirectory(std::string const& directory);

    std::vector<PcmWrapper::HwCounter> getCounters(int cpuModel) const;
private:
    std::unordered_map<std::string, PcmWrapper::HwCounter> m_hwCounters;

    HwCounter convert(nlohmann::json const& serializedCounter);
};

}

#endif
