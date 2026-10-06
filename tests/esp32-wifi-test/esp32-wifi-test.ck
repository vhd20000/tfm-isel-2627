// ===   ESP32-C3 WiFi Server Connection Test   ===

// This script serves to test the connection between an ESP32-C3
// board and ChucK.

// This script receives analog data from the board (sent from the
// ESP32-C3 board to a HTTP server and then transmitted to ChucK
// via OSC messages) and maps it to a pitch / note

// This script has two modes (also called "PLAY_MODE"):
//  - PITCH: On this mode it plays a continues tone, calculating
//  the pitch in real-time based on the value received on the
//  OSC messages 
//  - SCALE: On this mode it plays a note on a pre-configured
//  scale; based on the received value it determines the note
//  to play

// =============================
// --- Constants

// Play mode (PITCH - 0 / SCALE - 1)
1 => int PLAY_MODE;

// OSC
8001 => int OCS_PORT;

// Serial input values
0 => int MIN_SERIAL_INPUT;
4095 => int MAX_SERIAL_INPUT;
MAX_SERIAL_INPUT - MIN_SERIAL_INPUT => int RANGE_SERIAL_INPUT;

// Pitches
220 => int MIN_FREQ;
880 => int MAX_FREQ;
MAX_FREQ - MIN_FREQ => int RANGE_FREQ;

// Notes / scales
[60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72] @=> int notes[];
[60, 62, 64, 65, 67, 69, 71, 72] @=> int cMajScale[];
[60, 62, 63, 65, 67, 68, 70, 72] @=> int cMinScale[];
[60, 62, 63, 65, 67, 68, 71, 72] @=> int cMinHarmScale[];
[60, 62, 64, 67, 69, 72, 74, 76, 79, 81, 84] @=> int cMajPentScale[];
[60, 63, 65, 67, 70, 72, 75, 77, 79, 82, 84] @=> int cMinPentScale[];
cMinPentScale @=> int SYSTEM_SCALE[];

// =============================
// --- Variables

// OSC
OscIn oin;
OscMsg msg;

// Voices (oscilators)
SinOsc leadOsc => dac;
0.25 => leadOsc.gain;
0 => leadOsc.freq;

// Device to use
0 => int device;

// =============================
// --- Functions

fun int calcPitch(int value) {
    return (((value - MIN_SERIAL_INPUT) * RANGE_FREQ) / RANGE_SERIAL_INPUT) + MIN_FREQ;
}

fun float calcNoteInScale(int value, int scale[]) {
    MAX_SERIAL_INPUT / (scale.cap()-1) => int split;
    scale[value / split] => int note;
    return Std.mtof(note);
}

// =============================
// --- Setup OSC

OCS_PORT => oin.port;
oin.addAddress("/value");

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

        // Control pitch
        if (value < 0) {
            0 => leadOsc.freq;
        }
        else {
            if (PLAY_MODE == 0) {   // PITCH MODE (play a continues pitch)
                calcPitch(value) => leadOsc.freq;
            } else if (PLAY_MODE == 1) {    // SCALE MODE (play a note from a pre-configured scale)
                calcNoteInScale(value, SYSTEM_SCALE) => leadOsc.freq;
            }
        }
    }
}