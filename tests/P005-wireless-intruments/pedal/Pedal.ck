public class Pedal {
    TriOsc osc => dac;

    0 => osc.gain;
    0 => osc.freq;

    // Set pitch from a sensor value (0-4095)
    fun float setPitch(int value) {
        if (value == -1) {
            return 0;
        }

        Std.scalef(value, 0, 4095, 36, 60) => float scaledValue;
        return Std.mtof(scaledValue);
    }

    fun void play(int value) {
        this.setPitch(value) => osc.freq;
        0.2 => osc.gain;
    }

    fun void stop() {
        0 => osc.gain;
    }
}
