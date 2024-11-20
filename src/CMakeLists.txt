add_executable(jetson_app main.cpp)

target_include_directories(jetson_app PRIVATE ${CMAKE_SOURCE_DIR}/include)