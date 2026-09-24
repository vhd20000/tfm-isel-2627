// ===   Double Voice Test   ===

// This script serves as a test of ChucK's capability of
// playing multiple simultaneos voices, using MIDI notes
// 
// Prelude No.2 in C Minor (BWV 847), from the Well Tempered Clavier Book I - J. S. Bach
// (source: https://musescore.com/classicman/scores/227966)

// =============================

// Tempo
108 => int tempo;
4 => int timeDivision; // default = 4
60::second / tempo / timeDivision => dur beat;

// Voices (oscilators)
SinOsc leadOsc => dac;
0.25 => leadOsc.gain;
SinOsc counterOsc => dac;
0.10 => counterOsc.gain;

// Notes (4-first bars)
[
    72, 63, 62, 63, 60, 63, 62, 63, 72, 63, 62, 63, 60, 63, 62, 63,
    68, 65, 64, 65, 60, 65, 64, 65, 68, 65, 64, 65, 60, 65, 64, 65,
    71, 65, 63, 65, 62, 65, 63, 65, 71, 65, 63, 65, 62, 65, 63, 65,
    72, 67, 65, 67, 63, 67, 65, 67, 72, 67, 65, 67, 63, 67, 65, 67,
] @=> int lead[];
[
    48, 55, 53, 55, 51, 55, 53, 55, 48, 55, 53, 55, 51, 55, 53, 55,
    48, 56, 55, 56, 53, 56, 55, 56, 48, 56, 55, 56, 53, 56, 55, 56,
    48, 56, 55, 56, 53, 56, 55, 56, 48, 56, 55, 56, 53, 56, 55, 56,
    48, 51, 50, 51, 55, 51, 50, 51, 48, 51, 50, 51, 55, 51, 50, 51,
] @=> int counter[];

// Play loop
for (0 => int i; i < lead.cap(); i++) {
    Std.mtof(lead[i]) => leadOsc.freq;
    Std.mtof(counter[i]) => counterOsc.freq;
    beat => now;
}