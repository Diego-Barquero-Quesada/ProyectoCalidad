#ifndef AGENTE_VERDE_H
#define AGENTE_VERDE_H
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h> //exit function
#include <string.h> //strlen
#include <unistd.h> //write/read functions
#include <string.h> //strlen
#include <unistd.h> //write/read functions
#include <string>
#include <thread>
#include <vector>
#include <unistd.h>
#include <iostream>
#include "../../common/cola/Cola.h"
#include <fcntl.h>
#include<pthread.h>
#define LATIDO 1
#define RED 2

void recibirV(std::vector<datosNodo>*, Cola<struct CapaRed>*,
Cola<struct Latido> *);

void enviarV(size_t, std::vector<datosNodo>*,
std::vector<Cola<struct CapaEnlace>>*);

void latido(std::vector<datosNodo>*,
std::vector<Cola<struct CapaEnlace>>*);

void hiloVerde(std::vector<datosNodo>*,
std::vector<Cola<struct CapaEnlace>>*, Cola<struct CapaRed>*);

void verificarLatido(
std::vector<Cola<struct CapaEnlace>>* colasDeMensajes,
std::vector<datosNodo>* tablaVecinos,Cola<struct Latido> * colaLatido);

void timeoutLatido();

#endif //AGENTE_VERDE_H
