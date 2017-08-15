
set(PCM_WRAPPER_INCLUDE_DIRS  "${CMAKE_CURRENT_LIST_DIR}/../include")

set (PCM_WRAPPER_DEFINITIONS "-std=c++11")

file(GLOB PCM_WRAPPER_LIBRARIES "${CMAKE_CURRENT_LIST_DIR}/../lib/*.dylib" 
                                "${CMAKE_CURRENT_LIST_DIR}/../lib/*.a"
                                "${CMAKE_CURRENT_LIST_DIR}/../lib/*.so")
