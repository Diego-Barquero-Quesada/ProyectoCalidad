#include "Agente_red.h"
#include <unistd.h>

struct datosForwarding{
	uint16_t idNodo;
	uint16_t distancia;
	uint16_t camino;
};

uint16_t nodoSenderTWH;
bool reiniciarEnvio = false;
bool enviarDenuevo = true;
std::vector<struct datosForwarding> tablaForwarding;

void capaRed(Cola<struct ForwardingAplicacion>* colaAzul,
Cola<struct ArbolGenerador>* colaRosada,
std::vector<Cola<struct CapaEnlace>>* colasVerdes,
Cola<struct DatosForwardingAplicacion>* colaDespachadorAzul,
Cola<struct DatosArbolGenerador>* colaDespachadorRosado,
Cola<struct CapaRed>* colaDespachadorVerde,
std::vector<int>* nodosIDs, Cola<std::string>* despachadorMiembros,
Cola<std::string>* colaAlcanzabilidad,
std::vector<datosNodo>* tablaVecinos,char* IP,
Cola<std::string>* colaTablaForwarding,
Cola<std::string>* colaEnviarAlcanzabilidad){

	Cola<struct DatosForwarding> colaForwarding;
	Cola<struct Broadcast> colaBroadcast;
	
	std::thread hiloAzul(despachadorAzul,
	colaDespachadorAzul, &colaForwarding, &colaBroadcast, 
	(*nodosIDs)[0]);
	
	std::thread hiloRosado(despachadorRosado,colaDespachadorRosado,
	nodosIDs,
	colasVerdes);
		

	std::thread hiloVerde(despachadorVerde,
	colaDespachadorVerde, &colaForwarding, &colaBroadcast,
	colaRosada);
	
	std::thread hiloForwarding(forwarding, colaAzul,
	colasVerdes, &colaForwarding, nodosIDs);

	std::thread hiloBroadcast(broadcast, colaAzul,
	colasVerdes, &colaBroadcast, nodosIDs, despachadorMiembros,
	colaAlcanzabilidad, colaEnviarAlcanzabilidad);
		
    
	std::thread hiloAlcanzabilidad(enviarAlcanzabilidad,
	colaAlcanzabilidad, tablaVecinos,
	IP,&colaBroadcast, colaEnviarAlcanzabilidad);
	
	std::thread hiloTablaForwarding(verificarTablaForwarding,
	colaTablaForwarding, nodosIDs);

	std::thread hiloTimeout(timeout);
	
	hiloAzul.join();
	hiloRosado.join();
	hiloVerde.join();
	hiloForwarding.join();
	hiloBroadcast.join();
	hiloAlcanzabilidad.join();
	hiloTimeout.join();
}

void verificarTablaForwarding(Cola<std::string>* colaTablaForwarding,
std::vector<int>* nodosIDs){

	std::string recibirTabla;
	std::string aux;

	while(1){
		recibirTabla=colaTablaForwarding->pop();

		std::stringstream s_stream(recibirTabla);
		std::vector<std::string> resultado;

		while(s_stream.good()) {
			std::string substr;
			getline(s_stream, substr, ',');
			resultado.push_back(substr);
		}
		
		for(size_t indice=0;indice<resultado.size();indice++){
			struct datosForwarding nuevoDato;
			aux=resultado[indice];
			std::stringstream s_stream2(resultado[indice]);

			std::string substr2;
			getline(s_stream2, substr2, '-');
			for(size_t indice2=0;indice2<(*nodosIDs).size();indice2++){
				if((*nodosIDs)[indice2]==std::stoi(substr2)){
					nuevoDato.camino=std::stoi(substr2);
				}
			}

			getline(s_stream2, substr2, '-');
			nuevoDato.distancia=std::stoi(substr2);

			getline(s_stream2, substr2, '-');
			nuevoDato.idNodo=std::stoi(substr2);
			tablaForwarding.push_back(nuevoDato);
		}
	}
}

void enviarAlcanzabilidad(Cola<std::string>* colaAlcanzabilidad,
std::vector<datosNodo>* tablaVecinos,
char* IP,Cola<struct Broadcast>* colaBroadcast,
Cola<std::string>* colaEnviarAlcanzabilidad){

	std::string aux;
	std::string mensajeAlcanzabilidad;

	size_t indice;
	char host[256];

	mensajeAlcanzabilidad="";
	bzero(host,sizeof(host));
	mensajeAlcanzabilidad += "90,";
	mensajeAlcanzabilidad += std::to_string((*tablaVecinos)[0].ID);
	mensajeAlcanzabilidad += "," + 
	std::to_string((tablaVecinos->size()-1));

	for(indice=1;indice<tablaVecinos->size();indice++){
		mensajeAlcanzabilidad += "," +
		std::to_string((*tablaVecinos)[indice].ID);

		if(((*tablaVecinos)[indice].IP).compare(IP) == 0){
			mensajeAlcanzabilidad+=",1";
		}else{
			mensajeAlcanzabilidad+=",2";
		}
	}

	aux = mensajeAlcanzabilidad;
	colaEnviarAlcanzabilidad->push(aux);

	while(1){
		

		std::string estadoArbol=colaAlcanzabilidad->pop();
		uint16_t prueba;
		uint16_t prueba2;
		uint8_t prueba3;

		if(estadoArbol == "45"){
			char buffer[200];
			uint16_t aux1 =
			static_cast<uint16_t>(tablaVecinos->size()-1);

			memmove( buffer,&aux1 ,sizeof(aux1));
			memmove( &prueba,buffer ,sizeof(prueba));
			size_t offset = sizeof(aux1);

			for(indice=1;indice<tablaVecinos->size();indice++){
				uint16_t aux2 =
				static_cast<uint16_t>((*tablaVecinos)[indice].ID);

				memmove( buffer+offset,&aux2 ,sizeof(aux2));
				memmove( &prueba2,buffer+offset ,sizeof(prueba2));

				offset=offset+sizeof(aux2);
				uint8_t aux3;

				if(((*tablaVecinos)[indice].IP).compare(IP)==0){
					aux3=0x01;
					memmove( buffer+offset,&aux3 ,sizeof(aux3));
					memmove( &prueba3,buffer+offset ,sizeof(prueba3));
					offset=offset+sizeof(aux3);
				}else{
					aux3=0x02;
					memmove( buffer+offset,&aux3 ,sizeof(aux3));
					memmove( &prueba3,buffer+offset ,sizeof(prueba3));
					offset=offset+sizeof(aux3);
				}
			}

			struct Broadcast broadcast;
			broadcast.tipo = 0x02;
			broadcast.id_origen_inicial =
			static_cast<uint16_t>((*tablaVecinos)[0].ID);

			memmove(&broadcast.datos,buffer ,sizeof(broadcast.datos));
			broadcast.longitud=offset;
			colaBroadcast->push(broadcast);
		}
	}
}

void despachadorAzul(
Cola<struct DatosForwardingAplicacion>* colaDespachadorAzul,
Cola<struct DatosForwarding>* colaForwarding,
Cola<struct Broadcast>* colaBroadcast, int idPropio){

	while(1){
		struct DatosForwardingAplicacion datos =
		colaDespachadorAzul->pop();

		if(datos.tipo == 0x01){
			char buffer3 [1013];
			struct DatosForwarding forwarding;
			forwarding.idFuenteInicial =
			static_cast<uint16_t>(idPropio);

			forwarding.idDestino = datos.idDestino;
			size_t offset = 0;
			memmove(buffer3,&datos.tipoAplicacion,
			sizeof(datos.tipoAplicacion));

			offset = offset + sizeof(datos.tipoAplicacion);
			memmove(buffer3 + offset, &datos.longitud,
			sizeof(datos.longitud));

			offset = offset + sizeof(datos.longitud);
			memmove(buffer3 + offset, &datos.datos,
			sizeof(datos.datos));

			offset = offset + datos.longitud;
			
			struct DatosForwardingAplicacion datosNew;
			size_t offset1 = 0;
			memmove(&datosNew.tipoAplicacion, buffer3,
			sizeof(datosNew.tipoAplicacion));

			offset1 = offset1 + sizeof(datosNew.tipoAplicacion);
			memmove(&datosNew.longitud, buffer3 + offset1,
			sizeof(datosNew.longitud));

			offset1 = offset1 + sizeof(datosNew.longitud);

			memmove(&datosNew.datos, buffer3 + offset1,
			sizeof(datosNew.datos));

			memmove(&forwarding.datos, buffer3,
			sizeof(forwarding.datos));
			
			forwarding.longitud = offset;
			colaForwarding->push(forwarding);
			
		} else if(datos.tipo == 0x02){
			char buffer3 [200];
			struct Broadcast broadcast;
			broadcast.tipo = 0x01;
			broadcast.id_origen_inicial =
			static_cast<uint16_t>(idPropio);

			memmove( buffer3,&datos.datos, sizeof(buffer3));
			memmove( &broadcast.datos, buffer3,
			sizeof(broadcast.datos));
			
			broadcast.longitud = datos.longitud;
			colaBroadcast->push(broadcast);

		} else {
			std::cout << "Error!\n";
		}
	}
}

void despachadorRosado(
Cola<struct DatosArbolGenerador>* colaDespachadorRosado,
std::vector<int>* nodosIDs,
std::vector<Cola<struct CapaEnlace>>* colasVerdes){

	while(1){
		struct DatosArbolGenerador datos =
		colaDespachadorRosado->pop();

		size_t longitud = nodosIDs->size();

        for(size_t i = 0; i < longitud; ++i){
				
            if(datos.id_destino_final == (*nodosIDs)[i]){
				char buffer [1017];

                struct ArbolGenerador paqueteAg;

                paqueteAg.tipo = datos.tipo;

				paqueteAg.SN = datos.SN;
				paqueteAg.RN =  datos.RN;

				struct CapaRed capaRed;
				capaRed.tipo = 0x03;
				size_t offset = 0;
				memmove(buffer, &paqueteAg.tipo,
				sizeof(paqueteAg.tipo));

				offset = offset + sizeof(paqueteAg.tipo);
				memmove(buffer + sizeof(paqueteAg.tipo), &paqueteAg.SN,
				sizeof(paqueteAg.SN));

				offset = offset + sizeof(paqueteAg.SN);
				memmove(buffer + sizeof(paqueteAg.tipo) +
				sizeof(paqueteAg.SN), &paqueteAg.RN,
				sizeof(paqueteAg.RN));

				offset = offset + sizeof(paqueteAg.RN);
				memmove( &capaRed.datos,buffer ,sizeof(capaRed.datos));

				capaRed.longitud = offset;
					
				struct CapaEnlace paquete;

				paquete.tipo =0X02;
				paquete.idDestinoFinal = datos.id_destino_final;
				paquete.idFuenteInmediato = (*nodosIDs)[0];
				char buffer2[1040];
				offset = 0;
				memmove(buffer2, &capaRed.tipo, sizeof(capaRed.tipo));
				offset = offset + sizeof(capaRed.tipo);
				memmove(buffer2 + sizeof(capaRed.tipo),
				&capaRed.longitud, sizeof(capaRed.longitud));

				offset = offset + sizeof(capaRed.longitud);
				memmove(buffer2 + sizeof(capaRed.tipo) +
				sizeof(capaRed.longitud), &capaRed.datos,
				sizeof(capaRed.datos));

				offset = offset + capaRed.longitud;
				memmove(&paquete.datos, buffer2,
				sizeof(paquete.datos));

				paquete.longitud = offset;
	
                colasVerdes[0][i].push(paquete);
			}
        }
	}
}

void despachadorVerde(Cola<struct CapaRed>* colaDespachadorVerde,
Cola<struct DatosForwarding>* colaForwarding,
Cola<struct Broadcast>* colaBroadcast,
Cola<struct ArbolGenerador>* colaRosada){

	while(1){
		struct CapaRed capaRed =
		colaDespachadorVerde->pop();
		nodoSenderTWH=nodoSender;

		if(capaRed.tipo== 0x01){
			struct DatosForwarding fowardingRecibir;
			struct ForwardingAplicacion mensaje;

			memmove(&fowardingRecibir.idFuenteInicial, capaRed.datos,
			sizeof(fowardingRecibir.idFuenteInicial));

     		memmove(&fowardingRecibir.longitud, capaRed.datos +
			sizeof(fowardingRecibir.idFuenteInicial),
			sizeof(fowardingRecibir.longitud));

    		memmove(&fowardingRecibir.datos, capaRed.datos +
			sizeof(fowardingRecibir.idFuenteInicial) +
			sizeof(fowardingRecibir.longitud),
			sizeof(fowardingRecibir.datos));

			fowardingRecibir.idDestino = destinoFinal;
				
			colaForwarding->push(fowardingRecibir);
		} else if (capaRed.tipo== 0x02){
			
			struct Broadcast nuevoBroadcast;
			memmove(&nuevoBroadcast.tipo, capaRed.datos,
			sizeof(nuevoBroadcast.tipo));
			
			memmove(&nuevoBroadcast.id_origen_inicial, capaRed.datos +
			sizeof(nuevoBroadcast.tipo),
			sizeof(nuevoBroadcast.id_origen_inicial));
			
			memmove(&nuevoBroadcast.longitud, capaRed.datos +
			sizeof(nuevoBroadcast.tipo) +
			sizeof(nuevoBroadcast.id_origen_inicial),
			sizeof(nuevoBroadcast.longitud));

			memmove(&nuevoBroadcast.datos, capaRed.datos +
			sizeof(nuevoBroadcast.tipo) +
			sizeof(nuevoBroadcast.id_origen_inicial) +
			sizeof(nuevoBroadcast.longitud),
			sizeof(nuevoBroadcast.datos));
		
			colaBroadcast->push(nuevoBroadcast);
		} else if (capaRed.tipo== 0x03){
			
			struct ArbolGenerador paqueteAg;
			memmove(&paqueteAg.tipo, capaRed.datos,
			sizeof(paqueteAg.tipo));

			memmove(&paqueteAg.SN, capaRed.datos +
			sizeof(paqueteAg.tipo),sizeof(paqueteAg.SN));

			memmove(&paqueteAg.RN, capaRed.datos +
			sizeof(paqueteAg.tipo) + sizeof(paqueteAg.SN),
			sizeof(paqueteAg.RN));

			nodoSenderTWH=nodoSender;
			colaRosada->push(paqueteAg);
		}  
	}
}

void forwarding(Cola<struct ForwardingAplicacion>* colaAzul,
std::vector<Cola<struct CapaEnlace>>* colasVerdes,
Cola<struct DatosForwarding>* colaForwarding,
std::vector<int>* nodosIDs){

	bool condicon=false;

	while(1){
		char buffer[1017];
		char buffer2[1040];
		
		struct DatosForwarding datosForwarding = colaForwarding->pop();
		struct ForwardingAplicacion mensaje1;
		size_t offset3 = 0;
		char buffer5 [1013];
		memmove(buffer5, &datosForwarding.datos,
		sizeof(datosForwarding.datos));
			
		memmove(&mensaje1.tipoAplicacion, buffer5,
		sizeof(mensaje1.tipoAplicacion));

		offset3 = offset3 + sizeof(mensaje1.tipoAplicacion);
			
		memmove(&mensaje1.longitud, buffer5 + offset3,
		sizeof(mensaje1.longitud));

		offset3 = offset3 + sizeof(mensaje1.longitud);

		memmove(&mensaje1.datos, buffer5 + offset3,
		sizeof(mensaje1.datos));

		if(datosForwarding.idDestino == (*nodosIDs)[0]){
			
			char buffer4 [1013];
			size_t offset = 0;
			struct ForwardingAplicacion mensaje;
			memmove(buffer4, &datosForwarding.datos,
			sizeof(datosForwarding.datos));
			
			memmove(&mensaje.tipoAplicacion, buffer4,
			sizeof(mensaje.tipoAplicacion));

			offset = offset + sizeof(mensaje.tipoAplicacion);
			
			memmove(&mensaje.longitud, buffer4 + offset,
			sizeof(mensaje.longitud));

			offset = offset + sizeof(mensaje.longitud);

			memmove(&mensaje.datos, buffer4 + offset,
			sizeof(mensaje.datos));	
			
			colaAzul->push(mensaje);
		} else {
			
			size_t i = 0;
			while(i < tablaForwarding.size() && condicon == false){

				if(datosForwarding.idDestino ==
				tablaForwarding[i].idNodo){

					struct Forwarding forwardingEnviar;

                    forwardingEnviar.idFuenteInicial =
					datosForwarding.idFuenteInicial;

                    forwardingEnviar.longitud=datosForwarding.longitud;

                    memmove(&forwardingEnviar.datos,
					&datosForwarding.datos,
					sizeof(forwardingEnviar.datos));				

					for(size_t j=0; j<(*nodosIDs).size(); j++){

						if((*nodosIDs)[j]==tablaForwarding[i].camino){

							size_t offset = 0;

							memmove(buffer,
							&forwardingEnviar.idFuenteInicial,
							sizeof(forwardingEnviar.idFuenteInicial));

							offset = offset +
							sizeof(forwardingEnviar.idFuenteInicial);

							memmove(buffer +
							sizeof(forwardingEnviar.idFuenteInicial),
							&(forwardingEnviar.longitud),
							sizeof(forwardingEnviar.longitud));

							offset = offset +
							sizeof(forwardingEnviar.longitud);

							memmove(buffer +
							sizeof(forwardingEnviar.idFuenteInicial) +
							sizeof(forwardingEnviar.longitud),
							&(forwardingEnviar.datos),
							sizeof(forwardingEnviar.datos));

						    offset = offset + forwardingEnviar.longitud;							

							struct CapaRed	capaRed;
							capaRed.tipo=0x01;
							capaRed.longitud=offset;
							memmove( &capaRed.datos, buffer,
							sizeof(capaRed.datos));

							offset = 0;
							memmove(buffer2, &capaRed.tipo,
							sizeof(capaRed.tipo));

							offset = offset +sizeof(capaRed.tipo);
							memmove(buffer2+sizeof(capaRed.tipo),
							&capaRed.longitud,
							sizeof(capaRed.longitud));

							offset = offset +sizeof(capaRed.longitud);
							memmove(buffer2+sizeof(capaRed.tipo) +
							sizeof(capaRed.longitud), &capaRed.datos,
							sizeof(capaRed.datos));

							offset = offset +capaRed.longitud;

							struct CapaEnlace paquete;
							paquete.tipo = 0x02;
							paquete.idDestinoFinal =
							datosForwarding.idDestino;

							paquete.idFuenteInmediato =
							datosForwarding.idFuenteInicial;

							paquete.longitud=offset;
							memmove(&paquete.datos, buffer2,
							paquete.longitud);

							colasVerdes[0][i].push(paquete);
							condicon = true;
							break;
						}
					}
				}
				i++;
			}
			condicon=false;
		}
	}
}

void verificarEstructura(Cola<std::string>* despachadorMiembros,
std::vector<int>* miembrosArbol){

	int salir=0;
	int aux;
	int opcion;
	std::string nuevoMiembro;

	while(salir == 0){
		
		nuevoMiembro=despachadorMiembros->pop();
		std::stringstream s_stream(nuevoMiembro);
		std::vector<std::string> resultado;
		while(s_stream.good()) {
			std::string substr;
			getline(s_stream, substr, ',');
			resultado.push_back(substr);
		}
		
		aux=stoi(resultado[1]);
		opcion = stoi(resultado[0]);
		if(opcion == 46){
			int bandera = 0;
			size_t i = 0;
			while(i < miembrosArbol->size() && bandera == 0){
				if((*miembrosArbol)[i] == aux){
					miembrosArbol->erase(miembrosArbol->begin() + i);
					bandera = 1;
				}
				i++;
			}
		} else {
			if(opcion == 47){
				size_t i = 0;
				while(i< miembrosArbol->size()){
					miembrosArbol->erase (miembrosArbol->begin()+i);
					i++;
				}
			} else {
				if(opcion == 43 ||opcion == 44){				
					miembrosArbol->push_back(aux);
				}
			}
		}
	}
}

void timeout(){
	while(true){
		if(reiniciarEnvio == true){
			sleep(1);
			enviarDenuevo = true;
			reiniciarEnvio = false;
		}
	}
}

void broadcast(Cola<struct ForwardingAplicacion>* colaAzul,
std::vector<Cola<struct CapaEnlace>>* colasVerdes,
Cola<struct Broadcast>* colaBroadcast,
std::vector<int>* nodosIDs, Cola<std::string>* despachadorMiembros,
Cola<std::string>* colaAlcanzabilidad,
Cola<std::string>* colaEnviarAlcanzabilidad){

	std::vector<int> miembrosArbol;
	std::thread controladorTablaRosado(verificarEstructura,
	despachadorMiembros,&miembrosArbol);

	while(1){
		char buffer2[1040];
		struct Broadcast nuevoBroadcast = colaBroadcast->pop();
		struct CapaRed capaRed;
		struct CapaEnlace paquete;
		size_t longitud = nodosIDs->size();
		capaRed.tipo=0x02;
		char buffer [1017];
		size_t offset = 0;
		memmove(buffer, &nuevoBroadcast.tipo,
		sizeof(nuevoBroadcast.tipo));

		offset = offset + sizeof(nuevoBroadcast.tipo);
		memmove(buffer + sizeof(nuevoBroadcast.tipo),
		&nuevoBroadcast.id_origen_inicial,
		sizeof(nuevoBroadcast.id_origen_inicial));

		offset = offset + sizeof(nuevoBroadcast.id_origen_inicial);
		memmove(buffer + sizeof(nuevoBroadcast.tipo) +
		sizeof(nuevoBroadcast.id_origen_inicial),
		&nuevoBroadcast.longitud, sizeof(nuevoBroadcast.longitud));

		offset = offset + sizeof(nuevoBroadcast.longitud);
		memmove(buffer + sizeof(nuevoBroadcast.tipo) +
		sizeof(nuevoBroadcast.id_origen_inicial) +
		sizeof(nuevoBroadcast.longitud), &nuevoBroadcast.datos,
		sizeof(nuevoBroadcast.datos));
		
		offset = offset + nuevoBroadcast.longitud;
		memmove(&capaRed.datos,buffer ,sizeof(capaRed.datos));
		capaRed.longitud=offset;

		if(nuevoBroadcast.id_origen_inicial== (*nodosIDs)[0]){
			for(size_t i = 0; i < miembrosArbol.size(); ++i){
				for(size_t indice=1;indice<longitud;indice++){
					if(miembrosArbol[i]==(*nodosIDs)[indice]){
						paquete.tipo=0x02;
						int casteo=(*nodosIDs)[0];
						paquete.idFuenteInmediato =
						static_cast<uint16_t>(casteo);

						paquete.idDestinoFinal =
						static_cast<uint16_t>(miembrosArbol[i]);

						offset = 0;
						memmove(buffer2, &capaRed.tipo,
						sizeof(capaRed.tipo));

				        offset = offset + sizeof(capaRed.tipo);
						memmove(buffer2 + sizeof(capaRed.tipo),
						&capaRed.longitud, sizeof(capaRed.longitud));

				        offset = offset + sizeof(capaRed.longitud);
						memmove(buffer2 + sizeof(capaRed.tipo) +
						sizeof(capaRed.longitud), &capaRed.datos,
						sizeof(capaRed.datos));

						offset = offset + capaRed.longitud;
						capaRed.longitud = offset;
						memmove(&paquete.datos, buffer2,
						sizeof(paquete.datos));

						colasVerdes[0][indice].push(paquete);
					}
				}
			}
				
		} else {

			if(nuevoBroadcast.tipo == 0x01){
				struct ForwardingAplicacion nuevoMensaje;
				nuevoMensaje.tipoAplicacion = 0x01;
				char buffer3 [200];
				memmove(buffer3, &nuevoBroadcast.datos,
				sizeof(nuevoBroadcast.datos));

				memmove(&nuevoMensaje.datos, buffer3, sizeof(buffer3));
				nuevoMensaje.longitud = nuevoBroadcast.longitud;
				
				colaAzul->push(nuevoMensaje);
			} else if(nuevoBroadcast.tipo == 0x02){
				if(enviarDenuevo == true){
					colaAlcanzabilidad->push("45");
					enviarDenuevo = false;
					reiniciarEnvio = true;
				}
				char buffer3 [200];
				memmove(buffer3, &nuevoBroadcast.datos,
				sizeof(nuevoBroadcast.datos));

				uint16_t cantiadaVecinos;
				std::string mensajeAlcanzabilidad;
				memmove(&cantiadaVecinos, buffer3,
				sizeof(cantiadaVecinos));

				mensajeAlcanzabilidad = "90," +
				std::to_string(nodoSender) + "," +
				std::to_string(cantiadaVecinos);

				size_t offset=sizeof(cantiadaVecinos);
				for(uint16_t indice4 = 0; indice4 < cantiadaVecinos;
				indice4++){

					uint16_t idVecino;
					memmove(&idVecino, buffer3 + offset,
					sizeof(idVecino));

					offset=offset+sizeof(idVecino);
					uint8_t distancia;
					memmove(&distancia, buffer3 + offset,
					sizeof(distancia));

					offset=offset+sizeof(distancia);
					if(distancia == 0x01){
						char buffer[80];
   						int buffer_len = sprintf(buffer,
						"%d", idVecino);

					   	std::string str(buffer, buffer + buffer_len);
						mensajeAlcanzabilidad+=","+str+","+"1";
					}else if(distancia == 0x02){
						char buffer[80];
   						int buffer_len = sprintf(buffer,
						"%d", idVecino);

					   	std::string str(buffer, buffer + buffer_len );
						mensajeAlcanzabilidad+=","+str+","+"2";
					}					
				}
				colaEnviarAlcanzabilidad->push(mensajeAlcanzabilidad);
			}

			for(size_t i = 0; i < miembrosArbol.size(); ++i){
				for(size_t indice = 1; indice < longitud; indice++){
					int casteo4=(*nodosIDs)[indice];
					uint16_t casteo3 = static_cast<uint16_t>(casteo4);
					if(miembrosArbol[i]==(*nodosIDs)[indice] &&
					casteo3!= nuevoBroadcast.id_origen_inicial &&
					(*nodosIDs)[indice] != nodoSender){

						paquete.tipo=0x02;
						int casteo2=(*nodosIDs)[0];
						paquete.idFuenteInmediato =
						static_cast<uint16_t>(casteo2);

						paquete.idDestinoFinal =
						static_cast<uint16_t>(miembrosArbol[i]);
						size_t offset = 0;
						memmove(buffer2, &capaRed.tipo,
						sizeof(capaRed.tipo));

				    	offset = offset + sizeof(capaRed.tipo);
						memmove(buffer2 + sizeof(capaRed.tipo),
						&capaRed.longitud,
						sizeof(capaRed.longitud));

				        offset = offset + sizeof(capaRed.longitud);
						memmove(buffer2 + sizeof(capaRed.tipo) +
						sizeof(capaRed.longitud), &capaRed.datos,
						sizeof(capaRed.datos));

						offset = offset + longitud;
						memmove(&paquete.datos, buffer2,
						sizeof(paquete.datos));

						paquete.longitud=offset;
						colasVerdes[0][indice].push(paquete);
					}
				}
			}
		}
	}
	controladorTablaRosado.join();
}