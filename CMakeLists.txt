cmake_minimum_required(VERSION 3.16)
project(RescateUSB VERSION 0.4.0 LANGUAGES CXX)

add_executable(rescateusb
  src/main.cpp
  src/hash.cpp
  src/partition_tables.cpp
  src/image_analyzer.cpp
  src/report.cpp
  src/self_tests.cpp
)
target_compile_features(rescateusb PRIVATE cxx_std_17)
if(MSVC)
  target_compile_options(rescateusb PRIVATE /W4 /permissive-)
else()
  target_compile_options(rescateusb PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()
add_test(NAME rescateusb_self_test COMMAND rescateusb --self-test)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
add_test(NAME rescateusb_cli_integration
  COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/test_cli.py $<TARGET_FILE:rescateusb>)
install(TARGETS rescateusb RUNTIME DESTINATION bin)
