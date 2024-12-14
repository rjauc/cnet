cmake_minimum_required(VERSION 3.10)
project(cnet VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)

include_directories("inc")
file(GLOB_RECURSE cnet_SRC CONFIGURE_DEPENDS "src/*.cpp")
add_library(cnet ${cnet_SRC})
target_link_libraries(cnet PRIVATE "Ws2_32")

add_subdirectory("examples")
