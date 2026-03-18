cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable folders in IDEs like Visual Studio
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

file(GLOB_RECURSE STRATUM_PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src/Public/*.h")
file(GLOB_RECURSE STRATUM_PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src/Private/*.cpp")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/$<CONFIG>)

project(Stratum)
add_library(Stratum SHARED ${STRATUM_PUBLIC} ${STRATUM_PRIVATE})

target_include_directories(Stratum PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src/Public)
target_precompile_headers(Stratum PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/Public/Stratum.h)

add_compile_definitions(
	STRATUM_SHARED
)
