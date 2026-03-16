cmake_minimum_required(VERSION 3.22...3.29)
project(SeatCanvas LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# cmake_policy(SET CMP0063 NEW)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

set(SeatCanvas_VERSION "1.0.0")

option(ENABLE_TIME_PROFILER "Enable time profiler" OFF)

if(MACOS OR IOS)
  enable_language(OBJC OBJCXX Swift)
  # Set up swiftrt.o and runtime library search paths
  include(InitializeSwift)
  # cmake/modules/AddSwift.cmake provides the function for creating the Swift to
  # C++ bridging header
  include(AddSwift)

  set(SeatCanvas_BUILD_FRAMEWORK "Enable building framework")
else()
  set(SeatCanvas_BUILD_FRAMEWORK OFF)
endif()


add_files_by_extension(SeatCanvas_CORE_FILES ".h;.hpp;.cpp"
    core
)

list(APPEND SeatCanvas_ALL_FILES ${SeatCanvas_CORE_FILES})
list(APPEND SeatCanvas_STATIC_LIBS tgfx)

if(IOS)
    message(STATUS "Platform: IOS")
    list(APPEND SeatCanvas_PRIVATE_HEADERS
        ${CMAKE_CURRENT_LIST_DIR}/platform/apple/swift/SwiftBridge.h
    )

    add_files_by_extension(SeatCanvas_PLATFORM_IOS_FILES ".h;.hpp;.m;.mm;.cpp;.swift"
        platform/apple
        platform/ios
        swift
    )
    list(APPEND SeatCanvas_ALL_FILES ${SeatCanvas_PLATFORM_IOS_FILES})
    list(APPEND SeatCanvas_ALL_FILES ${SeatCanvas_PRIVATE_HEADERS})
    find_library(UIKit_LIBS UIKit REQUIRED)
    list(APPEND SeatCanvas_SHARED_LIBS ${UIKit_LIBS})
    find_library(Foundation_LIBS Foundation REQUIRED)
    list(APPEND SeatCanvas_SHARED_LIBS ${Foundation_LIBS})
    find_library(QuartzCore_LIBS QuartzCore REQUIRED)
    list(APPEND SeatCanvas_SHARED_LIBS ${QuartzCore_LIBS})
    find_library(CoreGraphics_LIBS CoreGraphics REQUIRED)
    list(APPEND SeatCanvas_SHARED_LIBS ${CoreGraphics_LIBS})
    find_library(CoreText_LIBS CoreText REQUIRED)
    list(APPEND SeatCanvas_SHARED_LIBS ${CoreText_LIBS})
    find_library(CoreMedia_LIBS CoreMedia REQUIRED)
    list(APPEND SeatCanvas_SHARED_LIBS ${CoreMedia_LIBS})
    find_library(ImageIO_LIBS ImageIO REQUIRED)
    list(APPEND SeatCanvas_SHARED_LIBS ${ImageIO_LIBS})
    find_library(CORE_VIDEO CoreVideo)
    list(APPEND SeatCanvas_SHARED_LIBS ${CORE_VIDEO})
    find_library(ICONV_LIBRARIES NAMES iconv libiconv libiconv-2 c)
    list(APPEND SeatCanvas_SHARED_LIBS ${ICONV_LIBRARIES})
    find_library(OpenGLES_LIBS OpenGLES REQUIRED)
    list(APPEND SeatCanvas_SHARED_LIBS ${OpenGLES_LIBS})

    list(APPEND SeatCanvas_LDS ${CMAKE_CURRENT_LIST_DIR}/SeatCanvas.ios.lds)
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -exported_symbols_list ${SeatCanvas_LDS}")
    add_source_group(SeatCanvas_LDS ${CMAKE_CURRENT_LIST_DIR} "Linker Script")
elseif (ANDROID)
    message(STATUS "Platform: ANDROID")
    add_files_by_extension(SeatCanvas_PLATFORM_ANDROID_FILES ".h;.hpp;.cpp"
       platform/android
    )
    list(APPEND SeatCanvas_ALL_FILES ${SeatCanvas_PLATFORM_ANDROID_FILES})

    find_library(LOG_LIB log)
    find_library(ANDROID_LIB android)
    list(APPEND SeatCanvas_SHARED_LIBS ${LOG_LIB} ${ANDROID_LIB})
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-z,max-page-size=16384 -Wl,--gc-sections -Wl,--version-script=${CMAKE_CURRENT_LIST_DIR}/SeatCanvas.android.def")
elseif (OHOS)
    message(STATUS "Platform: OHOS")
    add_files_by_extension(SeatCanvas_PLATFORM_OHOS_FILES ".h;.hpp;.cpp"
       platform/ohos
    )
    list(APPEND SeatCanvas_ALL_FILES ${SeatCanvas_PLATFORM_OHOS_FILES})

    find_library(GLESV3_LIB GLESv3)
    find_library(EGL_LIB EGL)
    find_library(ACE_NDK_LIB ace_ndk.z)
    find_library(HILOG_NDK_LIB hilog_ndk.z)
    find_library(ACE_NAPI_LIB ace_napi.z)
    find_library(NATIVE_DISPLAY_SOLOIST_LIB native_display_soloist)
    find_library(NATIVE_RAWFILE_Z_LIB rawfile.z)
    list(APPEND SeatCanvas_SHARED_LIBS
        ${GLESV3_LIB}
        ${EGL_LIB}
        ${ACE_NDK_LIB}
        ${HILOG_NDK_LIB}
        ${ACE_NAPI_LIB}
        ${NATIVE_DISPLAY_SOLOIST_LIB}
        ${NATIVE_RAWFILE_Z_LIB}
    )
    
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--gc-sections -Wl,--version-script=${CMAKE_CURRENT_LIST_DIR}/SeatCanvas.ohos.def")

endif()

add_source_group(SeatCanvas_ALL_FILES ${CMAKE_CURRENT_LIST_DIR} "Sources")

set(LIB_TARGET_NAME ${PROJECT_NAME})
if (OHOS)
    string(TOLOWER "${PROJECT_NAME}" PROJECT_NAME_LOWER)
    set(LIB_TARGET_NAME ${PROJECT_NAME_LOWER})
endif()

add_library(${LIB_TARGET_NAME} SHARED ${SeatCanvas_ALL_FILES} ${SeatCanvas_LDS})

target_include_directories(${LIB_TARGET_NAME} PRIVATE ${PROJECT_ROOT_INCLUDE_DIR}/${PROJECT_NAME})
target_include_directories(${LIB_TARGET_NAME} PRIVATE ${PROJECT_THIRD_PARTY_ROOT_DIR}/tgfx/include ${PROJECT_THIRD_PARTY_ROOT_DIR}/tgfx/src)
target_include_directories(${LIB_TARGET_NAME} PRIVATE .)
target_link_libraries(${LIB_TARGET_NAME} PRIVATE 
    ${SeatCanvas_STATIC_LIBS} 
    ${SeatCanvas_SHARED_LIBS}
    nlohmann_json
)

target_compile_definitions(${LIB_TARGET_NAME} PRIVATE $<$<CONFIG:Debug>:DEBUG>)
target_compile_definitions(${LIB_TARGET_NAME} PRIVATE $<$<CONFIG:RelWithDebInfo>:DEBUG>)
target_compile_definitions(${LIB_TARGET_NAME} PRIVATE
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:ENABLE_TIME_PROFILER=$<BOOL:${ENABLE_TIME_PROFILER}>>
)

# for debug symbol
target_compile_options(${LIB_TARGET_NAME} PRIVATE -g)
target_compile_options(${LIB_TARGET_NAME} PRIVATE
    $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:-O0>
    $<$<CONFIG:Release>:-Os>
    $<$<CONFIG:MinSizeRel>:-Os>
)

# tgfx
target_compile_options(${LIB_TARGET_NAME} PRIVATE
    $<$<COMPILE_LANGUAGE:C,CXX,OBJC,OBJCXX>:
        -ffunction-sections
        -fdata-sections
    >
    $<$<COMPILE_LANGUAGE:CXX,OBJCXX>:
        -fno-exceptions
        -fno-rtti
    >
)

if(MACOS OR IOS)
   
    set_target_properties(${LIB_TARGET_NAME} PROPERTIES
      XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC "YES"
      XCODE_ATTRIBUTE_SWIFT_OBJC_INTEROP_MODE "objcxx"
    #   XCODE_ATTRIBUTE_MODULE_VERIFIER_SUPPORTED_LANGUAGES "c objective-c c++ objective-c++"
    #   XCODE_ATTRIBUTE_MODULE_VERIFIER_SUPPORTED_LANGUAGE_STANDARDS "gnu11 gnu++17"
      XCODE_ATTRIBUTE_DEPLOYMENT_POSTPROCESSING "YES"
      XCODE_ATTRIBUTE_DEPLOYMENT_POSTPROCESSING[variant=Debug] "NO"
      XCODE_ATTRIBUTE_COPY_PHASE_STRIP "YES"
      XCODE_ATTRIBUTE_STRIP_INSTALLED_PRODUCT "YES"
      XCODE_ATTRIBUTE_STRIP_STYLE "non-global"
      XCODE_ATTRIBUTE_SWIFT_VERSION "6.0"
      XCODE_ATTRIBUTE_BUILD_LIBRARY_FOR_DISTRIBUTION "YES"
      XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS "YES"
      XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym"
      XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT[variant=Debug] "dwarf"
      XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT[variant=RelWithDebInfo] "dwarf"
    )

    target_compile_options(${LIB_TARGET_NAME} PRIVATE
        "$<$<COMPILE_LANGUAGE:Swift>:-package-name;${LIB_TARGET_NAME}>"
        "$<$<COMPILE_LANGUAGE:Swift>:-cxx-interoperability-mode=default>"
        "$<$<COMPILE_LANGUAGE:Swift>:-enable-library-evolution>"
        "$<$<COMPILE_LANGUAGE:Swift>:-emit-module-interface>"
    )
    
    _swift_generate_cxx_header_from_sources(${LIB_TARGET_NAME} 
        ${LIB_TARGET_NAME}-Bridge-swift.h
        SWIFT_SOURCES
            "${CMAKE_CURRENT_LIST_DIR}/platform/apple/swift/SeatCanvasRendererDelegate.swift"
            "${CMAKE_CURRENT_LIST_DIR}/platform/apple/swift/SeatCanvasRendererDelegateRegistry.swift"
            "${CMAKE_CURRENT_LIST_DIR}/platform/apple/swift/SwiftBridge.swift"
        MODULE_NAME
            "${LIB_TARGET_NAME}"
        SEARCH_PATHS 
            "${CMAKE_CURRENT_LIST_DIR}"
    )
endif()

if (SeatCanvas_BUILD_FRAMEWORK)
    set(SeatCanvas_IDENTIFIER "com.0x1306a94.seatcanvas")
    set_target_properties(${LIB_TARGET_NAME} PROPERTIES
        FRAMEWORK TRUE
        INSTALL_NAME_DIR "@rpath/${PROJECT_NAME}.framework/${PROJECT_NAME}"
        INSTALL_RPATH "@executable_path/Frameworks;@loader_path/Frameworks"
        XCODE_ATTRIBUTE_FRAMEWORK_VERSION A
        XCODE_ATTRIBUTE_WRAPPER_EXTENSION "framework"
        XCODE_ATTRIBUTE_DEFINES_MODULE "YES"
        XCODE_ATTRIBUTE_CLANG_ENABLE_MODULES "YES"
        # XCODE_ATTRIBUTE_MODULEMAP_FILE "${CMAKE_CURRENT_LIST_DIR}/module.modulemap"
        XCODE_ATTRIBUTE_MODULEMAP_PRIVATE_FILE "${CMAKE_CURRENT_LIST_DIR}/module.private.modulemap"
        XCODE_ATTRIBUTE_SKIP_INSTALL "YES"
        XCODE_ATTRIBUTE_INSTALL_PATH "@rpath"
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER ${SeatCanvas_IDENTIFIER}
        MACOSX_FRAMEWORK_IDENTIFIER ${SeatCanvas_IDENTIFIER}
        MACOSX_FRAMEWORK_BUNDLE_VERSION ${SeatCanvas_VERSION}
        MACOSX_FRAMEWORK_SHORT_VERSION_STRING ${SeatCanvas_VERSION}
        MACOSX_FRAMEWORK_BUNDLE_NAME ${LIB_TARGET_NAME}
    )

    foreach (header ${SeatCanvas_PRIVATE_HEADERS})
        file(RELATIVE_PATH dir ${CMAKE_CURRENT_LIST_DIR}/ ${header})
        get_filename_component(dir "${dir}" DIRECTORY)
        message(STATUS "PRIVATE_HEADERS: ${dir} ${header}")
        if (dir)
            set(location "PrivateHeaders/${dir}")
        else ()
            set(location "PrivateHeaders")
        endif ()
        set_source_files_properties(${header} PROPERTIES MACOSX_PACKAGE_LOCATION "${location}")
    endforeach ()

endif ()
