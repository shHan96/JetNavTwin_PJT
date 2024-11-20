cmake_minimum_required(VERSION 3.10)
project(paho_jetbot)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

# Paho MQTT C++ 라이브러리 찾기
find_package(PahoMqttCpp QUIET)

if (NOT PahoMqttCpp_FOUND)
    message(WARNING "PahoMqttCpp package not found. Falling back to manual configuration.")

    # 헤더 경로 탐색
    find_path(PAHO_MQTT_CPP_INCLUDE_DIR
        NAMES mqtt/async_client.h
        PATHS /usr/local/include /usr/include
    )

    # 라이브러리 경로 탐색
    find_library(PAHO_MQTT_CPP_LIB
        NAMES libpaho-mqttpp3.so libpaho-mqttpp3.a
        PATHS /usr/local/lib /usr/lib
    )

    if (PAHO_MQTT_CPP_INCLUDE_DIR AND PAHO_MQTT_CPP_LIB)
        message(STATUS "PahoMqttCpp headers found at ${PAHO_MQTT_CPP_INCLUDE_DIR}")
        message(STATUS "PahoMqttCpp library found at ${PAHO_MQTT_CPP_LIB}")
    else()
        message(FATAL_ERROR "PahoMqttCpp could not be found. Please install it or specify the correct paths.")
    endif()
else()
    message(STATUS "PahoMqttCpp package found.")
endif()



# 실행 파일 및 소스
set(SOURCES 
    src/main.cpp
)
add_executable(${PROJECT_NAME} ${SOURCES})

# 라이브러리 연결
target_include_directories(${PROJECT_NAME} PRIVATE ${PAHO_MQTT_CPP_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)

if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.13")
    target_link_directories(${PROJECT_NAME} PRIVATE ${PAHO_MQTT_CPP_LIB_PATH})
    target_link_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
else()
    link_directories(${PAHO_MQTT_CPP_LIB_PATH})
endif()

target_link_libraries(${PROJECT_NAME} paho-mqttpp3 paho-mqtt3as)