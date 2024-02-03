cmake_minimum_required(VERSION 3.27)
project(filr_explorer C)

set(CMAKE_C_STANDARD 11)

add_executable(filr_explorer main.c
        view.h
        src/view.c
        cstr.h
        src/cstr.c
        inputs.h
        src/inputs.c
        filr.h
        src/filr.c
        result.h
        hash_map.h
        src/hash_map.c
)

target_link_directories(filr_explorer PUBLIC lib)
target_link_libraries(filr_explorer raylib opengl32 gdi32 winmm)