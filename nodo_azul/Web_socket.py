#!/usr/bin/env python3

from autobahn.twisted.websocket import WebSocketServerProtocol, \
	WebSocketServerFactory
from twisted.python import log
from twisted.internet import reactor
import threading, sys, codecs, os, random, time, queue

CODIGO = '-1::Salir'.encode('ASCII', 'ignore')
LONGITUD = 207
conexiones = []

class WebSocketServer(WebSocketServerProtocol):
	
	'''
	brief Muestra que se conectó un cliente
	details Se imprime en salida estandar cada vez que se conecta un
	nuevo cliente
	param[in] request: brinda información específica como la ip del 
	cliente
	author Jeisson Hidalgo
	date 28-09-20
	'''
	def onConnect(self, request):
		print(f"Client connecting: {request.peer}")
		conexiones.append(self)
	
	'''
	brief Envía la información propia y la de los vecinos
	details Se imprime en salida estandar un mensaje de confirmación
	además de enviarle a la página web la información del ID propio y
	los IDs de los vecinos. Aparte de esto llama otra subrutina que
	genera mensajes de prueba
	author Jeisson Hidalgo
	date 28-09-20
	'''
	def onOpen(self):
		print("WebSocket connection open.")
		self.sendMessage(f"id\t{ids[0]}".encode('utf8'), False)
		print("Envié # nodo")
		idVecinos = 'vecinos'
		for i in range(1, len(ids)):
			idVecinos += ' ' + ids[i]
		self.sendMessage(f"{idVecinos}".encode('utf8'), False)
		print("Envié # vecinos")

	'''
	brief Imprime en salida estandar los mensajes que llegan
	details Esta subrutina impreme información de los mensajes que
	llegan
	param[in] payload: Es el cuerpo del mensaje
	isBinary: Nos dice si el mensaje es binario o no
	author Jeisson Hidalgo
	date 28-09-20
	'''
	def onMessage(self, payload, isBinary):
		if isBinary:
			print(f"Binary message received: {len(payload)} bytes")
		else:
			print(f"Text message received: {payload.decode('utf8')}")
		salidas.put(payload)

	'''
	brief Imprime en salida estandar cuando se cierra una conexión
	details Esta subrutina imprime un mensaje cuando un cliente cierra 
	el navegador.
	author Jeisson Hidalgo
	date 28-09-20
	'''
	def onClose(self, wasClean, code, reason):
		print(f"WebSocket connection closed: {reason}")

	'''
	brief Envía mensajes a la página web
	details Esta subrutina envía mensajes a la página web por mwdio del
	web socket, para que el usuario pueda visualizarlo
	author Johel Phillips
	date 05-10-20
	'''
	def enviarMensaje(self, mensaje):
		mensaje = mensaje.decode('utf8')
		self.sendMessage(f'{mensaje}'.encode('utf8'), False)


'''
 *	@brief Subrutina para enviar mensajes
 *	@details Esta subrutina utiliza uno de los pipes para enviarle
 *  los mensajes al agente azul, el cual se encarga de pasar este
 *  mensaje al nodo verde
 *	@param[in] [1] evento: Este evento es quien se encarga de parar
 *  la subrutina (evento.wait()) para evitar el busy waiting, cuando se
 *  quiere enviar un mensaje el hilo principal hace un evento.set() 
 *	[2] salidas: Es una lista de objetos Mensaje, para que los mensajes
 *  no se pierdan en el caso de que se requiera enviar muchos seguidos
 *	[3] mutex: Este mutex evita que hayan conflictos en la lista de 
 *  mensajes
 *	[4] pipe: Ruta para el pipe Servidor -> Agente
 *	@author Johel Phillips Ugalde B75821
 *	@date 29-09-20
 '''
def enviar():

	fd = os.open(pipeSalida, os.O_WRONLY)

	while True:
		mensaje = salidas.get(block = True)
		salidas.task_done()
		if len(mensaje) >= LONGITUD:
			mensaje = mensaje[:LONGITUD-1]
		os.write(fd, mensaje + '\x00'.encode('ASCII', 'ignore'))
		time.sleep(0.01)
		if(mensaje.find(CODIGO) != -1):
			break
	os.close(fd)

'''
 *	@brief Subrutina para recibir mensajes
 *	@details Esta subrutina utiliza uno de los pipes para recibir
 *  los mensajes provenientes de otro usuario conectado a un nodo
 *  vecino
 *	@param[in] [1] entradas: Es una lista de objetos Mensaje, para que
 *  los mensajes que lleguen no se peirdan hasta que el usuario los lea
 *	[3] mutex: Este mutex evita que hayan conflictos en la lista de 
 *  mensajes
 *	[4] pipe: Ruta para el pipe Agente -> Servidor
 *	@author Johel Phillips Ugalde B75821
 *	@date 29-09-20
 '''
def recibir():
	
	final = '\x00'.encode('ASCII', 'ignore')
	fd = open(pipeEntrada, 'br')

	while True:
		mensaje = fd.read(LONGITUD)
		time.sleep(0.01)
		if(mensaje[2] != "'"):
			if(mensaje.find(CODIGO[:3]) != -1):
				reactor.callFromThread(reactor.stop)
				break
			posicion = mensaje.find(final)
			mensaje = mensaje[:posicion]
			conexiones[0].enviarMensaje(mensaje)
	fd.close()

'''
 *	@brief Subrutina para almacenar la información de los nodos
 *	@details Esta subrutina divide las datos que entran como tercer
 *	argumento del programa, para tener acceso a los ids de los nodos
 *	vecinos y el id propio
 *	@param[in] [1] Lista: Esta lista almacenará los datos de los ids
 *	[2] String: Hilera que contiene los ids de los nodos
 *	@author Johel Phillips Ugalde B75821
 *	@date 29-09-20
 '''
def almacenarIDs(ids, nodos):
	len = nodos.count(',')
	for i in range(len):
		pos = nodos.find(',')
		ids.append(nodos[:pos])
		nodos = nodos[pos+1:]

if __name__ == '__main__':

	pipeEntrada = sys.argv[1]
	pipeSalida = sys.argv[2]
	nodos = sys.argv[3]
	host = sys.argv[4]
	port = int(sys.argv[5])

	ids = []
	salidas = queue.Queue()

	emisor = threading.Thread(target = enviar)
	receptor = threading.Thread(target = recibir)
	almacenarIDs(ids, nodos)

	log.startLogging(sys.stdout)
	factory = WebSocketServerFactory(f"ws://{host}:{port}")
	factory.protocol = WebSocketServer
	reactor.listenTCP(port, factory)
	
	emisor.start()
	receptor.start()
	reactor.run()

	emisor.join()
	receptor.join()
	salidas.join()