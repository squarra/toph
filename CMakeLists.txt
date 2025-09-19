cmake_minimum_required(VERSION 3.10)
project(OpenGLDemo LANGUAGES C CXX)

option(USE_PYBIND "compile with pybind" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 REQUIRED)

set(TOPH_SOURCES src/frame.cpp src/viewer.cpp src/glad.c)

add_library(toph SHARED ${TOPH_SOURCES})
target_include_directories(toph PUBLIC include)
target_link_libraries(toph PRIVATE glfw)

if(USE_PYBIND)
  set(PYBIND11_FINDPYTHON ON)
  find_package(pybind11 CONFIG REQUIRED)
  pybind11_add_module(pytoph src/bindings.cpp)
  target_link_libraries(pytoph PRIVATE toph)
  install(TARGETS pytoph DESTINATION lib/python)
endif(USE_PYBIND)

add_executable(main src/main.cpp)

target_include_directories(main PRIVATE include)
target_link_libraries(main PRIVATE toph)
