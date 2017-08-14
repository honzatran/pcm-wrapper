#include "hw_counters_type.hpp"
#include <mutex>

using namespace PcmWrapper;

std::vector<HwCounter> detail::RegistredCounters::counters = {};

std::vector<HwCounter> getRegistredHwCounters() {
    return detail::RegistredCounters::counters;
}

void registerCounter(HwCounter const& hwCounter) {
    std::lock_guard<std::mutex> lockGuard(detail::counter_mtx);

    detail::RegistredCounters::counters.push_back(hwCounter);
}
