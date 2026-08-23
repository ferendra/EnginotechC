cmake_minimum_required(VERSION 3.16)
project(enginotech VERSION 0.2.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(COMMON_SOURCES
    compiler/lexer/token.h
    compiler/lexer/lexer.h
    compiler/lexer/lexer.cpp
    compiler/ast/nodes.h
    compiler/diagnostics/diagnostic.h
    compiler/parser/parser.h
    compiler/parser/parser.cpp
    compiler/semantic/semantic.h
    compiler/semantic/semantic.cpp
    compiler/types/typechecker.h
    compiler/types/typechecker.cpp
    compiler/codegen/irgen.h
    compiler/codegen/irgen.cpp
    compiler/codegen/caller.h
    compiler/codegen/caller.cpp
    compiler/driver/main.cpp
    # Build system
    build-system/build.h
    build-system/build.cpp
    # Runtime
    runtime/runtime.h
    runtime/runtime.cpp
    # Std library
    std/io/io.h
    std/io/io.cpp
    std/collections/collections.h
    std/math/math.h
    std/math/math.cpp
    std/string/string.h
    std/string/string.cpp
    std/json/json.h
    std/json/json.cpp
    std/fs/fs.h
    std/fs/fs.cpp
    std/testing/testing.h
    std/testing/testing.cpp
    # Formatter
    formatter/formatter.h
    formatter/formatter.cpp
    # Linter
    linter/linter.h
    linter/linter.cpp
    # Embedded HAL
    std/embedded/gpio/gpio.h
    std/embedded/gpio/gpio.cpp
    std/embedded/pwm/pwm.h
    std/embedded/pwm/pwm.cpp
    std/embedded/adc/adc.h
    std/embedded/adc/adc.cpp
    std/embedded/uart/uart.h
    std/embedded/uart/uart.cpp
    std/embedded/i2c/i2c.h
    std/embedded/i2c/i2c.cpp
    std/embedded/spi/spi.h
    std/embedded/spi/spi.cpp
    std/embedded/timer/timer.h
    std/embedded/timer/timer.cpp
    std/embedded/interrupt/interrupt.h
    std/embedded/interrupt/interrupt.cpp
    std/embedded/wifi/wifi.h
    std/embedded/wifi/wifi.cpp
    std/embedded/bluetooth/bt.h
    std/embedded/bluetooth/bt.cpp
    std/embedded/sleep/sleep.h
    std/embedded/sleep/sleep.cpp
    std/embedded/system/system.h
    std/embedded/system/system.cpp
    std/embedded/embedded.h
    # Embedded runtime
    runtime/embedded/embedded_rt.h
    runtime/embedded/embedded_rt.cpp
    # Embedded target backends
    compiler/targets/target.h
    compiler/targets/target.cpp
    compiler/targets/avr.h
    compiler/targets/avr.cpp
    compiler/targets/esp32.h
    compiler/targets/esp32.cpp
    compiler/targets/x86.h
    compiler/targets/x86.cpp
    compiler/targets/baremetal.h
    compiler/targets/baremetal.cpp
    compiler/targets/ecconfig.h
    compiler/targets/ecconfig.cpp
    compiler/targets/embedded_ext.h
    compiler/targets/embedded_ext.cpp
    # Engine core
    compiler/enginecore/enginecore.h
    compiler/enginecore/enginecore.cpp
    # Embedded CLI commands
    compiler/driver/embedded.h
    compiler/driver/embedded.cpp
    # Package ecosystem
    packages/registry/registry.h
    packages/registry/registry.cpp
    packages/manager/manager.h
    packages/manager/manager.cpp
    # Cross-platform abstraction
    compiler/platform/platform.h
    compiler/platform/platform_common.cpp
    # Sensor packages
    std/embedded/packages/dht/dht.h
    std/embedded/packages/dht/dht.cpp
    std/embedded/packages/bme280/bme280.h
    std/embedded/packages/bme280/bme280.cpp
    std/embedded/packages/mpu6050/mpu6050.h
    std/embedded/packages/mpu6050/mpu6050.cpp
    std/embedded/packages/ssd1306/ssd1306.h
    std/embedded/packages/ssd1306/ssd1306.cpp
    std/embedded/packages/ws2812/ws2812.h
    std/embedded/packages/ws2812/ws2812.cpp
    std/embedded/packages/servo/servo.h
    std/embedded/packages/servo/servo.cpp
    std/embedded/packages/relay/relay.h
    std/embedded/packages/relay/relay.cpp
    std/embedded/packages/ultrasonic/ultrasonic.h
    std/embedded/packages/ultrasonic/ultrasonic.cpp
    std/embedded/packages/rfid/rfid.h
    std/embedded/packages/rfid/rfid.cpp
    std/embedded/packages/sdcards/sdcards.h
    std/embedded/packages/sdcards/sdcards.cpp
    std/embedded/packages/rtc/rtc.h
    std/embedded/packages/rtc/rtc.cpp
    std/embedded/packages/gps/gps.h
    std/embedded/packages/gps/gps.cpp
    std/embedded/packages/mqtt/mqtt.h
    std/embedded/packages/mqtt/mqtt.cpp
    std/embedded/packages/webserver/webserver.h
    std/embedded/packages/webserver/webserver.cpp
    std/embedded/packages/storage/storage.h
    std/embedded/packages/storage/storage.cpp
    std/embedded/packages/power/power.h
    std/embedded/packages/power/power.cpp
    # GUI library (excluding legacy widgets.cpp which conflicts with new hierarchy)
    gui/theme.h
    gui/theme.cpp
    gui/renderer.h
    gui/renderer.cpp
    gui/events.h
    gui/events.cpp
    gui/widget.h
    gui/widget.cpp
    gui/button.h
    gui/button.cpp
    gui/label.h
    gui/label.cpp
    gui/textbox.h
    gui/textbox.cpp
    gui/slider.h
    gui/slider.cpp
    gui/window.h
    gui/driver/cgui_driver.c
    # ETC skeleton target
    compiler/targets/etskeleton.h
    compiler/targets/etskeleton.cpp
)

# Build the compiler executable (no LLVM C++ API needed)
add_executable(engc ${COMMON_SOURCES})

# Compiler flags
target_compile_options(engc PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /EHsc /std:c++20>
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -std=c++20>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -std=c++20>
)

message(STATUS "Build configuration:")
message(STATUS "  C++ Standard: 20")
message(STATUS "  Codegen: LLVM IR text → clang")
