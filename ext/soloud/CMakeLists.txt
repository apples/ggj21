cmake_minimum_required(VERSION 3.12)
project(SoLoud)

file(GLOB_RECURSE soloud_SOURCES src/audiosource/* src/filter/* src/core/*)
file(GLOB_RECURSE soloud_PLATFORM_SOURCES src/backend/sdl/soloud_sdl2*)

add_library(soloud ${soloud_SOURCES} ${soloud_PLATFORM_SOURCES})
target_include_directories(soloud PUBLIC include)

if (EMSCRIPTEN)
    target_compile_definitions(soloud PRIVATE "WITH_SDL2_STATIC")
    set_target_properties(soloud PROPERTIES
        COMPILE_FLAGS "-s USE_SDL=2"
        LINK_FLAGS "-s USE_SDL=2")
else()
    target_compile_definitions(soloud PRIVATE "WITH_SDL2")

    if(WIN32)
        target_compile_definitions(soloud PRIVATE "WINDOWS_VERSION")
    endif()

    find_package(SDL2 REQUIRED)
    target_link_libraries(soloud SDL2::SDL2)
endif()
