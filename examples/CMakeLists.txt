
add_executable(cnet_client "client.cpp")
target_link_libraries(cnet_client "cnet")

add_executable(cnet_server "server.cpp")
target_link_libraries(cnet_server "cnet")
