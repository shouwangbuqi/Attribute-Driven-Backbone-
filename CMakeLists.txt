cmake_minimum_required(VERSION 3.8)
project(PCSTStaticV1)

message(STATUS "start running cmake...")

find_package(Boost 1.65.0 COMPONENTS system filesystem REQUIRED)

if(Boost_FOUND)

    message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")
    message(STATUS "Boost_VERSION: ${Boost_VERSION}")

    include_directories(${Boost_INCLUDE_DIRS})

endif()

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cc pairing_heap.h priority_queue.h pcst_fast.cc pcst_fast.h Graph.cpp Graph.h)
add_executable(PCSTStaticV1 ${SOURCE_FILES} graph.h)

if(Boost_FOUND)

    target_link_libraries(PCSTStaticV1 ${Boost_LIBRARIES})

endif()