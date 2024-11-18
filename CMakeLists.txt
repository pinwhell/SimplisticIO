cmake_minimum_required(VERSION 3.15)

project(simplistic-io)

option(USE_STATIC_LINKING "Use static linking for the runtime" ON)

if (USE_STATIC_LINKING)
    if (MSVC)
        # MSVC specific static linking flag
        add_compile_options(/MT$<$<CONFIG:Debug>:d>)
    else()
        # GCC/Clang specific static linking flag
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
    endif()
endif()


find_package(CBuildKit REQUIRED)

add_subdirectory(src)
add_subdirectory(tests)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/ DESTINATION include)

install_cfgpkg(simplistic
"
include(\${CMAKE_CURRENT_LIST_DIR}/simplistic-io-targets.cmake)"
)
