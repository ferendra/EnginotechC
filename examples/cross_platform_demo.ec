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
import embedded.i2c;
import embedded.system;

fn main() {
    println("=== EnginotechC++ Cross-Platform Sensor Demo ===");
    println("");
    
    // Check target platform
    let plat = platform();
    if plat.is_windows {
        println("[HOST] Running on Windows (simulated readings)");
    } else if plat.is_posix {
        println("[HOST] Running on POSIX (simulated readings)");
    }
    println("");
    
    // DHT Sensor Demo
    println("--- DHT Temperature/Humidity Sensor ---");
    let dht_sensor = dht.connect(4, DhtType.DHT22);
    
    // Read temperature and humidity
    let temp = dht.readTemperature(dht_sensor);
    let hum = dht.readHumidity(dht_sensor);
    let heatIndex = dht.heatIndex(dht_sensor);
    let dewPoint = dht.dewPoint(dht_sensor);
    
    print("Temperature: ");
    print(temp);
    println("°C");
    print("Humidity: ");
    print(hum);
    println("%RH");
    print("Heat Index: ");
    print(heatIndex);
    println("°C");
    print("Dew Point: ");
    print(dewPoint);
    println("°C");
    println("");
    
    // BME280 Demo (I2C)
    println("--- BME280 Barometric Sensor ---");
    let i2c_bus = i2c_open(0, 118);  // I2C bus 0, address 0x76 = 118
    let bme = bme280.connect(i2c_bus);
    
    let press = bme280.readPressure(bme);
    let alt = bme280.readAltitude(bme);
    let bmeTemp = bme280.readTemperature(bme);
    
    print("Pressure: ");
    print(press);
    println(" hPa");
    print("Altitude: ");
    print(alt);
    println(" m");
    print("Temperature: ");
    print(bmeTemp);
    println("°C");
    println("");
    
    // System uptime
    println("--- System Info ---");
    let uptime = millis();
    print("Uptime: ");
    print(uptime);
    println(" ms");
    println("");
    
    println("=== Demo Complete ===");
}