cmake_minimum_required(VERSION 3.10)
project(cnet VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)

include_directories("inc")

if (WIN32)
    set(SOURCE_DIR "src/win32")
elseif(UNIX)
    set(SOURCE_DIR "src/unix")
endif()
file(GLOB_RECURSE cnet_SRC CONFIGURE_DEPENDS "${SOURCE_DIR}/*.cpp")

add_library(cnet ${cnet_SRC})
if (WIN32)
    target_link_libraries(cnet PRIVATE "Ws2_32")
endif()

add_subdirectory("examples")
