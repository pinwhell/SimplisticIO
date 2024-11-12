cmake_minimum_required(VERSION 3.15)

project(simplistic-io)
include(CBuildKit)

add_subdirectory(src)
add_subdirectory(tests)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/ DESTINATION include)

install_cfgpkg(simplistic
"
include(\${CMAKE_CURRENT_LIST_DIR}/simplistic-io-targets.cmake)"
)
