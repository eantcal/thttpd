cmake_minimum_required(VERSION 2.8.12)

include_directories(. include)

file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/cppsrc/*.cc")

set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++14" )

add_executable(thttpd ${SOURCES})

target_link_libraries(thttpd -pthread)
