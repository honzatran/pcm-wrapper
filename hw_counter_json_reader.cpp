#include "hw_counter_json_reader.hpp"

#include "hw_counters_type.hpp"

#include <fstream>
#include <iostream>
#include <cstdint>
#include <string>

#include <json/json.hpp>

#include <range/v3/core.hpp>
#include <range/v3/view/map.hpp>
#include <range/v3/view/filter.hpp>

#include <dirent.h>
#include <unordered_map>

using json = nlohmann::json;

using namespace std;

using namespace PcmWrapper;

constexpr char c_EventName[] = "EventName";
constexpr char c_EventCode[] = "EventCode";
constexpr char c_UMask[] = "UMask";

const std::unordered_map<string, PCM::SupportedCPUModels> c_cpuModels = {
    {"broadwell", PCM::BROADWELL},
    {"broadwellde", PCM::BDX_DE},
    {"broadwellx", PCM::BDX},
    {"haswell", PCM::HASWELL},
    {"haswellx", PCM::HASWELLX},
    {"ivybridge", PCM::IVY_BRIDGE},
    {"ivytown", PCM::IVYTOWN},
    {"skylake", PCM::SKL},
};

bool
isFile(dirent *const ent) {
    return ent->d_type == DT_REG;
}

bool
isJson(string const& filename) {
    return filename.substr(filename.size() - 4) == "json";
}

void
HwCounterJsonReader::loadCounters(std::string const& filePath, int cpuModel) {
    std::ifstream inputStream(filePath);

    json j;

    inputStream >> j;

    for (const auto& element : j) {
        HwCounter counter = convert(element);

        auto counterIt = m_hwCounters.find(counter.getIdentifier());

        if (counterIt == m_hwCounters.end()) {
            counter.addSupportedCpuModel(cpuModel);
            m_hwCounters.insert({counter.getIdentifier(), counter});
        } else {
            counterIt->second.addSupportedCpuModel(cpuModel);
        }
    }
}

template <typename F>
void
processFile(string const& directory, dirent* const ent, F&& f) {
    string filename = ent->d_name;

    if (isJson(filename)) {
        f(filename);
    }
}


int getCPUModel(const std::string filename) {
    int pos = filename.find('_');

    if (pos == string::npos) {
        return -1;
    } else {
        auto it = c_cpuModels.find(filename.substr(0, pos));

        if (it != c_cpuModels.end()) {
            return it->second;
        }

        return -1;
    }
}


void 
HwCounterJsonReader::loadFromDirectory(std::string const& directory) {
    DIR *dir;
    dirent *ent;

    // use lambda and functor to avoid exposing libc api
    auto loadCounterFunctor = [this, &directory](std::string const& filename) {
        int cpuModel = getCPUModel(filename);

        if (cpuModel >= 0) {
            string fullpath = directory + "/" + filename;
            loadCounters(fullpath, getCPUModel(filename));
        }
    };

    if ((dir = opendir(directory.c_str())) != nullptr) {
        while ((ent = readdir(dir)) != nullptr) {
            if (isFile(ent)) {
                processFile(directory, ent, loadCounterFunctor);
            }
        }

        closedir(dir);
    }

}

HwCounter
HwCounterJsonReader::convert(nlohmann::json const& serializedCounter) {
    const std::string hexaEventMask = serializedCounter[c_EventCode];
    const std::string hexaUMask     = serializedCounter[c_UMask];
    const std::string hwCounterName = serializedCounter[c_EventName];

    const int eventMask = std::stoi(hexaEventMask, 0, 16);
    const int umask     = std::stoi(hexaUMask, 0, 16);

    return HwCounter(eventMask, umask, hwCounterName, {});
}

std::vector<HwCounter>
HwCounterJsonReader::getCounters(int cpuModel) const {
    return m_hwCounters 
        | ranges::view::values 
        | ranges::view::filter([cpuModel](HwCounter const& counter) {
               return counter.supports(cpuModel);
           });

    // std::vector<HwCounter> counters;
    // for (const auto& p : m_hwCounters) {
    //     auto& cpumodels = p.second.getSupportedModels();
    //
    //     bool supports = p.second.supports(cpuModel);
    //
    //     if (supports) {
    //         counters.push_back(p.second);
    //     }
    // }
    // return counters;
}






