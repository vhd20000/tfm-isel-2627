// ===   Arduino Connection Test   ===

// This script serves to test the connection between an Arduino
// board and ChucK

// This script receives an analog value from Arduino and maps
// it to a pitch / note

// =============================
// --- Constants

// Serial input values
0 => int MIN_SERIAL_INPUT;
1024 => int MAX_SERIAL_INPUT;
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

// =============================
// --- Variables

// Serial
SerialIO serial;

// Voices (oscilators)
SinOsc leadOsc => dac;
0.2 => leadOsc.gain;

// Playback flag
false => int playPitch;

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
// --- Open device connection

0 => int device;
SerialIO.list() @=> string list[];
if( !serial.open(device, SerialIO.B9600, SerialIO.ASCII) )
{
    <<< "ERROR - unable to open serial device" >>>;
	me.exit();
}

// =============================
// --- Infinite time-loop

while(true)
{
    // Handle serial msg
    serial.onLine() => now;
    serial.getLine() => string line;
    Std.atoi(line) => int value;

    // <<< value >>>;
    
    // Control pitch
    if (value < 0) {
        0 => leadOsc.freq;
    }
    else {
        // calcPitch(value) => leadOsc.freq;
        calcNoteInScale(value, cMinPentScale) => leadOsc.freq;
    }
}