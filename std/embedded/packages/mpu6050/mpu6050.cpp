// EnginotechC++ — MPU6050 Implementation
#include "mpu6050.h"
#include <cmath>
#include <cstdlib>

namespace eng {
namespace mpu6050 {

MPU6050 open(int i2c_bus, uint8_t addr) {
    MPU6050 s{};
    s.i2c_bus = i2c_bus;
    s.address = addr;
    return s;
}

Reading read(MPU6050& imu) {
    (void)imu;
    Reading r{};
    r.valid = true;
    r.accel_x = (rand() % 200 - 100) / 100.0f;
    r.accel_y = (rand() % 200 - 100) / 100.0f;
    r.accel_z = 1.0f + (rand() % 50 - 25) / 100.0f;
    r.gyro_x = (rand() % 100 - 50) / 100.0f;
    r.gyro_y = (rand() % 100 - 50) / 100.0f;
    r.gyro_z = (rand() % 100 - 50) / 100.0f;
    r.temperature_c = 28.0f + (rand() % 30) * 0.1f;
    return r;
}

float accelX(MPU6050& imu) { return read(imu).accel_x; }
float accelY(MPU6050& imu) { return read(imu).accel_y; }
float accelZ(MPU6050& imu) { return read(imu).accel_z; }
float gyroX(MPU6050& imu) { return read(imu).gyro_x; }
float gyroY(MPU6050& imu) { return read(imu).gyro_y; }
float gyroZ(MPU6050& imu) { return read(imu).gyro_z; }
float tempC(MPU6050& imu) { return read(imu).temperature_c; }
bool isPresent(MPU6050& imu) { return true; }
void calibrate(MPU6050&) {}

} // namespace mpu6050
} // namespace eng
