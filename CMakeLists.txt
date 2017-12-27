project(BENCH)

cmake_minimum_required(VERSION 3.0.2)

set(PCM_WRAPPER "pcm-wrapper")
set(CMAKE_CXX_FLAGS "-std=c++11 -fno-omit-frame-pointer")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/3rd-party/cmake)

find_package(PCM REQUIRED)
find_package(JSON REQUIRED)
find_package(CATCH REQUIRED)
find_package(Boost REQUIRED)

message(${PCM_INCLUDE_DIRS})

include_directories(${PCM_INCLUDE_DIRS} ${JSON_INCLUDE_DIRS} ${CATCH_INCLUDE_DIRS})
 
add_library(${PCM_WRAPPER} SHARED "hw_counter_json_reader.cpp" "pcm_context.cpp" "error_handling.cpp" "hw_counters_store.cpp")
target_link_libraries(${PCM_WRAPPER} PRIVATE ${PCM_LIBRARIES} ${BOOST_LIBRARIES})
add_definitions(${PCM_DEFINITIONS} ${JSON_DEFINITIONS})

install(TARGETS ${PCM_WRAPPER} DESTINATION "lib")
install(FILES "pcm_context.hpp" "hw_counter.hpp" "hw_counter_json_reader.hpp" "error_handling.hpp" DESTINATION "include")

install(DIRECTORY ${JSON_INCLUDE_DIRS} DESTINATION ".")
install(DIRECTORY ${PCM_INCLUDE_DIRS} DESTINATION ".")
install(FILES ${PCM_LIBRARIES} DESTINATION "lib")
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/FindPCMWrapper.cmake DESTINATION "cmake")

add_executable(main main.cpp)

target_link_libraries(main ${PCM_WRAPPER} ${PCM_LIBRARIES})

add_executable(multithread main_multithreaded.cpp)
target_link_libraries(multithread ${PCM_WRAPPER} ${PCM_LIBRARIES})

add_executable(pcm_test test/main_test.cpp test/assert_test.cpp test/hw_counter_store_test.cpp)

target_link_libraries(pcm_test ${PCM_WRAPPER} ${PCM_LIBRARIES})

if (UNIX)
    target_link_libraries(multithread pthread)
    target_link_libraries(main pthread)
endif(UNIX)
