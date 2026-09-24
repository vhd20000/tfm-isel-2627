// ===   Arduino Connection Test   ===

// This script serves to test the connection between an Arduino
// board and ChucK

// This script reads the analog value of a potenciometer, only
// when a button is pressed (check system connections on circuit.png) 

// =============================
// Constants / Variables

const int BAUD = 9600;
const int POTENC_PIN = A0;
const int BUTTON_PIN = A1;

bool isButtonPressed = 0;
int potentiometer = 0;
int loopDelay = 10;  // ms

// =============================
// Script loops

void setup() {
  Serial.begin(BAUD);
  pinMode(POTENC_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT);
}

void loop() {
  potentiometer = analogRead(POTENC_PIN);
  isButtonPressed = analogRead(BUTTON_PIN) > 100;
  Serial.println(isButtonPressed ? potentiometer : -1);
  delay(loopDelay);
}