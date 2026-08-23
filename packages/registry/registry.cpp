// EnginotechC++ — Embedded Package Registry Implementation
// Complete package metadata for all embedded ecosystem packages

#include "registry.h"
#include <algorithm>
#include <sstream>

namespace eng {
namespace packages {

// ======================== DHT ========================
static PackageInfo pkg_dht = {
    "dht", "1.0.0",
    "DHT11/DHT22 temperature and humidity sensor driver",
    "engtech-team",
    {"sensor", "temperature", "humidity", "i2c-none", "gpio"},
    {},
    "MIT",
    true, true, false,
    "std/embedded/packages/dht",
    "https://github.com/adafruit/DHT-sensor-library",
    "Usage: import dht; let s = dht.connect(4, dht.Type::DHT22); let temp = s.readTemperature();"
};

// ======================== BME280 ========================
static PackageInfo pkg_bme280 = {
    "bme280", "1.0.0",
    "BME280 barometric pressure, temperature, and humidity sensor (I2C/SPI)",
    "engtech-team",
    {"sensor", "pressure", "temperature", "humidity", "i2c"},
    {"i2c"},
    "MIT",
    true, true, false,
    "std/embedded/packages/bme280",
    "https://github.com/adafruit/BME280",
    "Usage: import bme280; let s = bme280.open(i2c_bus, 0x76); let p = s.readPressure();"
};

// ======================== BMP280 ========================
static PackageInfo pkg_bmp280 = {
    "bmp280", "1.0.0",
    "BMP280 barometric pressure and temperature sensor (I2C/SPI)",
    "engtech-team",
    {"sensor", "pressure", "temperature", "i2c"},
    {"i2c"},
    "MIT",
    true, true, false,
    "std/embedded/packages/bmp280",
    "https://github.com/adafruit/BMP280",
    "Usage: import bmp280; let s = bmp280.open(i2c_bus, 0x76);"
};

// ======================== MPU6050 ========================
static PackageInfo pkg_mpu6050 = {
    "mpu6050", "1.0.0",
    "MPU6050 6-axis gyroscope and accelerometer (I2C)",
    "engtech-team",
    {"sensor", "accelerometer", "gyroscope", "i2c", "motion"},
    {"i2c"},
    "MIT",
    true, true, false,
    "std/embedded/packages/mpu6050",
    "https://github.com/chrisajj/MPU6050",
    "Usage: import mpu6050; let imu = mpu6050.open(i2c_bus, 0x68); let acc = imu.readAccel();"
};

// ======================== SSD1306 OLED ========================
static PackageInfo pkg_ssd1306 = {
    "ssd1306", "1.0.0",
    "SSD1306 monochrome OLED display driver (I2C/SPI)",
    "engtech-team",
    {"display", "oled", "i2c", "spi", "graphics"},
    {"i2c", "spi"},
    "MIT",
    true, true, false,
    "std/embedded/packages/ssd1306",
    "https://github.com/adafruit/Adafruit_SSD1306",
    "Usage: import ssd1306; let display = ssd1306.open(i2c_bus, 0x3C, 128, 64); display.print(\"Hello!\");"
};

// ======================== WS2812 NeoPixel ========================
static PackageInfo pkg_ws2812 = {
    "ws2812", "1.0.0",
    "WS2812B/NeoPixel addressable LED strip driver",
    "engtech-team",
    {"led", "neopixel", "pwm", "gpio"},
    {},
    "MIT",
    true, true, false,
    "std/embedded/packages/ws2812",
    "https://github.com/adafruit/Adafruit_NeoPixel",
    "Usage: import ws2812; let leds = ws2812.open(2, 30); leds.setPixel(0, 255, 0, 0);"
};

// ======================== Servo ========================
static PackageInfo pkg_servo = {
    "servo", "1.0.0",
    "Servo motor control via PWM",
    "engtech-team",
    {"motor", "servo", "pwm"},
    {},
    "MIT",
    true, true, false,
    "std/embedded/packages/servo",
    "https://github.com/adafruit/Adafruit_ServoLibrary",
    "Usage: import servo; let s = servo.attach(9); s.write(90);"
};

// ======================== Relay ========================
static PackageInfo pkg_relay = {
    "relay", "1.0.0",
    "Digital relay module control",
    "engtech-team",
    {"relay", "gpio", "switch"},
    {},
    "MIT",
    true, true, false,
    "std/embedded/packages/relay",
    "",
    "Usage: import relay; let r = relay.connect(5); r.on(); delay(1000); r.off();"
};

// ======================== Ultrasonic HC-SR04 ========================
static PackageInfo pkg_ultrasonic = {
    "ultrasonic", "1.0.0",
    "HC-SR04 ultrasonic distance sensor",
    "engtech-team",
    {"sensor", "distance", "ultrasonic", "gpio"},
    {},
    "MIT",
    true, true, false,
    "std/embedded/packages/ultrasonic",
    "",
    "Usage: import ultrasonic; let u = ultrasonic.connect(trig_pin, echo_pin); let dist = u.distance();"
};

// ======================== SD Card ========================
static PackageInfo pkg_sdcard = {
    "sdcards", "1.0.0",
    "SD card filesystem access via SPI",
    "engtech-team",
    {"storage", "sdcard", "spi", "filesystem"},
    {"spi"},
    "MIT",
    true, true, false,
    "std/embedded/packages/sdcards",
    "https://github.com/adafruit/SD",
    "Usage: import sdcards; let fs = sdcards.open(SPI, CS_PIN); fs.write(\"data.txt\", \"hello\");"
};

// ======================== RFID ========================
static PackageInfo pkg_rfid = {
    "rfid", "1.0.0",
    "MFRC522 RFID card reader (SPI)",
    "engtech-team",
    {"rfid", "spi", "sensor", "access-control"},
    {"spi"},
    "MIT",
    true, true, false,
    "std/embedded/packages/rfid",
    "https://github.com/miguelbalboa/rfid",
    "Usage: import rfid; let reader = rfid.open(SPI, SS_PIN); if (reader.hasCard()) { print(reader.uid()); }"
};

// ======================== RTC ========================
static PackageInfo pkg_rtc = {
    "rtc", "1.0.0",
    "DS1307/DS3231 real-time clock (I2C)",
    "engtech-team",
    {"rtc", "time", "i2c", "clock"},
    {"i2c"},
    "MIT",
    true, true, false,
    "std/embedded/packages/rtc",
    "https://github.com/adafruit/RTClib",
    "Usage: import rtc; let clock = rtc.open(i2c_bus); let t = clock.now(); print(t.hour); print(t.minute);"
};

// ======================== GPS ========================
static PackageInfo pkg_gps = {
    "gps", "1.0.0",
    "GPS receiver module (UART NMEA)",
    "engtech-team",
    {"gps", "uart", "navigation", "gprmc"},
    {"uart"},
    "MIT",
    false, true, false,
    "std/embedded/packages/gps",
    "https://github.com/adafruit/Adafruit_GPS",
    "Usage: import gps; let g = gps.open(UART2); g.read(); print(g.latitude()); print(g.longitude());"
};

// ======================== MQTT ========================
static PackageInfo pkg_mqtt = {
    "mqtt", "1.0.0",
    "MQTT client for ESP32 IoT applications",
    "engtech-team",
    {"mqtt", "iot", "network", "esp32-only"},
    {"wifi"},
    "MIT",
    false, true, false,
    "std/embedded/packages/mqtt",
    "https://github.com/knolleary/pubsubclient",
    "Usage: import mqtt; let client = mqtt.connect(\"mqtt://broker\", \"my-device\"); client.publish(\"home/temp\", \"27.5\");"
};

// ======================== WiFi ========================
static PackageInfo pkg_wifi = {
    "wifi", "1.0.0",
    "WiFi client/server for ESP32",
    "engtech-team",
    {"wifi", "network", "esp32-only"},
    {},
    "MIT",
    false, true, false,
    "std/embedded/packages/wifi",
    "esp-idf built-in",
    "Usage: import wifi; wifi.connect(\"ssid\", \"pass\"); print(wifi.localIP());"
};

// ======================== Web Server ========================
static PackageInfo pkg_webserver = {
    "webserver", "1.0.0",
    "Lightweight HTTP server for ESP32",
    "engtech-team",
    {"web", "http", "server", "esp32-only", "iot"},
    {"wifi"},
    "MIT",
    false, true, false,
    "std/embedded/packages/webserver",
    "esp-idf built-in",
    "Usage: import webserver; let srv = webserver.open(80); srv.get(\"/\", fn(req) { return \"Hello!\"; });"
};

// ======================== Storage (NVS/SD/EEPROM) ========================
static PackageInfo pkg_storage = {
    "storage", "1.0.0",
    "Unified storage API: NVS (ESP32), EEPROM (Arduino), SD card",
    "engtech-team",
    {"storage", "filesystem", "persistent"},
    {},
    "MIT",
    true, true, false,
    "std/embedded/packages/storage",
    "",
    "Usage: import storage; storage.write(\"/config\", \"value=42\"); let v = storage.read(\"/config\");"
};

// ======================== Power Management ========================
static PackageInfo pkg_power = {
    "power", "1.0.0",
    "Deep sleep, light sleep, and wake source management for ESP32",
    "engtech-team",
    {"power", "sleep", "esp32-only", "low-power"},
    {},
    "MIT",
    false, true, false,
    "std/embedded/packages/power",
    "esp-idf built-in",
    "Usage: import power; power.deep_sleep(60.seconds()); // wakes via GPIO or timer"
};

// ======================== BLE ========================
static PackageInfo pkg_ble = {
    "ble", "1.0.0",
    "Bluetooth Low Energy server/client for ESP32",
    "engtech-team",
    {"ble", "bluetooth", "esp32-only", "iot"},
    {},
    "MIT",
    false, true, false,
    "std/embedded/packages/ble",
    "esp-idf built-in",
    "Usage: import ble; let server = ble.server(\"MyDevice\");"
};

// ======================== Task Scheduler ========================
static PackageInfo pkg_taskscheduler = {
    "taskscheduler", "1.0.0",
    "Multi-task scheduler with priority support for embedded systems",
    "engtech-team",
    {"scheduler", "rtos", "task", "real-time"},
    {},
    "MIT",
    true, true, false,
    "std/embedded/packages/taskscheduler",
    "",
    "Usage: import taskscheduler; let t = taskscheduler.create(high_priority, fn() { read_sensor(); });"
};

// ======================== Resource Analyzer ========================
static PackageInfo pkg_resourceanalyzer = {
    "resourceanalyzer", "1.0.0",
    "Build-time resource analyzer: flash/RAM usage report",
    "engtech-team",
    {"analyzer", "build-tool", "report", "memory"},
    {},
    "MIT",
    true, true, true,
    "compiler/resource_analyzer",
    "",
    "Usage: engc build --analyze — prints flash/RAM usage report after build"
};

// ======================== Registry ========================
static const PackageInfo* all_packages[] = {
    &pkg_dht,
    &pkg_bme280,
    &pkg_bmp280,
    &pkg_mpu6050,
    &pkg_ssd1306,
    &pkg_ws2812,
    &pkg_servo,
    &pkg_relay,
    &pkg_ultrasonic,
    &pkg_sdcard,
    &pkg_rfid,
    &pkg_rtc,
    &pkg_gps,
    &pkg_mqtt,
    &pkg_wifi,
    &pkg_webserver,
    &pkg_storage,
    &pkg_power,
    &pkg_ble,
    &pkg_taskscheduler,
    &pkg_resourceanalyzer,
    nullptr
};

std::vector<PackageInfo> getRegistry() {
    std::vector<PackageInfo> result;
    for (const auto* pkg : all_packages) {
        if (pkg) result.push_back(*pkg);
    }
    return result;
}

PackageInfo* findPackage(const std::string& name) {
    for (const auto* pkg : all_packages) {
        if (pkg && pkg->name == name) return const_cast<PackageInfo*>(pkg);
    }
    return nullptr;
}

std::vector<PackageInfo> search(const std::string& keyword) {
    std::vector<PackageInfo> results;
    std::string kw = keyword;
    std::transform(kw.begin(), kw.end(), kw.begin(), ::tolower);
    for (const auto* pkg : all_packages) {
        if (!pkg) continue;
        std::string combined = pkg->name + " " + pkg->description + " ";
        for (auto& t : pkg->tags) combined += t + " ";
        std::string lc = combined;
        std::transform(lc.begin(), lc.end(), lc.begin(), ::tolower);
        if (lc.find(kw) != std::string::npos) results.push_back(*pkg);
    }
    return results;
}

std::vector<PackageInfo> filterByTarget(const std::string& target) {
    std::vector<PackageInfo> results;
    for (const auto* pkg : all_packages) {
        if (!pkg) continue;
        if (target == "arduino" && pkg->for_arduino) results.push_back(*pkg);
        else if (target == "esp32" && pkg->for_esp32) results.push_back(*pkg);
        else if (target == "rpi-pico" && pkg->for_rpi_pico) results.push_back(*pkg);
        else if (target == "all") results.push_back(*pkg);
    }
    return results;
}

std::vector<std::string> getDependencies(const std::string& packageName) {
    auto* pkg = findPackage(packageName);
    if (!pkg) return {};
    return pkg->dependencies;
}

} // namespace packages
} // namespace eng
