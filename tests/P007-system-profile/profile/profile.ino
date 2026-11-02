/*
 *  === ESP32-C3 WiFi Instrument System Profile ===
 */

// === Test list
// 1. Measure sensor acquisition time: acc, gyro, mag and total times (a) w/out Wifi connection; b) w/ Wifi connection)
// 2. Measure UDP packets transmition time (a) paylod min size [1 byte]; b) payload max size [1472 bytes]; c) using OSC message [similar to P005])
// 3. Measure sensor acquisition w/ delay-based sampling rate
// 4. Measure sensor acquisition w/ delay-based sampling rate and transmission
// -- Future Tests Ideas --
// 3b. Measure sensor acquisition w/ controlled sampling rate
// 4b. Measure sensor acquisition w/ controlled sampling rate and transmission

#include <Arduino.h>
#include <WiFi.h>
#include <OSCMessage.h>
#include <Wire.h>

#define PIN_SDA 8
#define PIN_SCL 9


// ================================================
// === Constants
// -- Serial config
const int BAUD = 115200;
const uint8_t SERIAL_STARTUP_DELAY_MS = 10;

// -- Wifi config
const char SSID[] = "ssid";
const char PASS[] = "pass";
const uint16_t MAX_NETWORK_CONNECTION_TRIES = 360;
const uint16_t TIME_BETWEEN_CONNECT_TRIES_MS = 500;
const uint8_t WIFI_DISCONNECT_DELAY_MS = 100;

// -- Network config
const char HOST[] = "XXX.XXX.XXX.XXX";
const uint16_t UDP_PORT = 8888;

// -- OSC config
const uint16_t OSC_PORT = 6449;
const size_t SENSOR_DATA_COUNT = 9;

// -- I2C configuration
const uint32_t SENSOR_CLOCK_FREQ = 100000;
const size_t SENSOR_REGISTER_BLOCK_SIZE = 6;
const uint16_t SENSOR_STARTUP_DELAY_MS = 100;

// -- I2C addressess and registers
// ADXL345 (Accelerometer)
const uint8_t ACCEL_ADDRESS = 0x53;
const uint8_t ACCEL_REG_POWER_CTL = 0x2D;
const uint8_t ACCEL_POWER_CTL_VALUE = 0x08;
const uint8_t ACCEL_REG_DATA_START = 0x32;

// ITG3200/ITG3205 (Gyroscope)
const uint8_t GYRO_ADDRESS = 0x68;
const uint8_t GYRO_REG_PWR_MGM = 0x3E;
const uint8_t GYRO_PWR_MGM_VALUE = 0x00;
const uint8_t GYRO_REG_SMPLRT_DIV = 0x15;
const uint8_t GYRO_SMPLRT_DIV_VALUE = 0x07;
const uint8_t GYRO_REG_DLPF_FS = 0x16;
const uint8_t GYRO_DLPF_FS_VALUE = 0x1B;
const uint8_t GYRO_REG_DATA_START = 0x1D;

// HMC5883L (Magnetometer)
const uint8_t MAG_ADDRESS = 0x1E;
const uint8_t MAG_REG_CONFIG_A = 0x00;
const uint8_t MAG_CONFIG_A_VALUE = 0x70;
const uint8_t MAG_REG_CONFIG_B = 0x01;
const uint8_t MAG_CONFIG_B_VALUE = 0x20;
const uint8_t MAG_REG_MODE = 0x02;
const uint8_t MAG_CONTINUOUS_MODE_VALUE = 0x00;
const uint8_t MAG_REG_DATA_START = 0x03;


// ================================================
// === Variables
bool isConnectedToNetwork = false;
bool isConnectedToHost = false;
NetworkClient client;
WiFiUDP udp;


// ================================================
// === Connection
// -- Wi-Fi
bool connectToNetwork(const char ssid[], const char pass[]) {
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(WIFI_DISCONNECT_DELAY_MS);

  // Connect to Wi-Fi network
  WiFi.begin(ssid, pass);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);

  // Wait until connection is established, stop if MAX_NETWORK_CONNECTION_TRIES is exceeded
  int retryCount = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if (retryCount > MAX_NETWORK_CONNECTION_TRIES) {
      return false;
    }

    delay(TIME_BETWEEN_CONNECT_TRIES_MS);
    retryCount++;
  }

  return true;
}

// -- UDP
bool initUdp() {
  if (udp.begin(UDP_PORT) == 0) {
    return false;
  }
  return true;
}

// -- OSC
bool sendOSCMessage(OSCMessage msg) {
  if (udp.beginPacket(HOST, OSC_PORT)) {
    msg.send(udp);

    if (udp.endPacket() == 0) {
      return false;
    }

    return true;
  }
}

OSCMessage buildOscMessage(char msgAddress[], int16_t payload[], size_t payloadSize) {
  OSCMessage msg(msgAddress);
  for (int i = 0; i < payloadSize; i++) {
    msg.add(payload[i]);
  }
  return msg;
}


// ================================================
// === GY-85 Sensor functions
void setupSensor() {
  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.setClock(SENSOR_CLOCK_FREQ);

  // Initialize accelerometer
  writeRegister(ACCEL_ADDRESS, ACCEL_REG_POWER_CTL, ACCEL_POWER_CTL_VALUE);

  // Initialize gyroscope
  writeRegister(GYRO_ADDRESS, GYRO_REG_PWR_MGM, GYRO_PWR_MGM_VALUE);
  writeRegister(GYRO_ADDRESS, GYRO_REG_SMPLRT_DIV, GYRO_SMPLRT_DIV_VALUE);
  writeRegister(GYRO_ADDRESS, GYRO_REG_DLPF_FS, GYRO_DLPF_FS_VALUE);

  // Initialize magnetometer
  writeRegister(MAG_ADDRESS, MAG_REG_CONFIG_A, MAG_CONFIG_A_VALUE);
  writeRegister(MAG_ADDRESS, MAG_REG_CONFIG_B, MAG_CONFIG_B_VALUE);
  writeRegister(MAG_ADDRESS, MAG_REG_MODE, MAG_CONTINUOUS_MODE_VALUE);
}

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

// Combine bytes in little-endian order (for accelerometer)
int16_t littleEndian(uint8_t low, uint8_t high) {
  return (int16_t)((uint16_t)high << 8 | low);
}

// Combine bytes in big-endian order (gyroscope and magnetometer)
int16_t bigEndian(uint8_t high, uint8_t low) {
  return (int16_t)((uint16_t)high << 8 | low);
}

bool readSensorData(int16_t data[SENSOR_DATA_COUNT]) {
  uint8_t rawData[SENSOR_REGISTER_BLOCK_SIZE];
  bool accOk = false;
  bool gyroOk = false;
  bool magOk = false;

  // Accelerometer
  if (readRegisters(ACCEL_ADDRESS, ACCEL_REG_DATA_START, rawData, SENSOR_REGISTER_BLOCK_SIZE)) {
    int16_t ax = littleEndian(rawData[0], rawData[1]);
    int16_t ay = littleEndian(rawData[2], rawData[3]);
    int16_t az = littleEndian(rawData[4], rawData[5]);
    data[0] = ax; data[1] = ay; data[2] = az;
    accOk = true;
  }

  // Gyroscope
  if (readRegisters(GYRO_ADDRESS, GYRO_REG_DATA_START, rawData, SENSOR_REGISTER_BLOCK_SIZE)) {
    int16_t gx = bigEndian(rawData[0], rawData[1]);
    int16_t gy = bigEndian(rawData[2], rawData[3]);
    int16_t gz = bigEndian(rawData[4], rawData[5]);
    data[3] = gx; data[4] = gy; data[5] = gz;
    gyroOk = true;
  }

  // Magnetometer (register order is X, Z, Y)
  if (readRegisters(MAG_ADDRESS, MAG_REG_DATA_START, rawData, SENSOR_REGISTER_BLOCK_SIZE)) {
    int16_t mx = bigEndian(rawData[0], rawData[1]);
    int16_t mz = bigEndian(rawData[2], rawData[3]);
    int16_t my = bigEndian(rawData[4], rawData[5]);
    data[6] = mx; data[7] = mz; data[8] = my;
    magOk = true;
  }

  return accOk && gyroOk && magOk;
}


// ================================================
// === Test suite
const size_t NUM_TESTS = 100;
char OSC_ADDRESS[] = "/test";
const float SAMPLE_RATE[] = { 50.0, 100.0, 500.0, 1000.0 };

// Test 1.
void testPerformanceSensorReadInMicros() {
  unsigned long results[NUM_TESTS][4];
  
  // -- Test -------------
  int16_t data[SENSOR_DATA_COUNT];
  for (int i = 0; i < NUM_TESTS; i++) {
    uint8_t rawData[SENSOR_REGISTER_BLOCK_SIZE];

    // Accelerometer
    results[i][0] = micros();
    if (readRegisters(ACCEL_ADDRESS, ACCEL_REG_DATA_START, rawData, SENSOR_REGISTER_BLOCK_SIZE)) {
      int16_t ax = littleEndian(rawData[0], rawData[1]);
      int16_t ay = littleEndian(rawData[2], rawData[3]);
      int16_t az = littleEndian(rawData[4], rawData[5]);
      data[0] = ax; data[1] = ay; data[2] = az;
    }

    // Gyroscope
    results[i][1] = micros();
    if (readRegisters(GYRO_ADDRESS, GYRO_REG_DATA_START, rawData, SENSOR_REGISTER_BLOCK_SIZE)) {
      int16_t gx = bigEndian(rawData[0], rawData[1]);
      int16_t gy = bigEndian(rawData[2], rawData[3]);
      int16_t gz = bigEndian(rawData[4], rawData[5]);
      data[3] = gx; data[4] = gy; data[5] = gz;
    }

    // Magnetometer (register order is X, Z, Y)
    results[i][2] = micros();
    if (readRegisters(MAG_ADDRESS, MAG_REG_DATA_START, rawData, SENSOR_REGISTER_BLOCK_SIZE)) {
      int16_t mx = bigEndian(rawData[0], rawData[1]);
      int16_t mz = bigEndian(rawData[2], rawData[3]);
      int16_t my = bigEndian(rawData[4], rawData[5]);
      data[6] = mx; data[7] = mz; data[8] = my;
    }

    results[i][3] = micros();
  }
  // ---------------------

  // Show results
  Serial.println("=== TEST 1 - Sensor Read Performance ===");
  for (int i = 0; i < NUM_TESTS; i++) {
    unsigned long beforeAccReadTimestamp = results[i][0];
    unsigned long afterAccReadTimestamp = results[i][1];
    unsigned long beforeGyroReadTimestamp = results[i][1];
    unsigned long afterGyroReadTimestamp = results[i][2];
    unsigned long beforeMagReadTimestamp = results[i][2];
    unsigned long afterMagReadTimestamp = results[i][3];

    unsigned long accReadEllapsedTimeMicros = afterAccReadTimestamp - beforeAccReadTimestamp;
    unsigned long gyroReadEllapsedTimeMicros = afterGyroReadTimestamp - beforeGyroReadTimestamp;
    unsigned long magReadEllapsedTimeMicros = afterMagReadTimestamp - beforeMagReadTimestamp;
    unsigned long totalReadEllapsedTimeMicros = afterMagReadTimestamp - beforeAccReadTimestamp;

    Serial.print(i); Serial.print(";");

    Serial.print(beforeAccReadTimestamp); Serial.print(";"); Serial.print(afterAccReadTimestamp); Serial.print(";");
    Serial.print(afterGyroReadTimestamp); Serial.print(";");
    Serial.print(afterMagReadTimestamp); Serial.print(";");

    Serial.print(accReadEllapsedTimeMicros); Serial.print(";"); Serial.print(gyroReadEllapsedTimeMicros); Serial.print(";"); Serial.print(magReadEllapsedTimeMicros); Serial.print(";");
    Serial.println(totalReadEllapsedTimeMicros);
  }
}

// Test 2. a) and b)
void testPerformanceSendUdpPacketInMicros(size_t payloadSize) {
  unsigned long results[NUM_TESTS][2];
  uint8_t payload[payloadSize];
  for (int i = 0; i < payloadSize; i++) {
    payload[i] = 1;
  }
  
  // -- Test -------------
  for (int i = 0; i < NUM_TESTS; i++) {
    results[i][0] = micros();
    if (udp.beginPacket(HOST, OSC_PORT)) {
      udp.write(payload, payloadSize);
      udp.endPacket();
    }
    results[i][1] = micros();
  }
  // ---------------------

  // Show results
  Serial.print("=== TEST 2 - UDP packet transmition performance (payload= "); Serial.print(payloadSize); Serial.println(" bytes) ===");
  for (int i = 0; i < NUM_TESTS; i++) {
      Serial.print(i); Serial.print(";"); Serial.print(results[i][0]); Serial.print(";"); Serial.print(results[i][1]); Serial.print(";"); Serial.println(results[i][1] - results[i][0]);
  }
}

// Test 2. c)
void testPerformanceSendUdpPacketOSCMessageInMicros() {
  unsigned long results[NUM_TESTS][2];
  int16_t payload[SENSOR_DATA_COUNT] = { 1, 1, 1, 1, 1, 1, 1, 1, 1 };
  OSCMessage msg = buildOscMessage(OSC_ADDRESS, payload, SENSOR_DATA_COUNT);
  int payloadSize = msg.bytes();
  
  // -- Test -------------
  for (int i = 0; i < NUM_TESTS; i++) {
    results[i][0] = micros();
    sendOSCMessage(msg);
    results[i][1] = micros();
  }
  msg.empty();
  // ---------------------

  // Show results
  Serial.print("=== TEST 2c - UDP packet transmition performance (OSC message; payload= "); Serial.print(payloadSize); Serial.println(" bytes) ===");
  for (int i = 0; i < NUM_TESTS; i++) {
      Serial.print(i); Serial.print(";"); Serial.print(results[i][0]); Serial.print(";"); Serial.print(results[i][1]); Serial.print(";"); Serial.println(results[i][1] - results[i][0]);
  }
}

// Test 3.
void testPerformanceSensorReadDelayRateInMicros(float sampleRate) {
  unsigned long results[NUM_TESTS+1];
  uint32_t samplingIntervalMs = (uint32_t)(1/sampleRate * 1000);

  // -- Test -------------
  int16_t data[SENSOR_DATA_COUNT];
  for (int i = 0; i < NUM_TESTS; i++) {
    results[i] = micros();
    readSensorData(data);
    delay(samplingIntervalMs);
  }
  results[NUM_TESTS] = micros();
  // ---------------------

  // Show results
  Serial.print("=== TEST 3a - Sensor acquisition w/ delay-based sampling rate (sampling= "); Serial.print(sampleRate); Serial.println("Hz) ===");
  for (int i = 0; i < NUM_TESTS; i++) {
    unsigned long beforeSensorReadTimestamp = results[i];
    unsigned long endOperationTimestamp = results[i+1];
    unsigned long sampleIntervalMicros = endOperationTimestamp - beforeSensorReadTimestamp;
    float realSampleRate = 1.0 / ((float)sampleIntervalMicros / 1000.0) * 1000.0;

    Serial.print(i); Serial.print(";");
    Serial.print(beforeSensorReadTimestamp); Serial.print(";"); Serial.print(endOperationTimestamp); Serial.print(";");
    Serial.print(sampleIntervalMicros); Serial.print(";"); Serial.println(realSampleRate);
  }
  Serial.println("\n\n");
}

// Test 4.
void testPerformanceSensorReadSendUdpPacketOSCMessageDelayRateInMicros(float sampleRate) {
  unsigned long results[NUM_TESTS+1];
  uint32_t samplingIntervalMs = (uint32_t)(1/sampleRate * 1000);

  // -- Test -------------
  int16_t data[SENSOR_DATA_COUNT];
  for (int i = 0; i < NUM_TESTS; i++) {
    results[i] = micros();
    readSensorData(data);
    OSCMessage msg = buildOscMessage(OSC_ADDRESS, data, SENSOR_DATA_COUNT);
    sendOSCMessage(msg);
    msg.empty();
    delay(samplingIntervalMs);
  }
  results[NUM_TESTS] = micros();
  // ---------------------

  // Show results
  Serial.print("=== TEST 4a - Sensor acquisition w/ delay-based sampling rate and transmission (OSC messge size= 56 bytes / sampling= "); Serial.print(sampleRate); Serial.println("Hz) ===");
  for (int i = 0; i < NUM_TESTS; i++) {
    unsigned long beforeSensorReadTimestamp = results[i];
    unsigned long nextSensorReadTimestamp = results[i+1];

    unsigned long sampleIntervalMicros = nextSensorReadTimestamp - beforeSensorReadTimestamp;
    float realSampleRate = 1.0 / ((float)sampleIntervalMicros / 1000.0) * 1000.0;

    Serial.print(i); Serial.print(";");
    Serial.print(beforeSensorReadTimestamp); Serial.print(";"); Serial.print(nextSensorReadTimestamp); Serial.print(";");
    Serial.print(sampleIntervalMicros); Serial.print(";"); Serial.println(realSampleRate);
  }
  Serial.println("\n\n");
}

// ---

void setup() {
  Serial.begin(BAUD);
  delay(SERIAL_STARTUP_DELAY_MS);

  // -- For Test 1a (only), comment this lines here to deactivate the Wifi/UDP functionalities ----------
  isConnectedToNetwork = connectToNetwork(SSID, PASS);
  isConnectedToHost = initUdp();
  // ---------------------------------------------------------------------------------------------
  setupSensor();
  delay(SENSOR_STARTUP_DELAY_MS);

  // -- Test list
  // testPerformanceSensorReadInMicros(); // Test 1 [a) and b)]
  // ---
  // testPerformanceSendUdpPacketInMicros(1); // Test 2a
  // testPerformanceSendUdpPacketInMicros(1472); // Test 2b
  // testPerformanceSendUdpPacketOSCMessageInMicros(); // Test 2c
  // ---
  for (int rateIdx = 0; rateIdx < (sizeof(SAMPLE_RATE) / sizeof(SAMPLE_RATE[0])); rateIdx++) {
    // testPerformanceSensorReadDelayRateInMicros( SAMPLE_RATE[rateIdx] ); // Test 3
    // ...(); // Test 3b
  }
  // ---
  for (int rateIdx = 0; rateIdx < (sizeof(SAMPLE_RATE) / sizeof(SAMPLE_RATE[0])); rateIdx++) {
    // testPerformanceSensorReadSendUdpPacketOSCMessageDelayRateInMicros( SAMPLE_RATE[rateIdx] ); // Test 4
    // ...(); // Test 4b
  }
}

void loop() {}



