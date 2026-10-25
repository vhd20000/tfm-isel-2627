This is a test for the wireless communication between a microcontroller and ChucK over a Python server.

The microcontroller reads analog data from a potentiometer and transmits the sensor values over Wi-Fi. The data is received by a Python server that forwards it to ChucK to control the audio playback, mapping the input to a pitch or a note (depending on the mode used).

The project is organized into three main components:

ESP32-C3: Reads the analog input and transmits sensor data over the network
Python server: Receives data from the microcontroller and forwards it to ChucK via OSC
ChucK: Receives the sensor data and translates it into musical output
