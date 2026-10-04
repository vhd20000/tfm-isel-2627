# === Simple Python server ===
from http.server import BaseHTTPRequestHandler, HTTPServer
import urllib.parse


# -- Functions
def update_note(data):
    value = data.get('value', [None])[0]
    if value is None:
        return False
    print(f"Updated note with value={value}")
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
HTTPServer(("0.0.0.0", 8000), Handler).serve_forever()
