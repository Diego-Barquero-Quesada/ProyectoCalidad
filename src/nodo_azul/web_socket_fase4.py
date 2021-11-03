#!/usr/bin/env python3

from autobahn.twisted.websocket import WebSocketServerProtocol, \
	WebSocketServerFactory
from twisted.python import log
from twisted.internet import reactor
from random import seed, randint, random
import threading, sys, codecs, os, time, queue

# Códigos
CODIGO = '-1::Salir'.encode('ASCII', 'ignore')
NULO = '\x00'.encode('ASCII', 'ignore')
FIN = 'fin'.encode('ASCII', 'ignore')
INICIO_THREE_WAY = '#'.encode('ASCII', 'ignore')
INICIO_TWO_WAY = '@'.encode('ASCII', 'ignore')
INICIO_TRANSMISION = '!'.encode('ASCII', 'ignore')
CIERRE_TRANSMISION = '0,0'.encode('ASCII', 'ignore')
# Tamaños
LONGITUD = 207
TAM_MAX_PAQUETE = 1000
# Porcentaje de fallos
ERROR = 0.2
# ThreeWayHandShake
SOLICITUD = 1
CONFIRMACION = 2
RECHAZO = 3
FINAL = 4
# Transmisión confiable
ENVIO = 1
RESPUESTA = 2
# TwoWayHandShake
CIERRE = 1
CIERRE_ACK = 2


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
			inicio = 0
			tamano = len(payload)
			while inicio < tamano:
				if tamano > TAM_MAX_PAQUETE:
					salidas.put(payload[inicio:inicio+TAM_MAX_PAQUETE])
					#time.sleep(0.001)
					tamano -= TAM_MAX_PAQUETE
				else:
					salidas.put(payload[inicio:])
				inicio += TAM_MAX_PAQUETE
			salidas.put(FIN) #ojojojojojo
		else:
			print(f"Text message received: {payload.decode('utf8')}")
			salidas.put(payload.decode('utf8'))

	'''
	brief Imprime en salida estandar cuando se cierra una conexión
	details Esta subrutina imprime un mensaje cuando un cliente cierra 
	el navegador.
	author Jeisson Hidalgo
	date 28-09-20
	'''
	def onClose(self, wasClean, code, reason):
		print(f"WebSocket connection closed: {reason}")
		conexiones.pop(0)
		salidas.put(CODIGO)

	'''
	brief Envía mensajes a la página web
	details Esta subrutina envía mensajes a la página web por mwdio del
	web socket, para que el usuario pueda visualizarlo
	author Johel Phillips
	date 05-10-20
	'''
	def enviarPorWebSocket(self, mensaje, esBinario):

		#global nombreArchivo

		if esBinario:
			archivo = open(f'src/nodo_azul/archivosRecibidos/{nombreArchivo}', 'wb')
			archivo.write(mensaje)
			archivo.close()
			self.sendMessage(f'file /archivosRecibidos/{nombreArchivo}'.encode('utf8'), False)
		else:
			mensaje = mensaje.decode('utf8')
			if mensaje[0] == '_': #mensaje de texto referente a archivo
				array = mensaje.split('/')
				nombreArchivo = array[2]
			else:
				self.sendMessage(f'{mensaje}'.encode('utf8'), False)




'''
 *	@brief Subrutina que genera mensajes de Two Way Hand Shake
 *	@details Esta subrutina genera mensajes de Two Way Hand Shake y
 *  y espera por una respuesta
 *	@param[in] [1] destino: Id del nodo destino
 *	[2] fd: file descriptor del pipe por el cual se escribe
 *	@author Johel Phillips Ugalde B75821
 *	@date 07-12-20
 '''
def twoWayHandShakeEnvia(destino, fd):
	seed(1)
	sn = randint(2, 999)
	reintentos = 3
	bandera = False
	i = 0

	while i < reintentos and bandera == False:
		mensaje = '@,'+str(destino)+','+str(CIERRE)+','+str(sn)+',0'
		os.write(fd, mensaje + NULO)
		respuesta = colaTWH.get(block = True, timeout = 5)
		respuestaVector = respuesta.split(',')

		if int(respuestaVector[2]) == CIERRE_ACK and 
				int(respuestaVector[3]) == sn + 1:
			bandera = True
			
		i += 1


def ACKtwoWayHandShake(mensaje, fd):
	seed(1)
	rn = randint(2, 999)
	respuesta = mensaje.split(',')
	sn = int(respuesta[3]) + 1
	enviar = '#,'+str(respuesta[1])+','+str(CIERRE_ACK)+','+str(sn)+','+
		str(rn)
	os.write(fd, mensaje + NULO)

'''
 *	@brief Subrutina que genera mensajes de transmisión confiable
 *	@details Esta subrutina genera mensajes de transmisión confiable
 *	@param[in] [1] paquetes: Paquetes del archivo a enviar
 *  [2] destino: Id del nodo destino
 *	[3] fd: file descriptor del pipe por el cual se escribe
 *	@author Johel Phillips Ugalde B75821
 *	@date 07-12-20
 '''
def transmisionConfiable(paquetes, destino, fd):
	# Emisor/Receptor, Envio/ACK, SN, RN, datos
	sn = 0
	rnLocal = 0

	while len(paquetes):
		if ERROR <= random():
			mensaje = str(destino)+','+str(ENVIO)+','+str(sn)+',0,'+
				paquetes[0]
			os.write(fd, mensaje)
		respuesta = colaTC.get(block = True, timeout = 30)
		respuestaVector = respuesta.split(',')

		if sn + 1 == respuestaVector[3]: # SN + 1 = RN que me llega
			sn += 1
			paquetes.pop(0)

def receptorTransmisionConfiable(fd):
	rnLocal = 0
	mensaje = fd.read(TAM_MAX_PAQUETE+13)
	posicion = mensaje.find(NULO)
	mensaje = mensaje[:posicion]
	nuevoArchivo = ''.encode('ASCII', 'ignore')

	while mensaje[0] != 35: # ASCII Numeral (#) XD
		# Emisor/Receptor, Envio/ACK, SN, RN, datos
		mensajeDividido = split(',')
		if int(mensajeDividido[2]) == rnLocal:
			rnLocal += 1
			nuevoArchivo += mensajeDividido[4]

		respuesta = mensajeDividido[0]+str(RESPUESTA)+
			mensajeDividido[2]+str(rnLocal)
		salidas.put(respuesta.decode('ASCII', 'ignore'))

		mensaje = fd.read(TAM_MAX_PAQUETE+13)
		posicion = mensaje.find(NULO)
		mensaje = mensaje[:posicion]
	conexiones[0].enviarPorWebSocket(nuevoArchivo, True)

'''
 *	@brief Subrutina que genera mensajes de Three Way Hand Shake
 *	@details Esta subrutina genera mensajes de Three Way Hand Shake
 *	@param[in] [1] destino: Id del nodo destino
 *	[2] fd: file descriptor del pipe por el cual se escribe
 *	@author Johel Phillips Ugalde B75821
 *	@date 07-12-20
 '''
def threeWayHandShakeEnvia(destino, fd):
	seed(1)
	sn = randint(2, 999)
	reintentos = 3
	bandera = False
	exito = False
	responder = False
	i = 0

	while i < reintentos and bandera == False:
		mensaje = '#,'+str(destino)+','+str(SOLICITUD)+','+str(sn)+',0'
		os.write(fd, mensaje + NULO)
		respuesta = colaTWH.get(block = True, timeout = 5)
		respuestaVector = respuesta.split(',')

		if int(respuestaVector[2]) == CONFIRMACION and 
				int(respuestaVector[3]) == sn + 1:
			responder = True
			exito = True

		elif int(respuestaVector[2]) == RECHAZO and 
				int(respuestaVector[3]) == sn + 1:
			responder = True

		if responder:
			bandera = True
			rnLocal  = int(respuestaVector[4]) + 1
			sn += 1
			mensaje = '#,'+str(destino)+','+str(FINAL)+','+str(sn)+','+
				str(rnLocal)
			os.write(fd, mensaje + NULO)
			
		i += 1
	return exito

def ACKThreeWayHandShake(mensaje, fd):
	seed(1)
	rn = randint(2, 999)
	if estoyEnTM:
		tipo = RECHAZO
	else:
		tipo = CONFIRMACION
		estoyEnTM = True

	respuesta = mensaje.split(',')
	sn = int(respuesta[3]) + 1
	enviar = '#,'+str(respuesta[1])+','+str(tipo)+','+str(sn)+','+
		str(rn)
	os.write(fd, mensaje + NULO)

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
def enviarPorPipe():

	fd = os.open(pipeSalida, os.O_WRONLY)

	while True:
		mensaje = salidas.get(block = True)
		salidas.task_done()

		if mensaje[0] == '_' and mensaje[1] == '_': # Archivos
			datos = mensaje.split('/')
			destino = datos [1]
			os.write(INICIO_THREE_WAY + NULO)
			exito = threeWayHandShakeEnvia(destino, fd)
			os.write(fd, CIERRE_TRANSMISION + NULO)

			if exito:
				paquetes = []
				envio = destino.encode('ASCII', 'ignore') + 
					'::'.encode('ASCII', 'ignore')
				os.write(fd, envio + mensaje + NULO) # Nombre archivo
				mensaje = salidas.get(block = True)
				while mensaje.find(FIN) != -1:
					paquetes.append(mensaje)
					mensaje = salidas.get(block = True)
				os.write(INICIO_TRANSMISION + NULO)
				transmisionConfiable(paquetes, destino, fd)
				os.write(fd, CIERRE_TRANSMISION + NULO)
				os.write(INICIO_TWO_WAY + NULO)
				twoWayHandShakeEnvia(dentino, fd)
				os.write(fd, CIERRE_TRANSMISION + NULO)

		else:
			mensaje = mensaje.encode('ASCII', 'ignore')
			if len(mensaje) >= LONGITUD:
				mensaje = mensaje[:LONGITUD-1]

			os.write(fd, mensaje + NULO)
			if mensaje.find(CODIGO) != -1:
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
def recibirPorPipe():
	
	fd = open(pipeEntrada, 'br')
	
	while True:
		mensaje = fd.read(LONGITUD)

		if mensaje.find(':\t'.encode('ASCII', 'ignore')) != -1:
			if mensaje.find(CODIGO[:3]) != -1:
				reactor.callFromThread(reactor.stop)
				break
			posicion = mensaje.find(NULO)
			mensaje = mensaje[:posicion]
			conexiones[0].enviarPorWebSocket(mensaje, False)
	
		elif mensaje[0] == '_' and mensaje[1] == '_':
			posicion = mensaje.find(NULO)
			mensaje = mensaje[:posicion]
			conexiones[0].enviarPorWebSocket(mensaje, False)

			receptorTransmisionConfiable(fd)

		elif mensaje[0] == '#' and int(mensaje[2]) != SOLICITUD:
			mensaje = str(mensaje)
			colaTWH.put(mensaje)

		elif mensaje[0] == '#' and int(mensaje[2]) == SOLICITUD:
			mensaje = str(mensaje)
			ACKThreeWayHandShake(mensaje, fd)

		elif mensaje[0] == '#' and mensaje[2] == FINAL:
			respuesta = mensaje.split(',')
			if(int(mensaje[4]) == rn + 1):
				rn = 0
				estoyEnTM = True

		elif mensaje[0] == '@':
			mensaje = str(mensaje)
			ACKtwoWayHandShake(mensaje, fd)
			estoyEnTM = False

		else:
			mensaje = str(mensaje)
			colaTC.put(mensaje)

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
	
	conexiones = []
	nombreArchivo = ""
	rn = 0
	estoyEnTM = False
	ids = []
	salidas = queue.Queue()
	colaTWH = queue.Queue()
	colaTC = queue.Queue()

	emisor = threading.Thread(target = enviarPorPipe)
	receptor = threading.Thread(target = recibirPorPipe)
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