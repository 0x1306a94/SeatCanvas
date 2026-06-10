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

set(CACHE_DIR ${PROJECT_SOURCE_DIR}/tests/baseline/.cache)
file(MAKE_DIRECTORY ${CACHE_DIR})
execute_process(COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE HEAD_COMMIT
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)
if(HEAD_COMMIT)
    file(WRITE ${CACHE_DIR}/HEAD "${HEAD_COMMIT}")
endif()
