cmake_minimum_required(VERSION 3.10.0)
project(cnet VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 23)

file(GLOB_RECURSE cnet_SRC CONFIGURE_DEPENDS 
    # "ge2d.o"
    "src/*.cpp")
add_library(cnet ${cnet_SRC})
target_include_directories(cnet PRIVATE
    "inc")
target_link_libraries(cnet PRIVATE
    "Ws2_32")

file(GLOB_RECURSE cnet_test_SRC CMAKE_CONFIGURE_DEPENDS
    "test/src/*.cpp")
add_executable(cnet_test ${cnet_test_SRC})
target_include_directories(cnet_test PRIVATE
    "inc"
    "test/inc")
target_link_directories(cnet_test PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(cnet_test PRIVATE
    "cnet")

# add_custom_target(ge2d_copy_resources 
#     COMMAND ${CMAKE_COMMAND} -E copy_directory 
#     ${CMAKE_CURRENT_LIST_DIR}/resources 
#     ${CMAKE_CURRENT_BINARY_DIR}/resources)
# add_dependencies(ge2d ge2d_copy_resources)
