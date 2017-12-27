
#ifndef HW_COUNTER_JSON_READER_HPP
#define HW_COUNTER_JSON_READER_HPP

#include <json/json.hpp>

#include <unordered_map>
#include "hw_counter.hpp"

namespace pcm_wrapper
{
class HwCounterJsonReader
{
public:
    void loadCounters(std::string const& jsonDbPath, int cpumodel);

    void loadFromDirectory(std::string const& directory);

    std::vector<pcm_wrapper::HwCounter> getCounters(int cpuModel) const;

private:
    std::unordered_map<std::string, pcm_wrapper::HwCounter> m_hwCounters;

    HwCounter convert(nlohmann::json const& serializedCounter);
};
}

#endif
