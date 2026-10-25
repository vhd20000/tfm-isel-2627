// ===   ESP32-C3 WiFi Direct Connection Test   ===


// =============================
// --- CONSTANTS

// OSC
6449 => int OSC_PORT;
"/test" => string OSC_ADDRESS;


// =============================
// --- Variables

// OSC
OscIn oin;
OscMsg msg;


// =============================
// --- Setup OSC

OSC_PORT => oin.port;
oin.addAddress(OSC_ADDRESS);


// =============================
// --- Infinite time-loop
while(true)
{
    // Open OSC connection
    oin => now;

    // Handle OSC msgs
    while( oin.recv(msg) )
    {
        msg.getInt(0) => int value;
        <<< value >>>;
    }
}