const int BAUD = 115200;
const int POT_PIN = 1;
int val;

void setup() {
  Serial.begin(BAUD);
}

void loop() {
  val = analogRead(1);
  Serial.println(val);
  delay(200);
}
