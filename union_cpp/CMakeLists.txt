cmake_minimum_required(VERSION 3.10)
project(road_following)

# 크로스 컴파일 설정
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc-8)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++-8)

# C++14 표준 설정
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 경로 설정
set(TORCH_PATH "/usr/aarch64-linux-gnu")
set(CUDA_PATH "/usr/local/cuda-10.0")
set(TARGET_PATH "/usr/aarch64-linux-gnu")
set(TORCH_DEPLOY_PATH "/usr/local/lib/python3.6/dist-packages/torch")

# OpenCV 수동 설정 (find_package 대신)
set(OpenCV_INCLUDE_DIRS "/usr/aarch64-linux-gnu/include/opencv4")
set(OpenCV_LIBS 
    opencv_core
    opencv_highgui
    opencv_imgproc
    opencv_videoio
)

# Paho MQTT C++ 라이브러리 수동 설정 (find_package 대신)
set(PAHO_MQTT_CPP_INCLUDE_DIR "/usr/local/include")
set(PAHO_MQTT_CPP_LIB "/usr/local/lib/libpaho-mqttpp3.so")

# Include 디렉토리 설정
include_directories(
    ${TORCH_PATH}/include
    ${TORCH_PATH}/include/torch/csrc/api/include
    ${OpenCV_INCLUDE_DIRS}
    ${CUDA_PATH}/targets/aarch64-linux/include
    ${PROJECT_SOURCE_DIR}/include
)

# 라이브러리 경로 추가
link_directories(
    ${TORCH_PATH}/lib
    ${CUDA_PATH}/targets/aarch64-linux/lib
    ${TARGET_PATH}/lib
)

# 소스 파일 목록
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 실행 파일 생성
add_executable(road_following ${SOURCES})

# 라이브러리 링크
target_link_libraries(road_following
    ${OpenCV_LIBS}
    torch
    c10
    pthread
)

# rpath 설정
set_target_properties(road_following PROPERTIES
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "${TORCH_DEPLOY_PATH}/lib"
    LINK_FLAGS "-Wl,-rpath-link=${CUDA_PATH}/targets/aarch64-linux/lib -Wl,-rpath-link=${TARGET_PATH}/lib"
)

# 최적화 옵션 설정
target_compile_options(road_following PRIVATE -O3)

# 설치 규칙
install(TARGETS road_following
    RUNTIME DESTINATION bin
)

# 실행 스크립트 생성
file(WRITE ${CMAKE_BINARY_DIR}/run.sh
"#!/bin/bash
export LD_LIBRARY_PATH=${TORCH_DEPLOY_PATH}/lib:\$LD_LIBRARY_PATH
chmod +x ./road_following
./road_following
")

file(COPY ${CMAKE_BINARY_DIR}/run.sh
    DESTINATION ${CMAKE_BINARY_DIR}
    FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)