#ifndef AGENTE_NARANJA_H
#define AGENTE_NARANJA_H
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
//constantes numéricas
#define STD_SIZE 2
#define MAX_BUF 1024
#define PERMISOS_FIFO 0666
#define CANT_MIN_ARCH 7
#define CONF_SIZE 3
#define MIN_BYTES_VER 10
#define MAX_REINICIO 5
#define INTERPRETADOR "python3"
#define CAMINO_NODO_NARANJA "src/nodo_naranja/Nodo_naranja.py"
//Constantes de texto
#define COMP_2_BYTES "0"
#define CODIGO_CONFIRMACION "777"
#define CODIGO_AVISO_ENVIO_PID "100"
#define CODIGO_FINALIZACION_PID "101"
#define CODIGO_AVISO_ENV_VER "102"
#define CODIGO_AVISO_REC_VER "103"
#define CODIGO_AVISO_ENV_VEC "104"
#define CODIGO_AVISO_REC_VEC "106"
#define ERR_CREAR_NN "No se pudo crear el nodo naranja"
#define ERR_PIPE_CREAR_L "No se creó pipe de lectura"
#define ERR_PIPE_CREAR_E "No se creó pipe de escritura"
#define ERR_PIPE_ABRIR_L "No se abrió el pipe de lectura"
#define ERR_PIPE_BORRAR_L "No se eliminó pipe de lectura"
#define ERR_PIPE_BORRAR_E "No se eliminó pipe de escritura"
#define ERR_PIPE_ABRIR_L "No se abrió el pipe de lectura"
#define ERR_PIPE_ABRIR_E "No se abrió el pipe de escritura"
#define ERR_PIPE_CERRAR_L "No se cerró pipe de lectura"
#define ERR_PIPE_CERRAR_E "No se cerró pipe de escritura"


//se definen las funciones a utilizar
void trabajoAgenteNaranja(char*, char*, char*, char*);
void enviarConfirmacion(int, int, char*, char*);
void quitarPipe(char*, std::string);
std::vector<std::string> comunicarConNodoNaranja(int, char*, char*);
void comunicacionPid(char*, char*, int);
void cerrarPipe(int, std::string, char*, std::string);
void escribirPipe(std::string, std::string, char*, char*);
void escribirPipeTam(std::string, std::string, char*, char*);
void escribirPipeMen(std::string, std::string, char*, char*);
std::string leerPipe(std::string, char*, char*);
size_t leerPipeTam(std::string, char*, char*);
std::string leerPipeMen(std::string, char*, char*, size_t);
void revisarCodigos(std::string, std::string, std::string);

std::vector<std::string> comunicacionInfoVerde(char*, char*,
std::vector <std::string>);

std::vector<std::string> comunicacionVecinos(char*, char*,
std::vector <std::string>);

std::vector<std::string> recibirInfoArchivo(std::string, std::string,
char*, char*, std::vector <std::string>);

#endif //AGENTE_NARANJA_H
