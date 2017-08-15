project(BENCH)

cmake_minimum_required(VERSION 3.0.2)

set(PCM_WRAPPER "pcm-wrapper")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/pcm/cmake)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/3rd-party/cmake)

find_package(PCM REQUIRED)
find_package(JSON REQUIRED)

message(${PCM_INCLUDE_DIRS})
include_directories(${PCM_INCLUDE_DIRS} ${JSON_INCLUDE_DIRS})
 
add_library(${PCM_WRAPPER} SHARED "hw_counter_json_reader.cpp" "pcm_context.cpp")
target_link_libraries(${PCM_WRAPPER} PRIVATE ${PCM_LIBRARIES})
add_definitions(${PCM_DEFINITIONS} ${JSON_DEFINITIONS})

install(TARGETS ${PCM_WRAPPER} DESTINATION "lib")
install(FILES "pcm_context.hpp" "hw_counter.hpp" "hw_counter_json_reader.hpp"  DESTINATION "include")

install(DIRECTORY ${JSON_INCLUDE_DIRS} DESTINATION ".")
install(DIRECTORY ${PCM_INCLUDE_DIRS} DESTINATION ".")
install(FILES ${PCM_LIBRARIES} DESTINATION "lib")
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/FindPCMWrapper.cmake DESTINATION "cmake")

add_executable(main main.cpp)
target_link_libraries(main ${PCM_WRAPPER} ${PCM_LIBRARIES})
