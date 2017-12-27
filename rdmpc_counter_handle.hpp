
#ifndef RDPMC_COUNTER_HANDLE_HPP
#define RDPMC_COUNTER_HANDLE_HPP

#include "pcm_context.hpp"

namespace pcm_wrapper
{
namespace detail
{
constexpr unsigned RETIRED_INSTRUCTION = 1 << 30;
constexpr unsigned EXECUTED_CYCLES     = RETIRED_INSTRUCTION + 1;

template <unsigned COUNTER>
std::uint64_t
RDPMC()
{
    unsigned a, d;
    __asm__ __volatile__("rdpmc" : "=a"(a), "=d"(d) : "c"(COUNTER));
    return ((std::uint64_t)a) | (((std::uint64_t)d) << 32);
}

inline std::uint64_t
getRetiredInstructions()
{
    return RDPMC<RETIRED_INSTRUCTION>();
}

inline std::uint64_t
getExecutedCycles()
{
    return RDPMC<EXECUTED_CYCLES>();
}

}

class RDPMCCountersHandle
{
public:
#if __linux__
    void onStart()
    {
        m_startState.instructions = detail::getRetiredInstructions();
        m_startState.cycles       = detail::getExecutedCycles();

        m_startState.p[0] = detail::RDPMC<0>();
        m_startState.p[1] = detail::RDPMC<1>();
        m_startState.p[2] = detail::RDPMC<2>();
        m_startState.p[3] = detail::RDPMC<3>();
    }

    void onEnd()
    {
        m_endState.instructions = detail::getRetiredInstructions();
        m_endState.cycles       = detail::getExecutedCycles();

        m_endState.p[0] = detail::RDPMC<0>();
        m_endState.p[1] = detail::RDPMC<1>();
        m_endState.p[2] = detail::RDPMC<2>();
        m_endState.p[3] = detail::RDPMC<3>();
    }

    template <CounterRegister order>
    std::uint64_t getEventCounts() const
    {
        return m_endState.p[order] - m_startState.p[order];
    }

    std::uint64_t getExecutedInstructions() const
    {
        return m_endState.instructions - m_startState.instructions;
    }

    std::uint64_t getExecutedCycles() const
    {
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

#endif
};
};

#endif
