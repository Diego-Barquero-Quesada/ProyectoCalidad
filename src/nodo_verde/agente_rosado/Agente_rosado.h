#ifndef AGENTE_ROSADO_H
#define AGENTE_ROSADO_H
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <cerrno>
#include <vector>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdexcept>
#include <thread>
#include <sstream>
#include "../Nodo_verde.h"
#include "../../common/cola/Cola.h"
#include <sys/ioctl.h>

#define INTERPRETADOR "python3"
#define CAMINO_NODO_ROSADO "src/nodo_rosado/Nodo_rosado.py"
#define ERR_CREAR_NR "No se pudo crear el nodo rosado"

struct indiceNodo{
  int id;
  int indice;
};

void clienteTCP(Cola<std::string>*,int);
void serverTCP(Cola<std::string>*,Cola<std::string>*,int);
void trabajoAgenteRosado(std::vector<datosNodo>*,
Cola<struct ArbolGenerador>*,Cola<struct DatosArbolGenerador>*, char*,
char*,Cola<std::string>*,Cola<std::string>*,Cola<std::string>*,
Cola<std::string>*
);

void enviarEnlace(Cola<struct DatosArbolGenerador>*,Cola<std::string>*,
Cola<std::string>*,Cola<std::string>*,Cola<std::string>*);

void recibirEnlace(Cola<struct ArbolGenerador>*,Cola<std::string>*);
void inicializarNodoRosado(std::string, std::vector<indiceNodo>,
Cola<std::string>*,Cola<std::string>*, std::vector<datosNodo>*);

void enviarAlcanzabilidad2(Cola<std::string> *,Cola<std::string>*);

#endif //AGENTE_ROSADO_H