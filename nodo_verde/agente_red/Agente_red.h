#ifndef CAPA_RED_H
#define CAPA_RED_H

#include <thread>
#include <mutex>
#include "../../common/cola/Cola.h"
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <vector>
#include <string.h>
#define FORWARDING 1
#define BROADCAST 2
#define ARBOL 3
#define MENSAJE 1
#define ALCANZABILIDAD 2
#define RED 2

// Funciones

//Función principal de la capa de red
void capaRed(Cola<struct ForwardingAplicacion>*,
Cola<struct ArbolGenerador>*,
std::vector<Cola<struct CapaEnlace>>*,
Cola<struct DatosForwardingAplicacion>*,
Cola<struct DatosArbolGenerador>*, Cola<struct CapaRed>*,
std::vector<int>*, Cola<std::string>*, Cola<std::string>*,
std::vector<datosNodo>*, char*, Cola<std::string>*,Cola<std::string>*);

//despachador agente azul
void despachadorAzul(Cola<struct DatosForwardingAplicacion>*,
Cola<struct DatosForwarding>*, Cola<struct Broadcast>*, int);

//despachador agente rosado
void despachadorRosado(Cola<struct DatosArbolGenerador>* ,
std::vector<int>*,
std::vector<Cola<struct CapaEnlace>>*);

//despachador agente verde
void despachadorVerde(Cola<struct CapaRed>*,
Cola<struct DatosForwarding>*, Cola<struct Broadcast>*,
Cola<struct ArbolGenerador>*);

//forwarding
void forwarding(Cola<struct ForwardingAplicacion>*,
std::vector<Cola<struct CapaEnlace>>*,
Cola<struct DatosForwarding>*, std::vector<int>*g);

//broadcast

void broadcast(Cola<struct ForwardingAplicacion>*,
std::vector<Cola<struct CapaEnlace>>*,
Cola<struct Broadcast>*, std::vector<int>*, Cola<std::string>*,
Cola<std::string>*, Cola<std::string>*);

void verificarEstructura(Cola<std::string>*,std::vector<int>*);

void enviarAlcanzabilidad(Cola<std::string>*, std::vector<datosNodo>*,
char*, Cola<struct Broadcast>*, Cola<std::string>*);

void verificarTablaForwarding(Cola<std::string>*, std::vector<int>*);

void timeout();

#endif //AGENTE_RED_H
