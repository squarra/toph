cmake_minimum_required(VERSION 3.10)
project(OpenGLDemo LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 REQUIRED)

add_executable(toph src/main.cpp src/glad.c)

target_include_directories(toph PRIVATE include)
target_link_libraries(toph PRIVATE glfw)
