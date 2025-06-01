cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable folders in IDEs like Visual Studio
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/$<CONFIG>)

project(Instrumenta)
include_directories(src/Public)
add_library(Instrumenta SHARED src/Private/Instrumenta.cpp src/Public/Instrumenta.h)
