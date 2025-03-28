cmake_minimum_required(VERSION 3.10)
project(SimpleFileManager)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(FileManager 
    src/main.cpp
    src/FileManager.cpp
)

target_include_directories(FileManager PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

if(WIN32)
    target_compile_definitions(FileManager PRIVATE OS_WINDOWS)
endif()
