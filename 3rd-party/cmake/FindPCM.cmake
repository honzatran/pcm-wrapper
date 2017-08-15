
set(PCM_INCLUDE_DIRS  "${CMAKE_CURRENT_LIST_DIR}/../pcm/include")

set (PCM_LIBRARIES "${CMAKE_CURRENT_LIST_DIR}/../pcm/lib/libPCM.a")
set (PCM_DEFINITIONS "-std=c++11")

if (APPLE)
    list(APPEND PCM_LIBRARIES "${CMAKE_CURRENT_LIST_DIR}/../pcm/lib/libPcmMsr.dylib")
endif(APPLE)
