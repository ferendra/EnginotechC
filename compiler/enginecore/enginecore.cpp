// EnginotechC++ — Engine Core Implementation (platform-specific)
// Provides the low-level hardware abstraction for embedded targets

#include "enginecore.h"
#include <chrono>
#include <thread>
#include <cstdio>

namespace eng {
namespace enginecore {

static std::chrono::steady_clock::time_point sBootTime;

void ecInit() {
    sBootTime = std::chrono::steady_clock::now();
#ifdef ENG_TARGET_ARDUINO
    // AVR init
    cli();
#elif defined(ENG_TARGET_ESP32) || defined(ENG_TARGET_ESP32S3) || defined(ENG_TARGET_ESP32C3)
    // ESP32 init — called by esp-idf before app_init
#endif
}

void ecDelayMs(uint32_t ms) {
#ifdef ENG_TARGET_ARDUINO
    _delay_ms(ms);
#elif defined(ENG_TARGET_ESP32) || defined(ENG_TARGET_ESP32S3) || defined(ENG_TARGET_ESP32C3)
    vTaskDelay(ms / portTICK_PERIOD_MS);
#else
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
#endif
}

void ecDelayUs(uint32_t us) {
#ifdef ENG_TARGET_ARDUINO
    _delay_us(us);
#elif defined(ENG_TARGET_ESP32) || defined(ENG_TARGET_ESP32S3) || defined(ENG_TARGET_ESP32C3)
    ets_delay_us(us);
#else
    std::this_thread::sleep_for(std::chrono::microseconds(us));
#endif
}

uint32_t ecMillis() {
    auto now = std::chrono::steady_clock::now();
    return static_cast<uint32_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(now - sBootTime).count());
}

uint32_t ecMicros() {
    auto now = std::chrono::steady_clock::now();
    return static_cast<uint32_t>(
        std::chrono::duration_cast<std::chrono::microseconds>(now - sBootTime).count());
}

void ecPinMode(uint32_t pin, uint32_t mode) {
#ifdef ENG_TARGET_ARDUINO
    pinMode(pin, static_cast<uint8_t>(mode));
#elif defined(ENG_TARGET_ESP32)
    gpio_set_direction(static_castgpio_num_t>(pin),
                       mode == 1 ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT);
    if (mode == 2) gpio_set_pullup(pin);
    else if (mode == 3) gpio_set_pull_down(pin);
#endif
}

void ecDigitalWrite(uint32_t pin, uint32_t level) {
#ifdef ENG_TARGET_ARDUINO
    digitalWrite(pin, level ? HIGH : LOW);
#elif defined(ENG_TARGET_ESP32)
    gpio_set_level(static_castgpio_num_t>(pin), level);
#endif
}

uint32_t ecDigitalRead(uint32_t pin) {
#ifdef ENG_TARGET_ARDUINO
    return digitalRead(pin);
#elif defined(ENG_TARGET_ESP32)
    return gpio_get_level(static_castgpio_num_t>(pin));
#endif
    return 0;
}

void ecUartBegin(uint32_t port, uint32_t baud) {
#ifdef ENG_TARGET_ARDUINO
    Serial.begin(baud);
#elif defined(ENG_TARGET_ESP32)
    // uart_driver_install(UART_NUM_0, 256, 256, 0, nullptr, 0);
    // uart_set_baudrate(UART_NUM_0, baud);
    (void)port;
#endif
}

void ecUartWrite(uint32_t port, const char* s) {
    (void)port;
#ifdef ENG_TARGET_ARDUINO
    Serial.print(s);
#elif defined(ENG_TARGET_ESP32)
    uart_write_bytes(UART_NUM_0, s, strlen(s));
#else
    printf("%s", s);
#endif
}

int ecUartRead(uint32_t port) {
    (void)port;
#ifdef ENG_TARGET_ARDUINO
    return Serial.read();
#else
    return getchar();
#endif
}

int ecUartAvailable(uint32_t port) {
    (void)port;
#ifdef ENG_TARGET_ARDUINO
    return Serial.available();
#else
    return 0;
#endif
}

void ecPwmSetup(uint32_t pin, uint32_t channel, uint32_t freq, uint32_t resolution) {
    (void)pin; (void)channel; (void)freq; (void)resolution;
#ifdef ENG_TARGET_ESP32
    // ledcSetup(channel, freq, resolution);
    // ledcAttachPin(pin, channel);
#endif
}

void ecPwmWrite(uint32_t channel, uint32_t duty) {
    (void)channel; (void)duty;
#ifdef ENG_TARGET_ESP32
    // ledcWrite(channel, duty);
#endif
}

void ecPwmStop(uint32_t channel) {
    (void)channel;
#ifdef ENG_TARGET_ESP32
    // ledcDetachPin(channel);
#endif
}

uint32_t ecAdcRead(uint32_t pin) {
#ifdef ENG_TARGET_ESP32
    // return adc1_get_reading(static_castadc1_channel_t>(pin));
    return 0;
#endif
    return 0;
}

void ecAdcSetResolution(uint32_t pin, uint32_t bits) {
    (void)pin; (void)bits;
}

void ecAttachInterrupt(uint32_t pin, void (*callback)(void), uint32_t edge) {
    (void)pin; (void)callback; (void)edge;
#ifdef ENG_TARGET_ESP32
    // gpio_isr_handler_add((gpio_num_t)pin, callback, nullptr);
#endif
}

void ecDetachInterrupt(uint32_t pin) {
    (void)pin;
}

void ecInterruptsEnable() {
#ifdef ENG_TARGET_ARDUINO
    sei();
#elif defined(ENG_TARGET_ESP32)
    // esp_intr_enable();
#endif
}

void ecInterruptsDisable() {
#ifdef ENG_TARGET_ARDUINO
    cli();
#elif defined(ENG_TARGET_ESP32)
    // esp_intr_disable();
#endif
}

bool ecInterruptsEnabled() {
#ifdef ENG_TARGET_ARDUINO
    return (SREG & _BV(SREG_I)) != 0;
#else
    return true;
#endif
}

size_t ecFreeHeap() {
#ifdef ENG_TARGET_ESP32
    return heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
#elif defined(ENG_TARGET_ARDUINO)
    extern int __brkval;
    extern int __heap_start;
    int v;
    return (size_t)&v - (size_t)__brkval;
#else
    return 64ULL * 1024 * 1024;
#endif
}

size_t ecTotalHeap() {
#ifdef ENG_TARGET_ESP32
    return heap_caps_get_total_size(MALLOC_CAP_INTERNAL);
#else
    return ecFreeHeap();
#endif
}

const char* ecChipModel() {
#ifdef ESP32
    return "ESP32";
#elif defined(ESP32S3)
    return "ESP32-S3";
#elif defined(ESP32C3)
    return "ESP32-C3";
#elif defined(__AVR_ATmega328P__)
    return "ATmega328P";
#else
    return "Unknown";
#endif
}

uint32_t ecChipFreq() {
#ifdef ENG_CPU_FREQ_HZ
    return ENG_CPU_FREQ_HZ;
#else
    return 0;
#endif
}

} // namespace enginecore
} // namespace eng
