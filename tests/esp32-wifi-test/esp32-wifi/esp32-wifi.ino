/*
 *  === ESP32 WiFi / ChucK test ===
 *
 *  This sketch sends a message to a HTTP server.
 *  
 *    It has two use modes (also called "PLAY_MODE"):
 *  - INSTRUMENT: On this mode the board reads the analog data from a potenciometer sends it to the HTTP server. That info
 *  is later sent to a ChucK script via OSC to play a note on Sine Oscilator 
 *  - DEBUG_REQUEST: This mode is for DEBUG only. When configured to this mode, the board sends N requests to the HTTP server.
 *  The requests can call 1 out of 3 methods defined on the HTTP server (implemented in the script 'server.py' on the project
 *  directory) and additionally can send a value read from the board's analog pin 4
 */

#include <Arduino.h>
#include <WiFi.h>

#define PIN_BTN 0
#define PIN_POT 1
#define PIN_DEBUG 4

// === Enums
// Play modes (INSTRUMENT - read analog value and send to server/chuck; DEBUG_REQUEST - send N http requests to server)
enum PlayMode {
  INSTRUMENT,
  DEBUG_REQUEST
};

// === Constants
const int BAUD = 115200;

const char *SSID = "ssid";
const char *PASS = "pass";
const char *HOST = "XXX.XXX.XXX.XXX";
const uint16_t PORT = 8000;
const int MAX_NETWORK_CONNECTION_TRIES = 360;
const int NUM_REQUESTS_TOTAL = 10;

const int SETUP_DELAY = 10;  // ms
const int TIME_BETWEEN_CONNECT_TRIES = 500;  // ms
const int TIME_BETWEEN_REQUESTS = 50;  // ms

const int PLAY_MODE = PlayMode::INSTRUMENT;  // THIS FLAG CHANGES CURRENT SCRIPT MODE (INSTRUMENT or DEBUG_REQUEST)

// === Variables
bool isConnectedToNetwork = false;
bool isConnectedToServer = false;
bool stopExecution = false;
NetworkClient client;

// === Functions
// -- Wifi connection
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

void scanNearNetworks() {
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
      Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
      delay(10);
    }
  }
  Serial.println("");
}

// -- Server connection
bool openConnectionWithServer(const char host[], const uint16_t port) {
  Serial.print("\nConnecting to server ( "); Serial.print(host); Serial.println(" )");

  if (!client.connect(host, port)) {
    Serial.println("Connection failed.");
    return false;
  }

  Serial.println("Connected to server.");
  return true;
}

void closeConnectionWithServer() {
  Serial.println("Closing connection.");
  client.stop();
  isConnectedToServer = false;
}

// -- Http request
void sendRequest(char method[5], char action[20], char parameters[12] = "") {
  printHeap("Before request");

  // -- Open connection w/ server
  isConnectedToServer = openConnectionWithServer(HOST, PORT);
  if (!isConnectedToServer) {
    printHeap("Connection failed");
    return;
  }

  // -- Send request
  char httpRequest[70];
  snprintf(
    httpRequest,
    sizeof(httpRequest),
    "%s %s%s HTTP/1.1\r\n"
    "Host: %s\r\n\r\n",
    method,
    action,
    parameters,
    HOST
  );

  client.print(httpRequest);

  printHeap("After connect");

  // -- Close connection w/ server
  closeConnectionWithServer();
}

// -- Read data
int readAnalogData(int pin) {
  return analogRead(pin);
}

int readPotenciometer(int threshold = 100) {
  return analogRead(PIN_BTN) > threshold ? analogRead(PIN_POT) : -1;
}

// -- Play back modes
void buildParameters(char* buffer, size_t bufferSize, int data) {
  snprintf(
    buffer,
    bufferSize,
    "?value=%d",
    data
  );
}

void instrumentMode() {
  char parameters[12];
  buildParameters(parameters, sizeof(parameters), readPotenciometer());

  int oldTimeStamp = millis();
  sendRequest("GET", "/update_note", parameters);
  int newTimeStamp = millis();
  Serial.print("-> request time= "); Serial.print(newTimeStamp-oldTimeStamp); Serial.println(" ms");
  delay(TIME_BETWEEN_REQUESTS);
}

void requestDebugMode() {
  Serial.print("\nTest started\nSending "); Serial.print(NUM_REQUESTS_TOTAL); Serial.println(" requests to server ...\n");
  for (int i = 0; i < NUM_REQUESTS_TOTAL; i++) {
    Serial.print("====== "); Serial.print("REQUEST Nº"); Serial.println(i+1);

    char parameters[12];
    buildParameters(parameters, sizeof(parameters), readAnalogData(4));
    sendRequest("GET", "/update_note", parameters);

    Serial.println("\n\n");
    delay(TIME_BETWEEN_REQUESTS);
  }
  stopExecution = true;
  Serial.println("\n======   Program execution stopped   ======");
}

void printHeap(const char* label) {
  Serial.printf(
    "[%lu ms] %s | Free: %u B | Min free: %u B | Largest block: %u B\n",
    millis(),
    label,
    ESP.getFreeHeap(),
    ESP.getMinFreeHeap(),
    ESP.getMaxAllocHeap()
  );
}

// === Loops
void setup() {
  Serial.begin(BAUD);
  delay(SETUP_DELAY);

  isConnectedToNetwork = connectToNetwork(SSID, PASS);
}

void loop() {
  if (!isConnectedToNetwork || stopExecution) {
    while(1);
  }

  // -- Connect to Python server and act
  if (PLAY_MODE == PlayMode::INSTRUMENT) {
    instrumentMode();
  } else if (PLAY_MODE == PlayMode::DEBUG_REQUEST) {
    requestDebugMode();
  }
}
