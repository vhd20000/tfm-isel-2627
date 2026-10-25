public class OscAddresses {
    // Address list
    "/instrument/slider" => static string SLIDER;
    "/instrument/pedal" => static string PEDAL;
    "/instrument/gy85" => static string GY85;

    fun static string[] getAllAddresses() {
        string addresses[3];
        SLIDER => addresses[0];
        PEDAL => addresses[1];
        GY85 => addresses[2];
        return addresses;
    }
}
