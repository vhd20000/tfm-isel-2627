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
            else if (msg.address == OscAddresses.PEDAL) {
                msg.getInt(0) => int value;
                pedalInstrument.play(value);
                <<< "Address:", OscAddresses.PEDAL, "value:", value >>>;
            }
            // Slider
            if (msg.address == OscAddresses.SLIDER) {
                msg.getInt(0) => int value;
                <<< "Address:", OscAddresses.SLIDER, "value:", value >>>;
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
