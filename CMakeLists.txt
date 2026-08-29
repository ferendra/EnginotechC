cmake_minimum_required(VERSION 3.16)
project(enginotech VERSION 0.5.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build types
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Version from VERSION file
file(STRINGS VERSION _version LIMIT_COUNT 1 REGEX "^[0-9]")
string(STRIP "${_version}" ENG_VERSION)
message(STATUS "EnginotechC++ Version: ${ENG_VERSION}")

# Options
option(BUILD_TESTS "Build tests" ON)
option(BUILD_TOOLS "Build auxiliary tools" ON)
option(BUILD_STDLIB "Build standard library" ON)
option(BUILD_EMBEDDED "Build embedded targets (AVR, ESP32, baremetal)" ON)
option(BUILD_GUI "Build GUI library" OFF)
option(BUILD_LSP "Build Language Server" ON)
option(USE_SYSTEM_LLVM "Use system LLVM instead of bundled" OFF)
option(ENABLE_SANITIZERS "Enable Address/Undefined sanitizers (Debug only)" OFF)

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(BASE_CXX_FLAGS "/W4 /EHsc /std:c++20 /permissive-")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(BASE_CXX_FLAGS "${BASE_CXX_FLAGS} /MDd /Zi /Ob0 /Od")
    else()
        set(BASE_CXX_FLAGS "${BASE_CXX_FLAGS} /MD /O2 /Ob2")
    endif()
else()
    set(BASE_CXX_FLAGS "-Wall -Wextra -Wpedantic -Wshadow -Wconversion -std=c++20 -pipe")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(BASE_CXX_FLAGS "${BASE_CXX_FLAGS} -g -O0 -fno-omit-frame-pointer")
    elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
        set(BASE_CXX_FLAGS "${BASE_CXX_FLAGS} -O2 -g")
    else()
        set(BASE_CXX_FLAGS "${BASE_CXX_FLAGS} -O3 -march=native -flto")
    endif()
endif()

if(ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        set(BASE_CXX_FLAGS "${BASE_CXX_FLAGS} -fsanitize=address,undefined -fno-omit-frame-pointer")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address,undefined")
    endif()
endif()

add_compile_options(${BASE_CXX_FLAGS})

# Definitions
add_compile_definitions(
    ENG_VERSION="${ENG_VERSION}"
    BUILD_TYPE="${CMAKE_BUILD_TYPE}"
)

# Find LLVM (for cross-compilation targets)
if(USE_SYSTEM_LLVM)
    find_package(LLVM REQUIRED CONFIG)
    message(STATUS "Found LLVM: ${LLVM_VERSION}")
    include_directories(${LLVM_INCLUDE_DIRS})
    list(APPEND LLVM_DEFINITIONS ${LLVM_DEFINITIONS})
    add_compile_definitions(${LLVM_DEFINITIONS})
endif()

# Platform detection
if(WIN32)
    add_compile_definitions(_WIN32_WINNT=0x0A00 WIN32_LEAN_AND_MEAN NOMINMAX)
    if(BUILD_EMBEDDED)
        find_library(WS2_32 ws2_32)
        set(PLATFORM_LIBS ${PLATFORM_LIBS} ${WS2_32})
    endif()
elseif(APPLE)
    add_compile_definitions(__APPLE__)
elseif(UNIX)
    add_compile_definitions(_GNU_SOURCE)
    set(PLATFORM_LIBS ${PLATFORM_LIBS} pthread dl m)
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/compiler
    ${CMAKE_CURRENT_SOURCE_DIR}/std
    ${CMAKE_CURRENT_SOURCE_DIR}/runtime
)

# Helper function to add module
function(add_module name)
    file(GLOB_RECURSE ${name}_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/${name}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/${name}/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/${name}/*.hpp"
    )
    list(FILTER ${name}_SOURCES EXCLUDE REGEX ".*/tests/.*")
    if(${name}_SOURCES)
        add_library(${name} STATIC ${${name}_SOURCES})
        target_include_directories(${name} PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/${name}
            ${CMAKE_CURRENT_SOURCE_DIR}
        )
        message(STATUS "Module: ${name} (${${name}_SOURCES})")
    else()
        message(WARNING "Module ${name}: no sources found")
    endif()
endfunction()

# ========================================================================
# Core Modules
# ========================================================================

# Compiler modules
add_module(compiler/lexer)
add_module(compiler/ast)
add_module(compiler/diagnostics)
add_module(compiler/parser)
add_module(compiler/semantic)
add_module(compiler/types)
add_module(compiler/codegen)
add_module(compiler/driver)
add_module(compiler/targets)
add_module(compiler/enginecore)
add_module(compiler/platform)
add_module(compiler/repl)

# Runtime
add_module(runtime)

# Build system
add_module(build-system)

# Stdlib modules
if(BUILD_STDLIB)
    add_module(std/io)
    add_module(std/collections)
    add_module(std/math)
    add_module(std/string)
    add_module(std/json)
    add_module(std/fs)
    add_module(std/testing)
    add_module(std/embedded)
    add_module(std/embedded/gpio)
    add_module(std/embedded/pwm)
    add_module(std/embedded/adc)
    add_module(std/embedded/uart)
    add_module(std/embedded/i2c)
    add_module(std/embedded/spi)
    add_module(std/embedded/timer)
    add_module(std/embedded/interrupt)
    add_module(std/embedded/wifi)
    add_module(std/embedded/bluetooth)
    add_module(std/embedded/sleep)
    add_module(std/embedded/system)
    add_module(std/embedded/packages/dht)
    add_module(std/embedded/packages/bme280)
    add_module(std/embedded/packages/mpu6050)
    add_module(std/embedded/packages/ssd1306)
    add_module(std/embedded/packages/ws2812)
    add_module(std/embedded/packages/servo)
    add_module(std/embedded/packages/relay)
    add_module(std/embedded/packages/ultrasonic)
    add_module(std/embedded/packages/rfid)
    add_module(std/embedded/packages/sdcards)
    add_module(std/embedded/packages/rtc)
    add_module(std/embedded/packages/gps)
    add_module(std/embedded/packages/mqtt)
    add_module(std/embedded/packages/webserver)
    add_module(std/embedded/packages/storage)
    add_module(std/embedded/packages/power)
endif()

# Tools
if(BUILD_TOOLS)
    add_module(formatter)
    add_module(linter)
endif()

if(BUILD_LSP)
    add_module(langserver)
endif()

# Embedded targets
if(BUILD_EMBEDDED)
    add_module(compiler/targets/avr)
    add_module(compiler/targets/esp32)
    add_module(compiler/targets/x86)
    add_module(compiler/targets/baremetal)
    add_module(compiler/targets/etskeleton)
endif()

# GUI
if(BUILD_GUI)
    add_module(gui)
endif()

# Package manager
add_module(packages/registry)
add_module(packages/manager)

# ========================================================================
# Main Executable
# ========================================================================

# Collect all module libraries
set(ALL_MODULES
    compiler_lexer
    compiler_ast
    compiler_diagnostics
    compiler_parser
    compiler_semantic
    compiler_types
    compiler_codegen
    compiler_driver
    compiler_targets
    compiler_enginecore
    compiler_platform
    compiler_repl
    runtime
    build_system
)

if(BUILD_STDLIB)
    list(APPEND ALL_MODULES
        std_io
        std_collections
        std_math
        std_string
        std_json
        std_fs
        std_testing
        std_embedded
        std_embedded_gpio
        std_embedded_pwm
        std_embedded_adc
        std_embedded_uart
        std_embedded_i2c
        std_embedded_spi
        std_embedded_timer
        std_embedded_interrupt
        std_embedded_wifi
        std_embedded_bluetooth
        std_embedded_sleep
        std_embedded_system
        std_embedded_packages_dht
        std_embedded_packages_bme280
        std_embedded_packages_mpu6050
        std_embedded_packages_ssd1306
        std_embedded_packages_ws2812
        std_embedded_packages_servo
        std_embedded_packages_relay
        std_embedded_packages_ultrasonic
        std_embedded_packages_rfid
        std_embedded_packages_sdcards
        std_embedded_packages_rtc
        std_embedded_packages_gps
        std_embedded_packages_mqtt
        std_embedded_packages_webserver
        std_embedded_packages_storage
        std_embedded_packages_power
    )
endif()

if(BUILD_TOOLS)
    list(APPEND ALL_MODULES formatter linter)
endif()

if(BUILD_LSP)
    list(APPEND ALL_MODULES langserver)
endif()

if(BUILD_EMBEDDED)
    list(APPEND ALL_MODULES
        compiler_targets_avr
        compiler_targets_esp32
        compiler_targets_x86
        compiler_targets_baremetal
        compiler_targets_etskeleton
    )
endif()

if(BUILD_GUI)
    list(APPEND ALL_MODULES gui)
endif()

list(APPEND ALL_MODULES packages_registry packages_manager)

# Main executable
add_executable(engc
    compiler/driver/main.cpp
)

target_link_libraries(engc PRIVATE ${ALL_MODULES} ${PLATFORM_LIBS})

if(USE_SYSTEM_LLVM)
    target_link_libraries(engc PRIVATE ${LLVM_LIBRARIES})
endif()

# Install
install(TARGETS engc
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION bin
)

# Stdlib install
if(BUILD_STDLIB)
    install(DIRECTORY std/
        DESTINATION share/enginotech/std
        FILES_MATCHING PATTERN "*.ec" PATTERN "*.h" PATTERN "*.cpp"
    )
endif()

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Packaging
set(CPACK_PACKAGE_NAME "EnginotechC++")
set(CPACK_PACKAGE_VERSION ${ENG_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Modern systems programming language")
set(CPACK_PACKAGE_VENDOR "Enginotech")
set(CPACK_GENERATOR "TGZ;ZIP")
include(CPack)

message(STATUS "Build configuration:")
message(STATUS "  Version: ${ENG_VERSION}")
message(STATUS "  C++ Standard: 20")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Modules: ${ALL_MODULES}")