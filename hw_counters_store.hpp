#ifndef HW_COUNTERS_STORE
#define HW_COUNTERS_STORE

#include <cstdint>
#include <vector>

class HwCounterStore
{
public:
    HwCounterStore(std::size_t const capacity);

    void append(std::uint64_t value);

    std::uint64_t operator[](std::size_t index) const;

    std::size_t size() const { return m_index; }
private:
    std::vector<std::uint64_t> m_counters;
    std::size_t m_index;
};

class ImmutableCounterStore
{
public:
    ImmutableCounterStore(HwCounterStore store)
    {
        m_store = std::make_shared<HwCounterStore>(std::move(store));
    }

    std::uint64_t operator[](std::size_t index) const;

    // std::size_t size() const { return m_index; }
private:
    std::shared_ptr<HwCounterStore> m_store;
};

#endif
