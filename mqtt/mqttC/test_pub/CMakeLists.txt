cmake_minimum_required(VERSION 3.10)
project(mqtt_test_pub)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find mosquitto library
find_library(MOSQUITTO_LIBRARY mosquitto REQUIRED)

# Add executable
add_executable(${PROJECT_NAME} main.c)

# Link mosquitto library
target_link_libraries(${PROJECT_NAME} PRIVATE ${MOSQUITTO_LIBRARY})

# Add compiler warnings
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)

# Set build type to Release by default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()