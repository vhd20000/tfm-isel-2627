// ===   ESP32-C3 Wireless Instrument Test   ===
// 
// It controls a digital intrument through OSC messages
// sent over UDP
// 
// This script contains the main app execution cycle
// (controls the ChucK VM)


@import "osc-addresses.ck"
@import "pedal/Pedal.ck"


// =============================
// --- CONSTANTS

6449 => int OSC_PORT;


// =============================
// --- VARIABLES

OscIn oin;
OscMsg msg;
Pedal pedalInstrument;


// =============================
// --- FUNCIONTS
fun void run() {
    setupOSC();

    while(true)
    {
        // Open OSC connection
        oin => now;

        // Handle OSC msgs
        while( oin.recv(msg) )
        {
            // Pedal
            if (msg.address == OscAddresses.PEDAL) {
                msg.getInt(0) => int value;
                pedalInstrument.play(value);
                <<< "Address:", OscAddresses.PEDAL, "value:", value >>>;
            }
            // Slider
            else if (msg.address == OscAddresses.SLIDER) {
                msg.getInt(0) => int value;
                <<< "Address:", OscAddresses.SLIDER, "value:", value >>>;
            }
            // GY-85 test (movement sensor)
            else if (msg.address == OscAddresses.GY85) {
                msg.getInt(0) => int accX;
                msg.getInt(1) => int accY;
                msg.getInt(2) => int accZ;

                msg.getInt(3) => int gyrX;
                msg.getInt(4) => int gyrY;
                msg.getInt(5) => int gyrZ;

                msg.getInt(6) => int magX;
                msg.getInt(7) => int magY;
                msg.getInt(8) => int magZ;
                
                "" => string receivedData;
                receivedData + "ACC  X: " + accX + " Y: " + accY + " Z: " + accZ + " | " => receivedData;
                receivedData + "GYR  X: " + gyrX + " Y: " + gyrY + " Z: " + gyrZ + " | " => receivedData;
                receivedData + "MAG  X: " + magX + " Y: " + magY + " Z: " + magZ => receivedData;
                <<< "Address:", OscAddresses.GY85, " | ", receivedData >>>;
            }
            // else if () { ... }
        }
    }
}

fun void setupOSC() {
    OSC_PORT => oin.port;
    OscAddresses.getAllAddresses() @=> string addressess[];
    for (0 => int i; i < addressess.size(); i++) {
        oin.addAddress(addressess[i]);
    }
}


// =============================
// --- MAIN

run();
