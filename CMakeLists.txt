project(BENCH)

cmake_minimum_required(VERSION 3.0.2)

set(EXEC "pcm-wrapper")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/pcm/cmake)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/json/cmake)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/range-v3/cmake)

find_package(PCM REQUIRED)
find_package(JSON REQUIRED)
find_package(RANGE_V3 REQUIRED)

message(${PCM_INCLUDE_DIRS})
include_directories(${PCM_INCLUDE_DIRS} ${JSON_INCLUDE_DIRS} ${RANGE_V3_INCLUDE_DIRS})
 


add_library(${EXEC} SHARED "hw_counters_type.cpp" "hw_counter_json_reader.cpp" "pcm_context.cpp")
target_link_libraries(${EXEC} PRIVATE ${PCM_LIBRARIES})
add_definitions(${PCM_DEFINITIONS} ${JSON_DEFINITIONS} ${RANGE_V3_DEFINITIONS})

install(TARGETS ${EXEC} DESTINATION "lib")
install(FILES "pcm_context.hpp" "hw_counters_type.hpp" "hw_counter_json_reader.hpp"  DESTINATION "include")


install(DIRECTORY ${JSON_INCLUDE_DIRS} DESTINATION ".")
install(DIRECTORY ${PCM_INCLUDE_DIRS} DESTINATION ".")
install(DIRECTORY ${RANGE_V3_INCLUDE_DIRS} DESTINATION ".")
install(FILES ${PCM_LIBRARIES} DESTINATION "lib")
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/FindPCMWrapper.cmake DESTINATION "cmake")
