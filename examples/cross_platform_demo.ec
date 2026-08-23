// EnginotechC++ Cross-Platform Sensor Demo
// Demonstrates sensor packages on different platforms
// This code compiles and runs on:
//   - Linux (host testing)
//   - FreeBSD, OpenBSD, NetBSD (POSIX targets)
//   - macOS (development)
//   - Windows (development via MinGW)
//   - Arduino AVR (compiles to .hex)
//   - ESP32 (compiles to .bin)

import dht;
import bme280;
import i2c;
import system;

fn main() {
    io.println("=== EnginotechC++ Cross-Platform Sensor Demo ===");
    io.println("");
    
    // Check target platform
    if platform.isWindows() {
        io.println("[HOST] Running on Windows (simulated readings)");
    } else if platform.isPOSIX() {
        io.println("[HOST] Running on POSIX (simulated readings)");
    }
    io.println("");
    
    // DHT Sensor Demo
    io.println("--- DHT Temperature/Humidity Sensor ---");
    let dht_sensor = dht.connect(4, dht.Type.DHT22);
    
    // Read temperature and humidity
    let temp = dht.readTemperature(dht_sensor);
    let hum = dht.readHumidity(dht_sensor);
    let heatIndex = dht.heatIndex(dht_sensor);
    let dewPoint = dht.dewPoint(dht_sensor);
    
    io.print("Temperature: ");
    io.print(temp);
    io.println("°C");
    io.print("Humidity: ");
    io.print(hum);
    io.println("%RH");
    io.print("Heat Index: ");
    io.print(heatIndex);
    io.println("°C");
    io.print("Dew Point: ");
    io.print(dewPoint);
    io.println("°C");
    io.println("");
    
    // BME280 Demo (I2C)
    io.println("--- BME280 Barometric Sensor ---");
    let i2c_bus = i2c.open(0, 0x76);  // I2C bus 0, address 0x76
    let bme = bme280.open(i2c_bus);
    
    let press = bme280.readPressure(bme);
    let alt = bme280.readAltitude(bme);
    let bmeTemp = bme280.readTemperature(bme);
    
    io.print("Pressure: ");
    io.print(press);
    io.println(" hPa");
    io.print("Altitude: ");
    io.print(alt);
    io.println(" m");
    io.print("Temperature: ");
    io.print(bmeTemp);
    io.println("°C");
    io.println("");
    
    // System uptime
    io.println("--- System Info ---");
    let uptime = system.millis();
    io.print("Uptime: ");
    io.print(uptime);
    io.println(" ms");
    io.println("");
    
    io.println("=== Demo Complete ===");
}
