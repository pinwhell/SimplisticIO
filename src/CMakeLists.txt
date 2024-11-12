file(GLOB SIO_SRCS *.cpp)
add_library_ns(simplistic io STATIC ${SIO_SRCS})
target_include_dir_iface(simplistic-io PUBLIC ${CMAKE_SOURCE_DIR}/include include)
install_target_and_headers(simplistic io)