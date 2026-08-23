// EnginotechC++ — Embedded Compiler Extensions Implementation

#include "embedded_ext.h"
#include <algorithm>

namespace eng {
namespace embedded_ext {

std::vector<BuiltInFn> getBuiltInFunctions(const std::string& target) {
    std::vector<BuiltInFn> fns;

    // Common functions for all embedded targets
    fns.push_back({"delay", "void", {{"ms", "int"}}, ""});
    fns.push_back({"millis", "int", {}, ""});
    fns.push_back({"micros", "int", {}, ""});
    fns.push_back({"pinMode", "void", {{"pin", "int"}, {"mode", "int"}}, ""});
    fns.push_back({"digitalWrite", "void", {{"pin", "int"}, {"val", "int"}}, ""});
    fns.push_back({"digitalRead", "int", {{"pin", "int"}}, ""});
    fns.push_back({"analogRead", "int", {{"pin", "int"}}, ""});
    fns.push_back({"analogWrite", "void", {{"pin", "int"}, {"val", "int"}}, ""});
    fns.push_back({"shiftOut", "void", {{"dataPin", "int"}, {"clockPin", "int"}, {"bitOrder", "int"}, {"value", "int"}}, ""});
    fns.push_back({"shiftIn", "int", {{"dataPin", "int"}, {"clockPin", "int"}, {"bitOrder", "int"}}, ""});
    fns.push_back({"tone", "void", {{"pin", "int"}, {"frequency", "int"}, {"duration", "int"}}, ""});
    fns.push_back({"noTone", "void", {{"pin", "int"}}, ""});
    fns.push_back({"pulseIn", "int", {{"pin", "int"}, {"state", "int"}, {"timeout", "int"}}, ""});
    fns.push_back({"attachInterrupt", "void", {{"pin", "int"}, {"callback", "fn"}, {"mode", "int"}}, ""});
    fns.push_back({"detachInterrupt", "void", {{"pin", "int"}}, ""});
    fns.push_back({"noInterrupts", "void", {}, ""});
    fns.push_back({"interrupts", "void", {}, ""});

    // ESP32-specific
    if (target == "esp32" || target.empty()) {
        fns.push_back({"wifiConnect", "bool", {{"ssid", "string"}, {"password", "string"}}, "esp32"});
        fns.push_back({"wifiConnected", "bool", {}, "esp32"});
        fns.push_back({"wifiIP", "string", {}, "esp32"});
        fns.push_back({"httpGet", "string", {{"url", "string"}}, "esp32"});
        fns.push_back({"httpPost", "string", {{"url", "string"}, {"body", "string"}}, "esp32"});
        fns.push_back({"espSleepMs", "void", {{"ms", "int"}}, "esp32"});
        fns.push_back({"espDeepSleepMs", "void", {{"ms", "int"}}, "esp32"});
        fns.push_back({"bleInit", "bool", {{"name", "string"}}, "esp32"});
    }

    // Arduino-specific
    if (target == "arduino" || target.empty()) {
        fns.push_back({"Serial", "SerialClass", {{"port", "int"}}, "arduino"});
    }

    return fns;
}

std::vector<BuiltInType> getBuiltInTypes(const std::string& target) {
    std::vector<BuiltInType> types;

    types.push_back({"OutputPin", "object", {"high", "low", "toggle", "read", "write"}, ""});
    types.push_back({"InputPin", "object", {"read", "setMode"}, ""});
    types.push_back({"PWMPin", "object", {"frequency", "duty", "stop", "start"}, ""});
    types.push_back({"ADCChannel", "object", {"read", "readAverage", "readVoltage", "setResolution"}, ""});
    types.push_back({"SerialPort", "object", {"begin", "end", "write", "read", "available", "flush"}, ""});
    types.push_back({"I2CBus", "object", {"begin", "end", "write", "read", "stop"}, "esp32"});
    types.push_back({"SPIBus", "object", {"begin", "end", "transfer", "transferBlock"}, ""});
    types.push_back({"Timer", "object", {"stop", "isRunning"}, ""});
    types.push_back({"WiFiStatus", "enum", {}, "esp32"});

    return types;
}

bool isBuiltInFunction(const std::string& name, const std::string& target) {
    auto fns = getBuiltInFunctions(target);
    return std::any_of(fns.begin(), fns.end(),
        [&name](const BuiltInFn& f) { return f.name == name; });
}

std::vector<std::string> getTargetDefines(const std::string& target, const std::string& board) {
    std::vector<std::string> defines;
    if (target == "arduino" || target == "avr") {
        defines.push_back("-DENG_TARGET_ARDUINO=1");
        if (board == "arduino-uno") defines.push_back("-DENG_TARGET_ARDUINO_UNO=1");
        else if (board == "arduino-nano") defines.push_back("-DENG_TARGET_ARDUINO_NANO=1");
        else if (board == "arduino-mega") defines.push_back("-DENG_TARGET_ARDUINO_MEGA=1");
    } else if (target == "esp32") {
        defines.push_back("-DENG_TARGET_ESP32=1");
        if (board == "esp32s3") defines.push_back("-DENG_TARGET_ESP32S3=1");
        else if (board == "esp32c3") defines.push_back("-DENG_TARGET_ESP32C3=1");
        else defines.push_back("-DENG_TARGET_ESP32=1");
    }
    defines.push_back("-DENG_EMBEDDED=1");
    return defines;
}

std::vector<std::string> getIncludePaths(const std::string& target) {
    (void)target;
    return {
        "std/embedded",
        "compiler/enginecore",
        "runtime/embedded"
    };
}

} // namespace embedded_ext
} // namespace eng
