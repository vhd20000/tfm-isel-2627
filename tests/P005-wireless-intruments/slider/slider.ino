/*
 *  === ESP32-C3 WiFi Direct Connection Test ===
 */

#include <Arduino.h>
#include <WiFi.h>
#include <OSCMessage.h>

#define PIN_POT 1


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
void sendOSCMessage(char msgAddress[], int16_t data=-1) {
  // Build OSC message
  OSCMessage msg(msgAddress);
  msg.add(data);
  
  // Send one UDP datagram to ChucK
  if (udp.beginPacket(HOST, OSC_PORT)) {
    msg.send(udp);

    if (udp.endPacket() == 0) {
      Serial.println("UDP send failed");
    }

    Serial.print("Successfully sent data!!"); Serial.print("data= "); Serial.println(data);
  }

  // Release msg allocated memory
  msg.empty();
}

// === Main cycle
void run() {
  sendOSCMessage("/instrument/pedal", analogRead(PIN_POT));
  delay(SEND_INTERVAL_MS);
}


// === Loops
void setup() {
  Serial.begin(BAUD);
  delay(10);
  
  isConnectedToNetwork = connectToNetwork(SSID, PASS);
  isConnectedToHost = initUdp();
}

void loop() {
  if (!isConnectedToNetwork || !isConnectedToHost) {
    while(1);
  }

  run();
}
