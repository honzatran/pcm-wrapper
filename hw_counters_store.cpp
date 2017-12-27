#include "hw_counters_store.hpp"

#include "error_handling.hpp"

using namespace pcm_wrapper;

HwCounterStore::HwCounterStore(std::size_t const capacity)
    : m_counters(capacity), m_index(0)
{
}

void
HwCounterStore::append(std::uint64_t count)
{
    ASSERT(m_index < m_counters.size(), "index out of bounds");

    m_counters[m_index++] = count;
}

std::uint64_t HwCounterStore::operator[](std::size_t index) const
{
    ASSERT(index < m_index, "index out of bounds");
    return m_counters[index];
}
