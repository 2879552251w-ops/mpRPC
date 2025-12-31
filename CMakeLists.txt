cmake_minimum_required(VERSION 3.0)
project(mprpc)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

add_compile_options(-g )#-Wextra -Wall -pthread

include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/example)
include_directories(${PROJECT_SOURCE_DIR}/example/caller)

link_directories(${CMAKE_SOURCE_DIR}/lib)
link_directories(/usr/local/lib)
#src包含mprpc框架所有代码
add_subdirectory(src)
#包含使用框架的示例代码
add_subdirectory(example)