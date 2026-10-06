"""
    ===   ESP32-C3 WiFi / ChucK Connection Test   ===

# === Simple Python HTTP server ===

This script implements a simple Python HTTP server, serving
as the bridge between an ESP32-C3 board and a ChucK script

The server receives analog data from the board via WiFi and
transmits it to ChucK via OSC messages
"""


from http.server import BaseHTTPRequestHandler, HTTPServer
from pythonosc.udp_client import SimpleUDPClient
import urllib.parse


# -- Contacts
HTTP_ADDRESS = ("0.0.0.0", 8000)
OSC_ADDRESS = ("127.0.0.1", 8001)


# -- Functions
def send_osc_message(osc_address, message, value):
    with SimpleUDPClient(*osc_address) as client:
        client.send_message(message, value)


def update_note(data):
    value = data.get('value', [None])[0]
    if value is None:
        return False
    # print(f"Updated note with value={value}")
    send_osc_message(OSC_ADDRESS, "/value", int(value))
    return True


def change_note_up(data):
    print("Note change UP")
    return True


def change_note_down(data):
    print("Note change DOWN")
    return True


# -- Map
get_requests_map = {
    "/change_note_up": change_note_up,
    "/change_note_down": change_note_down,
    "/update_note": update_note,
}


# -- Http Server
class Handler(BaseHTTPRequestHandler):
    def do_GET(self):
        status_code = 200

        # Process request
        parsed_path = urllib.parse.urlparse(self.path)
        data = urllib.parse.parse_qs(parsed_path.query)

        # Execute request
        action = get_requests_map.get(parsed_path.path, None)
        could_execute_action = False
        if action is not None:
            could_execute_action = action(data)
        else:
            status_code = 404

        # Response
        status_code = 500 if not could_execute_action and status_code != 404 else status_code
        self.send_response(status_code)
        self.send_header("Content-Type", "text/plain")
        self.end_headers()


# -- Run server
HTTPServer(HTTP_ADDRESS, Handler).serve_forever()
