cmake_minimum_required(VERSION 3.22...3.29)
project(SeatCanvasKit LANGUAGES C CXX)


list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/modules")

include(Utils)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()


set(PROJECT_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})
set(PROJECT_ROOT_INCLUDE_DIR ${CMAKE_CURRENT_LIST_DIR}/include)
set(PROJECT_THIRD_PARTY_ROOT_DIR ${CMAKE_CURRENT_LIST_DIR}/third_party)

# set(_SEATCANVAS_PREV_BUILD_TYPE "${CMAKE_BUILD_TYPE}")
# set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)

set(TGFX_BUILD_SVG ON CACHE INTERNAL "")
set(TGFX_BUILD_LAYERS ON CACHE INTERNAL "")

if(EMSCRIPTEN)
    set(TGFX_USE_FREETYPE ON CACHE BOOL "Use FreeType on Web instead of WebTypeface" FORCE)
endif()

if(MACOS OR IOS)
    set(TGFX_USE_METAL ON CACHE INTERNAL "")
endif()

add_subdirectory(third_party/tgfx)


set(JSON_BuildTests OFF CACHE INTERNAL "")
set(JSON_Install OFF CACHE INTERNAL "")
add_subdirectory(third_party/tgfx/third_party/json)

# set(CMAKE_BUILD_TYPE "${_SEATCANVAS_PREV_BUILD_TYPE}" CACHE STRING "" FORCE)

option(SEATCANVAS_BUILD_TESTS "Build tests" OFF)
message(STATUS "SEATCANVAS_BUILD_TESTS: ${SEATCANVAS_BUILD_TESTS}")

add_subdirectory(src/SeatCanvas)

if(SEATCANVAS_BUILD_TESTS AND MACOS)
    enable_testing()
    set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(gtest_build_samples OFF CACHE BOOL "" FORCE)
    set(gtest_build_tests OFF CACHE BOOL "" FORCE)
    add_subdirectory(third_party/googletest)
    add_subdirectory(tests)
endif()
