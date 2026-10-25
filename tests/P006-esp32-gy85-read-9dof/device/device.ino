/*
 *  === ESP32-C3 WiFi Direct Connection Test ===
 */

#include <Arduino.h>
#include <Wire.h>

#define PIN_SDA 8
#define PIN_SCL 9


// === Constants
const int BAUD = 115200;
const uint8_t ACCEL_ADDR = 0x53; // ADXL345
const uint8_t GYRO_ADDR  = 0x68; // ITG3200/ITG3205
const uint8_t MAG_ADDR   = 0x1E; // HMC5883L

// === Functions
// Write one register
bool writeRegister(uint8_t address, uint8_t reg, uint8_t value) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(value);
  return Wire.endTransmission() == 0;
}

// Read consecutive registers
bool readRegisters(uint8_t address, uint8_t reg, uint8_t* data, size_t length) {
  Wire.beginTransmission(address);
  Wire.write(reg);

  if (Wire.endTransmission(false) != 0) {
    return false;
  }

  if (Wire.requestFrom(address, length) != length) {
    return false;
  }

  for (size_t i = 0; i < length; i++) {
    data[i] = Wire.read();
  }

  return true;
}

// Combine bytes in little-endian order (ADXL345)
int16_t littleEndian(uint8_t low, uint8_t high) {
  return (int16_t)((uint16_t)high << 8 | low);
}

// Combine bytes in big-endian order (gyro and magnetometer)
int16_t bigEndian(uint8_t high, uint8_t low) {
  return (int16_t)((uint16_t)high << 8 | low);
}


// === Loops
void setup() {
  Serial.begin(BAUD);

  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.setClock(100000);

  // Initialize accelerometer: measurement mode
  writeRegister(ACCEL_ADDR, 0x2D, 0x08);

  // Initialize gyroscope: wake up and configure
  writeRegister(GYRO_ADDR, 0x3E, 0x00);
  writeRegister(GYRO_ADDR, 0x15, 0x07);
  writeRegister(GYRO_ADDR, 0x16, 0x1B);

  // Initialize magnetometer: continuous measurement
  writeRegister(MAG_ADDR, 0x00, 0x70);
  writeRegister(MAG_ADDR, 0x01, 0x20);
  writeRegister(MAG_ADDR, 0x02, 0x00);

  delay(100);

  Serial.println("GY-85 sensor test");
  Serial.println("Accelerometer | Gyroscope | Magnetometer");
}

void loop() {
  uint8_t data[6];

  // Accelerometer
  if (readRegisters(ACCEL_ADDR, 0x32, data, 6)) {
    int16_t ax = littleEndian(data[0], data[1]);
    int16_t ay = littleEndian(data[2], data[3]);
    int16_t az = littleEndian(data[4], data[5]);

    Serial.printf("ACC  X:%6d Y:%6d Z:%6d | ", ax, ay, az);
  } else {
    Serial.print("ACC read failed | ");
  }

  // Gyroscope
  if (readRegisters(GYRO_ADDR, 0x1D, data, 6)) {
    int16_t gx = bigEndian(data[0], data[1]);
    int16_t gy = bigEndian(data[2], data[3]);
    int16_t gz = bigEndian(data[4], data[5]);

    Serial.printf("GYR  X:%6d Y:%6d Z:%6d | ", gx, gy, gz);
  } else {
    Serial.print("GYR read failed | ");
  }

  // Magnetometer: register order is X, Z, Y
  if (readRegisters(MAG_ADDR, 0x03, data, 6)) {
    int16_t mx = bigEndian(data[0], data[1]);
    int16_t mz = bigEndian(data[2], data[3]);
    int16_t my = bigEndian(data[4], data[5]);

    Serial.printf("MAG  X:%6d Y:%6d Z:%6d", mx, my, mz);
  } else {
    Serial.print("MAG read failed");
  }

  Serial.println();
  delay(100);
}
