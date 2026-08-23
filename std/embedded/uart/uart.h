// EnginotechC++ — Embedded UART/Serial HAL
// Portable UART abstraction for Arduino and ESP32

#ifndef ENG_EMBEDDED_UART_H
#define ENG_EMBEDDED_UART_H

#include <cstdint>
#include <string>

namespace eng {
namespace embedded {
namespace uart {

struct SerialPort {
    int port;
    uint32_t baudrate;

    void begin(uint32_t baud);
    void end();
    size_t write(const char* s);
    size_t write(const uint8_t* buf, size_t len);
    int read();
    int available() const;
    void flush();
    bool isRunning() const;

    // Non-blocking read into buffer
    int readBytes(char* buf, int len, uint32_t timeoutMs = 1000);
};

// Open a serial port (port 0 = primary, 1 = secondary, etc.)
SerialPort open(int port = 0, uint32_t baud = 115200);

// Global default serial (for print debugging)
SerialPort& serial();

} // namespace uart
} // namespace embedded
} // namespace eng

#endif // ENG_EMBEDDED_UART_H
