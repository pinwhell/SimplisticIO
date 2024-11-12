add_executable(playground playground.cpp)
#target_include_directories(playground  PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(playground simplistic::io)
