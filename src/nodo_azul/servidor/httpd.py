import http.server, sys

# (host, port) where this HTTP server will be listening to

#ip = sys.args[1]
puertoServidor = int(sys.argv[1])
datosWebSocket = sys.argv[2]

ADDRESS = ('', puertoServidor)

# Source: https://developer.mozilla.org/en-US/docs/Web/HTTP/Basics_of_HTTP/MIME_types/Common_types
CONTENT_TYPES = {
	'css': 'text/css',
	'csv': 'text/csv',
	'htm': 'text/html',
	'html': 'text/html',
	'jpeg': 'image/jpeg',
	'jpg': 'image/jpeg',
	'js': 'text/javascript',
	'json': 'application/json',
	'png': 'image/png',
	'pdf': 'application/pdf',
	'svg': 'image/svg+xml',
	'txt': 'text/plain',
	'xhtml': 'application/xhtml+xml',
	'xml': 'application/xml',
}

BINARY_TYPES = ('jpeg', 'jpg', 'png', 'pdf')

class RequestManager(http.server.BaseHTTPRequestHandler):
	"""Respond to HTTP requests"""
	def do_GET(self):
		print('GET', self.path)
		path = self.path
		if path == '/': 			# Define root as index file
			path = 'index.xhtml'
		if path[0] == '/':			# Convert absolute to relative path
			path = path[1:]
		print('path', path)
		try:
			extension = self.findExtension(path)
			mode = 'r'
			if extension in BINARY_TYPES:
				mode = 'rb'
			file = open(path, mode).read()
			self.send_response(200)
		except:
			file = "Not found"
			self.send_response(404)
		self.setContentType(path)
		self.end_headers()
		if extension in BINARY_TYPES:
			self.wfile.write(file)
		else:
			self.wfile.write(bytes(file, 'utf-8'))

	def setContentType(self, path):
		extension = self.findExtension(path)
		if extension in CONTENT_TYPES:
			self.send_header("Content-Type", CONTENT_TYPES[extension])
			print("Content-Type:", CONTENT_TYPES[extension])

	def findExtension(self, path):
		pos = path.rfind('.')
		extension = ''
		if pos != -1:
			extension = path[pos + 1:]
		return extension

def escribirDatosWebSocket(dato1, dato2):
	fd = open('index.xhtml', 'r')
	archivo = fd.read()
	posicion = archivo.find(dato1)
	archivo = archivo[:posicion] +dato2+ archivo[posicion+len(dato1):]
	fd.close()
	fd = open('index.xhtml', 'w')
	fd.write(archivo)
	fd.close()

try:
	# Create a HTTP server and make it to listen HTTP requests
	escribirDatosWebSocket('${ws_data}', datosWebSocket)
	httpd = http.server.HTTPServer(ADDRESS, RequestManager)
	print("HTTP Server listening at", ADDRESS)
	httpd.serve_forever()
except KeyboardInterrupt:
	print('HTTP Server finished')
	escribirDatosWebSocket(datosWebSocket, '${ws_data}')
httpd.server_close()
