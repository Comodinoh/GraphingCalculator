cmake_minimum_required (VERSION 3.10)

project("GraphingCalculator")

add_executable(GraphingCalculator src/main.cpp)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET GraphingCalculator PROPERTY CXX_STANDARD 20)
endif()

add_subdirectory(vendor/SFML)

target_link_libraries(GraphingCalculator PRIVATE SFML::Graphics SFML::System)

# TODO: Add tests and install targets if needed.
