#ifndef AGENTE_AZUL_H
#define AGENTE_AZUL_H

	#include <fcntl.h>
	#include <sys/stat.h>
	#include <unistd.h>
	#include <string.h>
	#include <iostream>
	#include <thread>
	#include <fstream>
	#include "../../common/cola/Cola.h"

	#define LONGITUD 207
	#define LONG_ARCHIVO 1013
	#define FORWARDING 1
	#define BROADCAST 2
	#define THREE_WAY 0x01
	#define STOP_AND_WAIT 0x02
	#define TWO_WAY 0x03
	#define MENSAJE_CORTO 0x01
	#define TRANSMISION 0x02

	// Funciones
	void hiloAzul(std::vector<int>*, Cola<struct FowardingAplicacion>*,
		char*, char*, char*, Cola<struct DatosFowardingAplicacion>*,
		Cola<uint16_t>*);

	void enviar(Cola<struct FowardingAplicacion>*, Cola<uint16_t>*,
		char*);
	
	void recibir(Cola<struct DatosFowardingAplicacion>*, char*, 
		Cola<struct FowardingAplicacion>*, Cola<uint16_t>*);

	void mostrarError(const char*);

	void handShake(int, Cola<struct DatosFowardingAplicacion>*);

	void respuestaHandShake(int, char*, int);

	void transmisionConfiable(int, 
		Cola<struct DatosFowardingAplicacion>*);

	void respuestaTransmision(int, char*, int);

	void dividir(char*, const char*, bool, std::vector<char*>*);

#endif //AGENTE_AZUL_H