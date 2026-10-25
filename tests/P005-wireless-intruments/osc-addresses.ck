public class OscAddresses {
    // Address list
    "/instrument/slider" => static string SLIDER;
    "/instrument/pedal" => static string PEDAL;

    fun static string[] getAllAddresses() {
        string addresses[2];
        SLIDER => addresses[0];
        PEDAL => addresses[1];
        return addresses;
    }
}
