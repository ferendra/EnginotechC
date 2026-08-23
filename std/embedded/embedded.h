// EnginotechC++ — Embedded HAL Module Registry
// Single include point for all embedded modules
//
// Usage in EC source:
//   import embedded.gpio;
//   import embedded.uart;
//   import embedded.pwm;
//   import embedded.adc;
//   import embedded.i2c;
//   import embedded.spi;
//   import embedded.timer;
//   import embedded.interrupt;
//   import embedded.wifi;     // ESP32 only
//   import embedded.bluetooth; // ESP32 only
//   import embedded.sleep;
//   import embedded.system;

#ifndef ENG_EMBEDDED_H
#define ENG_EMBEDDED_H

#include "gpio/gpio.h"
#include "pwm/pwm.h"
#include "adc/adc.h"
#include "uart/uart.h"
#include "i2c/i2c.h"
#include "spi/spi.h"
#include "timer/timer.h"
#include "interrupt/interrupt.h"
#include "wifi/wifi.h"
#include "bluetooth/bt.h"
#include "sleep/sleep.h"
#include "system/system.h"

#endif // ENG_EMBEDDED_H
