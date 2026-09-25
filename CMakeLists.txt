cmake_minimum_required(VERSION 3.20)
project(srt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

file(GLOB SRT_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/srt/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/srt/sources/*.cpp)

add_library(srt STATIC ${SRT_SOURCES})
target_include_directories(srt PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/srt)

find_package(Threads REQUIRED)
target_link_libraries(srt PUBLIC Threads::Threads)

if(MSVC)
    target_compile_options(srt PUBLIC /bigobj)
else()
    # Windows loads fonts from C:\Windows\Fonts; elsewhere fontconfig finds them
    find_package(Fontconfig REQUIRED)
    target_link_libraries(srt PRIVATE Fontconfig::Fontconfig)
endif()

# one program per example topic, e.g. Examples/telescope.cpp -> telescope
foreach(topic coma render telescope color prism sky sampling)
    add_executable(${topic} Examples/${topic}.cpp)
    target_link_libraries(${topic} PRIVATE srt)
endforeach()
