

#ifndef PCM_WRAPPER_DYNAMIC_COUNTER_RECORDER_HPP
#define PCM_WRAPPER_DYNAMIC_COUNTER_RECORDER_HPP

template <typename COUNTER_HANDLE>
class DynamicCounterRecorder
{
public:
    DynamicCounterRecorder(COUNTER_HANDLE handle);

    void onStart()
    {
        m_counter_handle.onStart();
    }

    void onEnd()
    {
        m_counter_handle.onEnd();
    }

private:

    COUNTER_HANDLE m_counter_handle;
};



#endif /* dynamic_counter_recorder.hpp */
