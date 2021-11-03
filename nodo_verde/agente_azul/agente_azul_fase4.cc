#include "Agente_Azul.h"

//Variables globales
bool conexion;

/**
 *	@brief Subrutina del hilo agente azul
 *	@details Esta subrutina crea los pipes para la comunicación entre
 *	el agente y el servidor web, además de generar otro proceso
 *	que corre el programa Servidor.py. Además se generan hilos para
 *	realizar las comunicaciones (Con el servidor y con el agente verde)
 *	@param[in] [1] Vector de enteros: Para conocer los ids de los
 *	vecinos y enviar los mensajes a la cola correspondiente
 *	[2] Cola de mensajes: Es un puntero a un arreglo de colas de
 *	mensajes, da salida y entrada a los mensajes. Una de las colas
 *	es para los mensajes que recibe el usuario, las otras colas son
 *	para los mensajes de salida
 *	[3] String: Ruta para el pipe Agente -> Servidor
 *	[4] String: Ruta para el pipe Servidor -> Agente
 *	@pre Los 4 parámetros deben tener datos, deben estar las
 *	direcciones para los pipes, además de los ids de los vecinos y
 *	de colas que deben estar creadas
 *	@remark Utiliza las colas para enviar codificaciones a lo interno
 *	como un mensaje de finalización para desconectar al cliente del
 *	servidor (-1::Salir)
 *	@author Johel Phillips Ugalde B75821
 *	@date 15-09-20
 */
void hiloAzul(std::vector<int>* nodosIDs,
		Cola<struct FowardingAplicacion>* colaAzul,
		char* fifoAgenteServidor, char* fifoServidorAgente,
		char* puertoWebSocket, 
		Cola<struct DatosFowardingAplicacion>* colaDespachadorAzul,
		Cola<uint16_t>* colaIdsAzul){

	pid_t pid;
	size_t longitud;
	std::string ids;
	conexion = true;

	if(mkfifo(fifoAgenteServidor, 0666) == -1)
		mostrarError("No se pudo crear el fifo Agente -> Servidor");

	if(mkfifo(fifoServidorAgente, 0666) == -1)
		mostrarError("No se pudo crear el fifo Servidor -> Agente");

	longitud = (*nodosIDs).size();
	for(size_t i = 0; i < longitud; ++i){
		ids.append(std::to_string((*nodosIDs)[i]));
		ids.append(",");
	}

	pid = fork();
	if(!pid){
		if(execlp("python3","python3", "src/nodo_azul/./Web_socket.py",
				fifoAgenteServidor, fifoServidorAgente, ids.c_str(),
				"localhost", puertoWebSocket, (char*)0)== -1)
			mostrarError("No se pudo ejecutar Servidor.py");
	}

	std::thread emisor(enviar, colaAzul, colaIdsAzul, 
		fifoAgenteServidor);
	std::thread receptor(recibir, colaDespachadorAzul,
		fifoServidorAgente, colaAzul, colaIdsAzul);

	receptor.join();
	emisor.join();
}

/**
 *	@brief Subrutina del hilo emisor
 *	@details Esta subrutina se encarga de enviarle mensajes al
 *	servidor web, y por ende al usuario, provenientes de los
 *	vecinos, además del código de finalización.
 *	@param[in] [1] Cola de mensajes: Es un puntero a un arreglo de
 *	colas de mensajes, en este caso se sacan los mensajes de la
 *	primer cola para ser enviados al servidor
 *	[2] String: Ruta del pipe para el paso de los mensajes
 *	Agente -> Servidor
 *	@pre Las colas deben existir y la ruta de los pipes debe ser
 *	correcta
 *	@remark Utiliza las colas para enviar codificaciones a lo interno
 *	como un mensaje de finalización para desconectar al cliente del
 *	servidor (-1::Salir)
 *	@author Johel Phillips Ugalde B75821
 *	@date 15-09-20
 */
void enviar(Cola<struct FowardingAplicacion>* colaAzul,
		Cola<uint16_t>* colaIdsAzul, char* fifoAgenteServidor){

	int fd = open(fifoAgenteServidor, O_WRONLY);
	std::string buffer;
	struct FowardingAplicacion paquete;
	uint16_t idFuente;

	while(conexion){
		paquete = colaAzul->pop();
		idFuente = colaIdAzul->pop();
		if(paquete.tipoForwardingAplicacion == MENSAJE_CORTO){
			buffer.clear();
			buffer.append(std::to_string(idFuente));
			buffer.append(":\t");
			buffer.append(paquete.buffer);
			if(buffer.size() > LONGITUD)
				buffer = buffer.substr(0, LONGITUD);
			write(fd, buffer.c_str(), LONGITUD);

		} else{
			uint8_t tipo = (uint8_t)paquete.datos[0];
			if(tipo == THREE_WAY || tipo == TWO_WAY){
				respuestaHandShake(fd, datos, idFuente);
			} else if(tipo == STOP_AND_WAIT){
				respuestaTransmision(fd, datos, idFuente);
			} else {
				mostrarError("Tipo inválido!");
			}
		}
	}
	close(fd);
}

/**
 *	@brief Subrutina del hilo receptor
 *	@details Esta subrutina se encarga de recibir mensajes del
 *	servidor web, y enviarlos a la cola de salida correspondiente
 *	@param[in] [1] Cola de mensajes: Es un puntero a un arreglo de
 *	colas de mensajes, en este caso se envían los mensajes a la
 *	cola correspondiente para ser enviados a un vecino
 *	[2] Vector de enteros: Contiene los ids de los vecinos, sirve
 *	para enviar los mensajes a la cola correspondiente
 *	[3] String: Ruta del pipe para el paso de los mensajes
 *	Agente -> Servidor
 *	@pre Las colas deben existir y la ruta de los pipes debe ser
 *	correcta, además de contener la información de los ids de los
 *	vecinos
 *	@remark Utiliza las colas para enviar codificaciones a lo interno
 *	como un mensaje de finalización para desconectar al cliente del
 *	servidor (-1::Salir)
 *	@author Johel Phillips Ugalde B75821
 *	@date 15-09-20
 */
void recibir(Cola<struct DatosFowardingAplicacion>*colaDespachadorAzul,
		char* fifoServidorAgente, 
		Cola<struct FowardingAplicacion>* colaAzul,
		Cola<uint16_t>* colaIdsAzul){

	int fd = open(fifoServidorAgente, O_RDONLY);
	std::string msg;
	char buffer[LONGITUD];
	int posicion, destino;

	while(conexion){
		msg.clear();
		read(fd, buffer, LONGITUD);
		msg.append(buffer);
		posicion = msg.find("::");

		if(posicion == -1){
			if(msg[0] == "#" || msg[0] == "@"){
				handShake(fd, colaDespachadorAzul);
			} else {
				transmisionConfiable(fd, colaDespachadorAzul);
			}

		} else{
			destino = std::stoi(msg.substr(0,posicion));
			msg = msg.substr(posicion+2);
			struct DatosForwardingAplicacion salida;
			salida.idDestino = destino;
			if(destino == -1 || destino == 0){ // Salida o broadcast
				salida.tipo = BROADCAST;
			} else{
				salida.tipo = FORWARDING;
			}
			salida.tipoForwardingAplicacion = MENSAJE_CORTO;
			strcpy(salida.datos, msg.c_str());
			salida.lengthForwarding = (uint16_t)strlen(salida.datos);
			colaDespachadorAzul->push(salida);

			if(destino == -1){ // Terminar el hilo
				conexion = false;
				struct FowardingAplicacion estructura;
				estrucutra.tipo = MENSAJE_CORTO;
				strcpy(estructura.datos, msg.c_str());
				estructura.lengthForwarding = 
					(uint16_t)strlen(estructura.datos);
				colaAzul->push(estructura);
				colaIdsAzul->push(0);
			}
		}
	}
	close(fd);
}


void mostrarError(const char* mensaje){
	std::cerr << "Error: " << mensaje << std::endl;
}

/**
 *	@brief Subrutina que envía mensajes de three / two Way Hand Shake
 *  al nodo verde para que sean enviadas por la red
 *	@details Esta subrutina se encarga de enviar solicitudes con la
 *  estructura de three / two Way Hand Shake para que sean respondidas
 *  por otro nodo
 *	@param[in] [1] fd: Es el file descriptor del pipe por el cual se
 *  leen los mensaje que van a ser enviados posteriormente
 *  [2] colaDespachadorAzul: Es la cola a la que le agregan las
 *  solucitudes que van de salida
 *	@pre Debe llegar un mensaje de aviso para que se llame este método
 *	@author Johel Phillips Ugalde B75821
 *	@date 07-12-20
 */
void handShake(int fd, 
	Cola<struct DatosFowardingAplicacion>* colaDespachadorAzul){

	//#/@, fuente/destino, tipo, sn, rn -> three/two WayHandShake
	struct DatosFowardingAplicacion salida;
	struct TransmisionConfiable transmision;
	struct ThreeWayHandShakeAzul estructura;
	std::vector<char*> datos;
	bool terminar = false;
	char buffer[LONGITUD];
	size_t offset = 0;

	while(!terminar){
		datos.clear();
		read(fd, buffer, LONGITUD);
		dividir(buffer, ",", false, &datos);

		if(datos[0] == "0" && datos[1] == "0"){
			terminar = true;

		} else{
			offset = 0;
			estructura.tipo = (uint8_t)atoi(datos[1]);
			estructura.SN = (uint16_t)atoi(datos[2]);
			estructura.RN = (uint16_t)atoi(datos[3]);

			if(buffer[0] == "#"){
				transmision.tipo = THREE_WAY;
			} else{
				transmision.tipo = TWO_WAY;
			}
			memmove(transmision.datos, &estructura.tipo, 
				sizeof(estructura.tipo));
			offset += sizeof(estructura.tipo);
			memmove(transmision.datos+offset, &estructura.SN, 
				sizeof(estructura.SN));
			offset += sizeof(estructura.SN);
			memmove(transmision.datos+offset, &estructura.RN, 
				sizeof(estructura.RN));
			transmision.length = (uint16_t)strlen(transmision.datos);

			salida.idDestino = (uint8_t)atoi(datos[0]);
			salida.tipo = FORWARDING;
			salida.tipoForwardingAplicacion = TRANSMISION;

			offset = 0;
			memmove(salida.datos, &transmision.tipo, 
				sizeof(transmision.tipo));
			offset += sizeof(transmision.tipo);
			memmove(salida.datos+offset, &transmision.length, 
				sizeof(transmision.length));
			offset += sizeof(transmision.length);
			memmove(salida.datos+offset, &transmision.datos, 
				sizeof(transmision.datos));
			salida.lengthForwarding = (uint16_t)strlen(salida.datos);

			colaDespachadorAzul->push(salida);
		}
	}
}

/**
 *	@brief Subrutina que responde mensajes de three /two Way Hand Shake
 *	@details Esta subrutina se encarga de recibir las solicitudes con
 *  la estructura de three/two Way Hand Shake para que sean respondidas
 *  por el servidor azul, estas solicitudes son pasadas por el pipe
 *	@param[in] [1] fd: Es el file descriptor del pipe por el cual se
 *  escriben las solicitudes entrantes
 *  [2] datos: Es la solicitud entrante, en el formato definido por
 *  todos los grupos de trabajo
 *  [3] idFuente: Es el id del nodo emisor de la solicitud
 *	@pre Debe llegar un mensaje de aviso para que se llame este método
 *	@author Johel Phillips Ugalde B75821
 *	@date 07-12-20
 */
void respuestaHandShake(int fd, char* datos, int idFuente){
	//#/@, fuente/destino, tipo, sn, rn -> three/two WayHandShake
	std::string mensaje;
	std::string datosStr(datos);

	if(datos[0] == THREE_WAY){
		mensaje.append("#,");
	} else{
		mensaje.append("@,");
	}
	mensaje.append(std::to_string(idFuente));
	mensaje.append(",");
	mensaje.append(datosStr, 0, 1);
	mensaje.append(",");
	mensaje.append(datosStr, 1, 2);
	mensaje.append(",");
	mensaje.append(datosStr, 3, 2);
	write(fd, mensaje.c_str(), (int)mensaje.size());
}

/**
 *	@brief Subrutina que envía paquetes
 *	@details Esta subrutina se encarga de enviar paquetes 
 *  pertenecientes a un archivo
 *	@param[in] [1] fd: Es el file descriptor del pipe por el cual se
 *  leen los mensaje que van a ser enviados posteriormente
 *  [2] colaDespachadorAzul: Es la cola a la que le agregan los
 *  paquetes del archivo que van de salida
 *	@pre Debe llegar un mensaje de aviso para que se llame este método
 *	@author Johel Phillips Ugalde B75821
 *	@date 07-12-20
 */
void transmisionConfiable(int fd, 
	Cola<struct DatosFowardingAplicacion>* colaDespachadorAzul){

	// Emisor/Receptor, Envio/ACK, SN, RN, datos
	struct DatosFowardingAplicacion salida;
	struct TransmisionConfiable transmision;
	struct StopAndWaitAzul estuctura;
	std::vector<char*> datos;
	bool terminar = false;
	char buffer[LONG_ARCHIVO];

	while(!terminar){
		datos.clean();
		read(fd, buffer, LONG_ARCHIVO);
		dividir(buffer, ",", true, &datos);

		if(datos[0] == "0" && datos[1] == "0"){
			terminar = true;

		} else{
			offset = 0;
			estructura.tipo = (uint8_t)atoi(datos[1]);
			estructura.SN = (uint16_t)atoi(datos[2]);
			estructura.RN = (uint16_t)atoi(datos[3]);
			strcpy(estructura.datos, datos[4]);
			estructura.length = (uint16_t)strlen(estructura.datos);

			transmision.tipo = STOP_AND_WAIT;
			memmove(transmision.datos, &estructura.tipo, 
				sizeof(estructura.tipo));
			offset += sizeof(estructura.tipo);
			memmove(transmision.datos+offset, &estructura.SN, 
				sizeof(estructura.SN));
			offset += sizeof(estructura.SN);
			memmove(transmision.datos+offset, &estructura.RN, 
				sizeof(estructura.RN));
			offset += sizeof(estructura.RN);
			memmove(transmision.datos+offset, &estructura.length, 
				sizeof(estructura.length));
			offset += sizeof(estructura.length);
			memmove(transmision.datos+offset, &estructura.datos, 
				sizeof(estructura.datos));
			transmision.length = (uint16_t)strlen(transmision.datos);

			salida.idDestino = (uint8_t)atoi(datos[0]);
			salida.tipo = FORWARDING;
			salida.tipoForwardingAplicacion = TRANSMISION;

			offset = 0;
			memmove(salida.datos, &transmision.tipo, 
				sizeof(transmision.tipo));
			offset += sizeof(transmision.tipo);
			memmove(salida.datos+offset, &transmision.length, 
				sizeof(transmision.length));
			offset += sizeof(transmision.length);
			memmove(salida.datos+offset, &transmision.datos,
				sizeof(transmision.datos));
			salida.lengthForwarding = (uint16_t)strlen(salida.datos);

			colaDespachadorAzul->push(salida);
		}
	}
}

/**
 *	@brief Subrutina que responde a los paquetes de un archivo
 *	@details Esta subrutina se encarga de recibir los paquetes de los
 *  archivos que sean enviados desde otros nodos y enviarlos en orden
 *  al servidor azul para que sean ensamblados
 *	@param[in] [1] fd: Es el file descriptor del pipe por el cual se
 *  envian los paquetes al servidor
 *  [2] datos: Es el paquete entrante, en el formato definido por
 *  todos los grupos de trabajo
 *  [3] idFuente: Es el id del nodo emisor del paquete
 *	@pre Debe llegar un mensaje de aviso para que se llame este método
 *	@author Johel Phillips Ugalde B75821
 *	@date 07-12-20
 */
void respuestaTransmision(int fd, char* datos, int idFuente){
	// Emisor/Receptor, Envio/ACK, SN, RN, datos
	std::string mensaje;
	std::string datosStr(datos);

	mensaje.append(std::to_string(idFuente));
	mensaje.append(",");
	mensaje.append(datosStr, 0, 1);
	mensaje.append(",");
	mensaje.append(datosStr, 1, 2);
	mensaje.append(",");
	mensaje.append(datosStr, 3, 2);
	mensaje.append(",");
	mensaje.append(datosStr, 7, std::stoi(datosStr.substr(5, 2)));
	write(fd, mensaje.c_str(), (int)mensaje.size());
}

/**
 *	@brief Subrutina que divide una hilera en varias
 *	@details Esta subrutina se encarga de dividir una hilera en varias
 *  dependiendo de los parámetros, se ayuda con la función strtok
 *	@param[in] [1] cadena: Es la hilera que se desea dividir
 *  [2] caracter: Es el carcter a partir del cual se van a generar
 *  nuevas hileras
 *  [3] cadenaCompleta: Variable booleana que permite desechar el
 *  primer token encontrado (False) o no (True)
 *  [4] datos: Es el vector en el que se almacenan los tokens
 *  resultantes
 *	@author Johel Phillips Ugalde B75821
 *	@date 07-12-20
 */
void dividir(char* cadena, const char* caracter, bool cadenaCompleta,
		std::vector<char*>* datos){

	char* token = strtok(cadena, caracter);

	if(cadenaCompleta){
		datos->push_back(token);
	}
	token = strtok(NULL, caracter);

	while(token != NULL){
		datos->push_back(token);
		token = strtok(NULL, caracter);
	}
}