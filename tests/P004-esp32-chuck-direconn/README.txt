This is a test for the connection between a micro-controller board and ChucK, without using a server (for example, a simple HTTP one implemented using Python)

The micro-controller first connects to a Wi-Fi network. Once connected, it reads the analog signal from a potentiometer and transmits the data directly to ChucK as OSC messages over UDP
