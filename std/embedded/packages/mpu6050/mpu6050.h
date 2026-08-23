// EnginotechC++ — MPU6050 IMU Package
// 6-axis gyroscope + accelerometer via I2C

#ifndef ENG_MPU6050_H
#define ENG_MPU6050_H

#include <cstdint>

namespace eng {
namespace mpu6050 {

struct Reading {
    float accel_x, accel_y, accel_z;  // g
    float gyro_x, gyro_y, gyro_z;     // degrees/sec
    float temperature_c;
    bool valid;
};

struct MPU6050 {
    int i2c_bus;
    uint8_t address;  // Default 0x68
};

MPU6050 open(int i2c_bus, uint8_t addr = 0x68);
Reading read(MPU6050& imu);
float accelX(MPU6050& imu);
float accelY(MPU6050& imu);
float accelZ(MPU6050& imu);
float gyroX(MPU6050& imu);
float gyroY(MPU6050& imu);
float gyroZ(MPU6050& imu);
float tempC(MPU6050& imu);
bool isPresent(MPU6050& imu);
void calibrate(MPU6050& imu);

} // namespace mpu6050
} // namespace eng

#endif // ENG_MPU6050_H
