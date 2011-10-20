cmake_minimum_required(VERSION 2.6)

find_package(Boost COMPONENTS system)

add_executable(udp_multicast_receiver_example udp_multicast_receiver_example.cpp)
target_link_libraries(udp_multicast_receiver_example ${Boost_LIBRARIES} pthread)
