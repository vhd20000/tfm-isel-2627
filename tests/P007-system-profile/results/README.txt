
	=== ESP32-C3 Instrument Performance Tests ===

  > This directory contains the results of the performance tests conducted on the system developed on the P005_Wireless_DirectConnection_Instruments (both hardware and software).
  
  > This directory contains the results of the performance tests conducted on the system developed for P005_Wireless_DirectConnection_Instruments (both hardware and software). The results are available in the "results" subdirectory in .csv format.

  > The system is composed of an ESP32-C3 Super Mini development board and a GY-85 IMU module (for more detail, check the system diagram on Basecamp in "/Docs&Files/Diagrams/system_v2 (gy85).png")

  > The ESP32-C3 communicates with the GY-85 through I2C and transmits sensor data to a remote host using Wifi and UDP/OSC

  > All tests were performed with the system connected to the PC via USB

  > System configuration used for the tests:
	- I2C clock frequency: 100 kHz
	- Serial baud rate: 115200
	- Sensor startup delay: 100 ms
	- Number of iterations: 100 per test
	- Wifi configuration: ESP32-C3 operating in station mode (STA)
	- Communication protocol: UDP with OSC message encoding
	- Wifi transmit power: 8.5 dBm
	- Sensor configuration: default register configurations defined in the code for the ADXL345, ITG-3205 and HMC5883L


===============


		Test List:

  - Test 1: Measure the sensor acquisition time (GY-85)
	a) Without Wifi connection
	b) While connected to Wifi and with UDP initialized

  - Test 2: Measure the UDP transmission time 
	a) With a 1-byte payload.
	b) With a 1472-byte payload (1500 bytes total, including IPv4 and UDP headers)
	c) Using an OSC message containing nine sensor values (similar to what is used on P005)

  - Test 3: Evaluate the sensor acquisition cadence using delay-based sampling, without data transmission
  
  - Test 4: Evaluate the sensor acquisition cadence using delay-based sampling, including UDP/OSC transmission


  For Tests 3 and 4, the target sampling rates used were 50, 100, 500 and 1000 Hz.
  The acquisition timestamps were recorded using micros(), and the time calculated afterwards, during the print of the results.