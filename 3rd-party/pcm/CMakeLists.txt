cmake_minimum_required(VERSION 3.0.2)

add_library(PCM msr.cpp cpucounters.cpp pci.cpp client_bw.cpp utils.cpp)


target_compile_options(PCM PUBLIC "-std=c++14")

set_property(TARGET PCM PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(PCM PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


set (HEADERS client_bw.h cpucounters.h mutex.h types.h width_extender.h cpuasynchcounter.h msr.h pci.h utils.h)

install(TARGETS PCM DESTINATION lib)
install(FILES ${HEADERS} DESTINATION "include/pcm")

if (APPLE)
    target_include_directories(PCM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/MacMSRDriver")
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/MacMSRDriver/build/Release/libPcmMsr.dylib DESTINATION "lib")
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/FindPCM.cmake DESTINATION "cmake")
endif (APPLE)

