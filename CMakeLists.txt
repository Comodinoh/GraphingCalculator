cmake_minimum_required (VERSION 3.10)

project("GraphingCalculator")

add_executable(GraphingCalculator src/expressions.h src/ui.h src/main.cpp)
target_include_directories(GraphingCalculator PRIVATE src)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

target_compile_definitions(GraphingCalculator PRIVATE GRAPHICS_ROOT_DIR="${CMAKE_SOURCE_DIR}")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET GraphingCalculator PROPERTY CXX_STANDARD 20)
endif()

add_subdirectory(vendor/SFML)

target_link_libraries(GraphingCalculator PRIVATE SFML::Graphics SFML::System)
target_include_directories(GraphingCalculator PRIVATE vendor/SFML/include)

# TODO: Add tests and install targets if needed.
