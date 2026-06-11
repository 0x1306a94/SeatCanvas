cmake_minimum_required(VERSION 3.22...3.29)

if(MACOS)
    enable_language(OBJCXX)
endif()

add_files_by_extension(SEATCANVAS_FULL_TEST_FILES ".h;.hpp;.cpp;.mm"
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_source_group(SEATCANVAS_FULL_TEST_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src "Sources")

add_executable(SeatCanvasFullTests ${SEATCANVAS_FULL_TEST_FILES})

target_include_directories(SeatCanvasFullTests PRIVATE
    ${PROJECT_ROOT_INCLUDE_DIR}/SeatCanvas
    ${PROJECT_THIRD_PARTY_ROOT_DIR}/tgfx/include
    ${PROJECT_THIRD_PARTY_ROOT_DIR}/tgfx/third_party/json/include
    ${PROJECT_SOURCE_DIR}/src/SeatCanvas
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils
)

target_link_libraries(SeatCanvasFullTests PRIVATE
    SeatCanvas
    tgfx
    gtest
)

set_target_properties(SeatCanvasFullTests PROPERTIES MACOSX_BUNDLE FALSE)
seatcanvas_enable_objc_arc(SeatCanvasFullTests)

# Match SeatCanvas framework ABI (-fno-rtti/-fno-exceptions) so test subclasses of
# PlatformView do not require base-class typeinfo from the dylib.
target_compile_options(SeatCanvasFullTests PRIVATE
    $<$<COMPILE_LANGUAGE:CXX,OBJCXX>:-fno-exceptions -fno-rtti>
)

set(SEATCANVAS_BACKEND_NAME "metal")
set(SEATCANVAS_CACHE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/baseline/.cache/${SEATCANVAS_BACKEND_NAME})
file(MAKE_DIRECTORY ${SEATCANVAS_CACHE_DIR})
execute_process(COMMAND git rev-parse --short HEAD
    OUTPUT_VARIABLE SEATCANVAS_GIT_HEAD
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
file(WRITE ${SEATCANVAS_CACHE_DIR}/HEAD "${SEATCANVAS_GIT_HEAD}")

list(APPEND SEATCANVAS_TEST_DEFINES SEATCANVAS_BACKEND_NAME="${SEATCANVAS_BACKEND_NAME}")
target_compile_definitions(SeatCanvasFullTests PRIVATE ${SEATCANVAS_TEST_DEFINES})

add_executable(SeatCanvasUpdateBaseline ${SEATCANVAS_FULL_TEST_FILES})
target_include_directories(SeatCanvasUpdateBaseline PRIVATE
    ${PROJECT_ROOT_INCLUDE_DIR}/SeatCanvas
    ${PROJECT_THIRD_PARTY_ROOT_DIR}/tgfx/include
    ${PROJECT_THIRD_PARTY_ROOT_DIR}/tgfx/third_party/json/include
    ${PROJECT_SOURCE_DIR}/src/SeatCanvas
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/base
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils
)
target_link_libraries(SeatCanvasUpdateBaseline PRIVATE SeatCanvas tgfx gtest)
set_target_properties(SeatCanvasUpdateBaseline PROPERTIES MACOSX_BUNDLE FALSE)
seatcanvas_enable_objc_arc(SeatCanvasUpdateBaseline)
target_compile_options(SeatCanvasUpdateBaseline PRIVATE
    $<$<COMPILE_LANGUAGE:CXX,OBJCXX>:-fno-exceptions -fno-rtti>
)
target_compile_definitions(SeatCanvasUpdateBaseline PRIVATE
    ${SEATCANVAS_TEST_DEFINES}
    UPDATE_BASELINE
)

if(SEATCANVAS_ENABLE_COVERAGE AND MACOS)
    target_compile_options(SeatCanvas PRIVATE
        $<$<COMPILE_LANGUAGE:CXX,OBJCXX>:-fprofile-instr-generate -fcoverage-mapping>
    )
    target_link_options(SeatCanvas PRIVATE "LINKER:-undefined,dynamic_lookup")

    execute_process(COMMAND xcrun --sdk macosx clang -print-file-name=libclang_rt.profile_osx.a
        OUTPUT_VARIABLE CLANG_RT_PROFILE_LIB
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(EXISTS "${CLANG_RT_PROFILE_LIB}")
        target_link_libraries(SeatCanvas PRIVATE "${CLANG_RT_PROFILE_LIB}")
    endif()

    target_compile_options(SeatCanvasFullTests PRIVATE -fprofile-instr-generate -fcoverage-mapping)
    target_link_options(SeatCanvasFullTests PRIVATE -fprofile-instr-generate)
endif()
