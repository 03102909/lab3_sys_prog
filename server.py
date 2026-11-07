import json
import os
import subprocess
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer

ROOT_DIR = os.path.dirname(os.path.abspath(__file__))
WEB_DIR = os.path.join(ROOT_DIR, "web")
PORT = int(os.environ.get("PORT", 8000))


def find_binary():
    for candidate in (
        os.path.join(ROOT_DIR, "build", "sp_lab3"),
        os.path.join(ROOT_DIR, "cmake-build-debug", "sp_lab3"),
    ):
        if os.path.isfile(candidate) and os.access(candidate, os.X_OK):
            return candidate
    return None


class Handler(SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=WEB_DIR, **kwargs)

    def do_POST(self):
        if self.path != "/parse":
            self.send_error(404)
            return

        length = int(self.headers.get("Content-Length", 0))
        body = self.rfile.read(length)
        try:
            code = json.loads(body.decode("utf-8")).get("code", "")
        except Exception:
            self._send_json(400, {"error": "Invalid JSON request"})
            return

        binary = find_binary()
        if not binary:
            self._send_json(
                500,
                {"error": "Binary not found. Build first with cmake."},
            )
            return

        proc = subprocess.run(
            [binary, "--stdin"],
            input=code,
            text=True,
            capture_output=True,
        )

        if proc.returncode != 0:
            err = proc.stderr.strip() or "Syntax error"
            self._send_json(400, {"error": err})
            return

        try:
            payload = json.loads(proc.stdout)
        except json.JSONDecodeError:
            self._send_json(500, {"error": "Invalid output from parser"})
            return

        self._send_json(200, payload)

    def _send_json(self, status, data):
        raw = json.dumps(data).encode("utf-8")
        self.send_response(status)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(raw)))
        self.end_headers()
        self.wfile.write(raw)


if __name__ == "__main__":
    server = ThreadingHTTPServer(("127.0.0.1", PORT), Handler)
    print(f"Serving at http://127.0.0.1:{PORT}")
    server.serve_forever()
