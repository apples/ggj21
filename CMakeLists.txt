cmake_minimum_required(VERSION 3.12)
project(GGJ21)

set(EMBER_DATA_DIR "${CMAKE_SOURCE_DIR}/data" CACHE PATH "Data Directory")
set(EMBER_DATA_SRC "${CMAKE_SOURCE_DIR}/data_src" CACHE PATH "Data Source Directory")
set(EMBER_DATA_DST "${CMAKE_BINARY_DIR}/data" CACHE PATH "Temporary Data Output Directory")

set(EMBER_PLATFORM_DATA_DIR "${CMAKE_SOURCE_DIR}/platform/x86_64/data" CACHE PATH "Platform Data Directory")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(ExternalProject)

set(SKIP_INSTALL_ALL Yes)
set(FT_WITH_ZLIB Off)
set(FT_WITH_BZIP2 Off)
set(FT_WITH_PNG Off)
set(FT_WITH_HARFBUZZ Off)
set(FT_WITH_BROTLI Off)

add_subdirectory(ext/freetype2)
add_subdirectory(ext/ginseng)
add_subdirectory(ext/lua)
add_subdirectory(ext/sol2)
add_subdirectory(ext/glm)
add_subdirectory(ext/lodepng)
add_subdirectory(ext/sushi)
add_subdirectory(ext/msdfgen)
add_subdirectory(ext/soloud)

option(BOX2D_BUILD_UNIT_TESTS "Build the Box2D unit tests" OFF)
option(BOX2D_BUILD_TESTBED "Build the Box2D testbed" OFF)
add_subdirectory(ext/box2d)

target_compile_definitions(sol2 INTERFACE SOL_CXX17_FEATURES=1)

set(EMBER_OUT_DIR "${CMAKE_BINARY_DIR}" CACHE PATH "Output Directory")
set(EMBER_OUT_DATA_DIR "${EMBER_OUT_DIR}/data" CACHE PATH "Data Output Directory")

find_package(SDL2 REQUIRED)

function(add_data_files TARGETNAME DIRECTORY)
    file(GLOB_RECURSE EMBER_DATA_FILES CONFIGURE_DEPENDS "${DIRECTORY}/*")
    set(EMBER_DATA_FILES_OUT)
    foreach(DATA_FILE ${EMBER_DATA_FILES})
        file(RELATIVE_PATH DATA_FILE_REL "${DIRECTORY}" "${DATA_FILE}")
        set(DATA_FILE_OUT "${EMBER_OUT_DATA_DIR}/${DATA_FILE_REL}")
        add_custom_command(
            OUTPUT "${DATA_FILE_OUT}"
            COMMAND "${CMAKE_COMMAND}" -E copy "${DATA_FILE}" "${DATA_FILE_OUT}"
            COMMENT "Copying data file ${DATA_FILE_REL}"
            DEPENDS "${DATA_FILE}")
        list(APPEND EMBER_DATA_FILES_OUT "${DATA_FILE_OUT}")
    endforeach()
    add_custom_target(${TARGETNAME}
        SOURCES ${EMBER_DATA_FILES}
        DEPENDS ${EMBER_DATA_FILES_OUT})
endfunction()

# Static Data Files
add_data_files(ember_data "${EMBER_DATA_DIR}")

# Platform Data Files
add_data_files(ember_platform_data "${EMBER_PLATFORM_DATA_DIR}")

# Client C++
file(GLOB_RECURSE EMBER_SRCS CONFIGURE_DEPENDS src/*.cpp src/*.hpp)
add_executable(ggj21 ${EMBER_SRCS} ${EMBER_JS})
target_compile_features(ggj21 PUBLIC cxx_std_17)
target_compile_definitions(ggj21 PUBLIC SDL_MAIN_HANDLED)
target_include_directories(ggj21 PUBLIC
    "${CMAKE_SOURCE_DIR}/ext/asio/asio/include"
    "${CMAKE_SOURCE_DIR}/ext/trellis/include"
    "${CMAKE_SOURCE_DIR}/ext/cereal/include")
target_link_libraries(ggj21
    -static
    ginseng
    sushi
    sol2
    msdfgen
    soloud
    box2d
    SDL2::SDL2-static
    freetype)
add_dependencies(ggj21 ember_data ember_platform_data)

if(WIN32)
    target_link_libraries(ggj21 ws2_32)
endif()

if(MINGW)
    message(STATUS "Applying hacks for MinGW builds")

    set(ver ${CMAKE_SYSTEM_VERSION})
    string(REPLACE "." "" ver ${ver})
    string(REGEX REPLACE "([0-9])" "0\\1" ver ${ver})

    target_compile_options(ggj21 PUBLIC "-Wa,-mbig-obj")
    target_compile_definitions(ggj21 PUBLIC "_WIN32_WINNT=0x${ver}")

    set_source_files_properties("${CMAKE_SOURCE_DIR}/src/ember/box2d_lua.cpp"
        PROPERTIES
        COMPILE_FLAGS "-O3")
endif()
