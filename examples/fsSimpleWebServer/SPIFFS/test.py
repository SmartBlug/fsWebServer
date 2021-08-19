import http.server
import socketserver
import json

PORT = 80

config = {"ver":"0.0.1","ssid":"your-ssid","pass":"your-password"}
fsfiles = [{"name":"/fs.html","size":2304},{"name":"/formJSON.js","size":2357},{"name":"/params.html","size":1736},{"name":"/default.css","size":3818}]

class NewHandler(http.server.SimpleHTTPRequestHandler):

    def _headers(self, contenttype):
        self.send_response(200)
        self.send_header('Content-type', contenttype)
        self.end_headers()

    def _write(self,message):
        self.wfile.write(message.encode("utf8"))

    def do_GET(self):

        if self.path == '/fs':
            self.path = 'fs.html'
            return http.server.SimpleHTTPRequestHandler.do_GET(self)
        
        elif self.path == '/params':
            self.path = 'params.html'
            return http.server.SimpleHTTPRequestHandler.do_GET(self)

        elif self.path == '/config':
            global config
            self._headers('application/json')
            self._write(json.dumps(config))
            return

        elif self.path == '/files':
            global fsfiles
            self._headers('application/json')
            self._write(json.dumps(fsfiles))
            return

        else:
            return http.server.SimpleHTTPRequestHandler.do_GET(self)

    def do_POST(self):
        
        if self.path == '/saveConfig':
            global config
            content = self.rfile.read(int(self.headers['Content-Length']))
            config = json.loads(content)
            self._headers('application/json')
            self._write('{"result":"ok"}')

        elif self.path == '/delete':
            global fsfiles
            content = self.rfile.read(int(self.headers['Content-Length']))
            currentfile = json.loads(content)
            for i in range(len(fsfiles)):
                if fsfiles[i]['name'] == currentfile['name']:
                    fsfiles.pop(i)
                    break
            self._headers('application/json')
            self._write('{"result":"ok"}')
            return


Handler = NewHandler

with socketserver.TCPServer(("", PORT), Handler) as httpd:
    print("serving at port", PORT)
    httpd.serve_forever()