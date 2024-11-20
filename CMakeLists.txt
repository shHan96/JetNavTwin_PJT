cmake_minimum_required(VERSION 3.10)
project(JetsonCrossCompile)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 크로스 컴파일 툴체인 파일 포함
if(DEFINED ENV{CROSS_COMPILE})
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/jetson-toolchain.cmake)
endif()

# 서브 디렉토리 추가
add_subdirectory(src)