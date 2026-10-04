/*
 *  === WiFi test ===
 *
 *  This sketch sends a message to a HTTP server
 *
 */

#include <Arduino.h>
#include <WiFi.h>

// === Constants
const int BAUD = 9600;

const char *SSID = "ssid";
const char *PASS = "pass";
const char *HOST = "XXX.XXX.XXX.XXX";
const uint16_t PORT = 8000;
const int MAX_TRIES = 360;

const int TIME_BETWEEN_TRIES = 500; // ms

const String SPACE = String(" ");
const String NEW_LINE = String("\r\n");
const String REQUEST_PROTOCOL = String("HTTP/1.1");

// === Variables
bool isConnectedToNetwork = false;
bool isConnectedToServer = false;
bool stopExecution = false;
NetworkClient client;

// === Functions
bool connectToNetwork(const char* ssid, const char* pass) {
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  // Connect to Wi-Fi network
  Serial.print("Connecting to "); Serial.println(ssid);
  WiFi.begin(ssid, pass);

  // Wait until connection is established, stop if MAX_TRIES is exceeded
  int retryCount = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if (retryCount > MAX_TRIES) {
      Serial.print("Unable to connect to "); Serial.println(ssid);
      Serial.println("Please try again");
      return false;
    }

    delay(TIME_BETWEEN_TRIES);
    Serial.print(".");
    retryCount++;
  }

  // Connection successful
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP address: "); Serial.println(WiFi.localIP());
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

bool openConnectionWithServer(const char *host, const uint16_t port) {
  Serial.print("Connecting to "); Serial.println(host);

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

String formatRequest(char *method, String request, const char *host, String parameters) {
  return method + SPACE + request + parameters + SPACE + REQUEST_PROTOCOL + NEW_LINE + 
  "Host: " + host + NEW_LINE + NEW_LINE; 
}

void sendRequest(char *method, String action, String parameters = "") {
  // -- Open connection w/ server
  isConnectedToServer = openConnectionWithServer(HOST, PORT);
  if (!isConnectedToServer) {
    return;
  }

  // -- Send request
  String req = formatRequest(method, action, HOST, parameters);
  client.print(req);

  // //wait for the server's reply to become available
  // int maxloops = 0;
  // while (!client.available() && maxloops < 1000) {
  //   maxloops++;
  //   delay(1);
  // }

  // if (client.available() > 0) {
  //   String line = client.readStringUntil('\r');
  //   Serial.println(line);
  // } else {
  //   Serial.println("client.available() timed out ");
  // }
  
  // -- Close connection w/ server
  closeConnectionWithServer();
}

// === Loops
void setup() {
  Serial.begin(BAUD);
  delay(10);

  isConnectedToNetwork = connectToNetwork(SSID, PASS);
}

void loop() {
  if (!isConnectedToNetwork || stopExecution) {
    while(1);
  }

  // -- Connect to Python server
  Serial.println("\n\n");
  for (int i = 0; i < 10; i++) {
    Serial.print("=========== "); Serial.print("REQUEST Nº"); Serial.println(i+1);

    // String request = random(2) ? String("/change_note_up") : String("/change_note_down");
    String request = "/update_note";
    String parameters = String("?value=") + String( analogRead(0) );
    sendRequest("GET", request, parameters);

    Serial.println("\n\n");
    delay(5000);
  }
}
