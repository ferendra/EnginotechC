// EnginotechC++ — Embedded UART Implementation

#include "uart.h"
#include <cstdio>
#include <cstring>

namespace eng {
namespace embedded {
namespace uart {

static SerialPort defaultSerial{0, 115200};

SerialPort open(int port, uint32_t baud) {
    SerialPort sp{};
    sp.port = port;
    sp.baudrate = baud;
    sp.begin(baud);
    return sp;
}

SerialPort& serial() {
    return defaultSerial;
}

void SerialPort::begin(uint32_t baud) {
    baudrate = baud;
    // Platform-specific UART init (e.g., UBRR for AVR, UxCFG for ESP32)
}

void SerialPort::end() {
    // Disable UART
}

size_t SerialPort::write(const char* s) {
    if (!s) return 0;
    size_t len = strlen(s);
    // In production, use platform-specific UART write
    (void)len;
    return len;
}

size_t SerialPort::write(const uint8_t* buf, size_t len) {
    for (size_t i = 0; i < len; ++i) {
        write(&buf[i], 1);
    }
    return len;
}

int SerialPort::read() {
    return getchar();  // Host fallback
}

int SerialPort::available() const {
    // Platform-specific: check UART buffer
    return 0;
}

void SerialPort::flush() {
    fflush(stdout);
}

bool SerialPort::isRunning() const {
    return baudrate > 0;
}

int SerialPort::readBytes(char* buf, int len, uint32_t timeoutMs) {
    (void)timeoutMs;
    int count = 0;
    for (int i = 0; i < len; ++i) {
        int c = read();
        if (c < 0) break;
        buf[i] = (char)c;
        count++;
    }
    return count;
}

} // namespace uart
} // namespace embedded
} // namespace eng
