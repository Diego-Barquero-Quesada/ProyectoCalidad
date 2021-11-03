#ifndef AGENTE_AZUL_H
#define AGENTE_AZUL_H

	#include <fcntl.h>
	#include <sys/stat.h>
	#include <unistd.h>
	#include <string.h>
	#include <iostream>
	#include <thread>
	#include <mutex>
	#include <fstream>
	#include "../../common/cola/Cola.h"

	#define LONGITUD 207
	#define FORWARDING 1
	#define BROADCAST 2

	// Funciones
	void hiloAzul(std::vector<int>*,Cola<struct ForwardingAplicacion>*,
	char*, char*, char*, Cola<struct DatosForwardingAplicacion>*);

	void enviar(Cola<struct ForwardingAplicacion>*, char*);
	
	void recibir(Cola<DatosForwardingAplicacion>*, char*, 
	Cola<struct ForwardingAplicacion>*);   

	void mostrarError(const char*);

#endif //AGENTE_AZUL_H
