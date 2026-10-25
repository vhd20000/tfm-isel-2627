/*
 *  === ESP32-C3 WiFi Direct Connection Test ===
 */

#include <Arduino.h>
#include <WiFi.h>
#include <OSCMessage.h>
#include <Wire.h>

#define PIN_SDA 8
#define PIN_SCL 9


// === Constants
const int BAUD = 115200;

const char SSID[] = "ssid";
const char PASS[] = "pass";
const char HOST[] = "XXX.XXX.XXX.XXX";
const uint16_t UDP_PORT = 8888;
const uint16_t OSC_PORT = 6449;

const uint16_t MAX_NETWORK_CONNECTION_TRIES = 360;
const uint16_t TIME_BETWEEN_CONNECT_TRIES = 500;  // ms
const uint16_t SEND_INTERVAL_MS = 20;  // send every 20ms = at 50Hz

const uint8_t ACCEL_ADDR = 0x53; // ADXL345
const uint8_t GYRO_ADDR  = 0x68; // ITG3200/ITG3205
const uint8_t MAG_ADDR   = 0x1E; // HMC5883L


// === Variables
bool isConnectedToNetwork = false;
bool isConnectedToHost = false;
NetworkClient client;
WiFiUDP udp;


// === Connection
// -- Wi-Fi
bool connectToNetwork(const char ssid[], const char pass[]) {
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Connect to Wi-Fi network
  Serial.print("Connecting to "); Serial.println(ssid);
  WiFi.begin(ssid, pass);
  WiFi.setTxPower(WIFI_POWER_8_5dBm);

  // Wait until connection is established, stop if MAX_NETWORK_CONNECTION_TRIES is exceeded
  int retryCount = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if (retryCount > MAX_NETWORK_CONNECTION_TRIES) {
      Serial.print("Unable to connect to "); Serial.println(ssid);
      Serial.println("Please try again");
      return false;
    }

    delay(TIME_BETWEEN_CONNECT_TRIES);
    Serial.print(".");
    retryCount++;
  }

  // Connection successful
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("Device IP address: "); Serial.println(WiFi.localIP());
  return true;
}

// -- UDP
bool initUdp() {
  if (udp.begin(UDP_PORT) == 0) {
    Serial.println("Failed to initialize UDP");
    Serial.println("Please try again");
    return false;
  }

  Serial.println("UDP initialized");
  return true;
}

// -- OSC
void sendOSCMessage(char msgAddress[], int16_t data[], size_t count) {
  // Build OSC message
  OSCMessage msg(msgAddress);
  for (int i = 0; i < count; i++) {
    msg.add(data[i]);
  }
  
  // Send one UDP datagram to ChucK
  if (udp.beginPacket(HOST, OSC_PORT)) {
    msg.send(udp);

    if (udp.endPacket() == 0) {
      Serial.println("UDP send failed");
    }

    Serial.println("Successfully sent data!!");
  }

  // Release msg allocated memory
  msg.empty();
}


// === GY-85 Sensor functions
void setupSensor() {
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

// Combine bytes in little-endian order (ADXL345)
int16_t littleEndian(uint8_t low, uint8_t high) {
  return (int16_t)((uint16_t)high << 8 | low);
}

// Combine bytes in big-endian order (gyro and magnetometer)
int16_t bigEndian(uint8_t high, uint8_t low) {
  return (int16_t)((uint16_t)high << 8 | low);
}

bool readSensorData(int16_t data[9]) {
  uint8_t rawData[6];
  bool accOk = false;
  bool gyroOk = false;
  bool magOk = false;

  // Accelerometer
  if (readRegisters(ACCEL_ADDR, 0x32, rawData, 6)) {
    int16_t ax = littleEndian(rawData[0], rawData[1]);
    int16_t ay = littleEndian(rawData[2], rawData[3]);
    int16_t az = littleEndian(rawData[4], rawData[5]);
    data[0] = ax; data[1] = ay; data[2] = az;
    accOk = true;
  }

  // Gyroscope
  if (readRegisters(GYRO_ADDR, 0x1D, rawData, 6)) {
    int16_t gx = bigEndian(rawData[0], rawData[1]);
    int16_t gy = bigEndian(rawData[2], rawData[3]);
    int16_t gz = bigEndian(rawData[4], rawData[5]);
    data[3] = gx; data[4] = gy; data[5] = gz;
    gyroOk = true;
  }

  // Magnetometer: register order is X, Z, Y
  if (readRegisters(MAG_ADDR, 0x03, rawData, 6)) {
    int16_t mx = bigEndian(rawData[0], rawData[1]);
    int16_t mz = bigEndian(rawData[2], rawData[3]);
    int16_t my = bigEndian(rawData[4], rawData[5]);
    data[6] = mx; data[7] = mz; data[8] = my;
    magOk = true;
  }

  return accOk && gyroOk && magOk;
}


// === Main cycle
void run() {
  int16_t sensorData[9];
  if(readSensorData(sensorData)) {
    sendOSCMessage("/instrument/gy85", sensorData, 9);
  }
  else {
    Serial.println("Sensor read failed, skipping OSC message");
  }
  delay(SEND_INTERVAL_MS);
}


// === Loops
void setup() {
  Serial.begin(BAUD);
  delay(10);
  
  // Handle connection
  isConnectedToNetwork = connectToNetwork(SSID, PASS);
  isConnectedToHost = initUdp();

  // Setup sensor reading
  setupSensor();
  delay(100);
}

void loop() {
  if (!isConnectedToNetwork || !isConnectedToHost) {
    while(1);
  }

  run();
}
