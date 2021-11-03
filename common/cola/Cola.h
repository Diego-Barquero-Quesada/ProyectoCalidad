//Modificado de: https://gist.github.com/ictlyh/f8473ad0cb1008c6b32c41f3dea98ef5#file-producer-consumer-L25
//Autor Original: Yuanhao Luo "ictlyh"
//Fecha de última modificación: 11/12/20

#ifndef COLA_H
#define COLA_H

#include <queue>
#include <mutex>
#include <condition_variable>

#define SIZE 200

extern uint16_t nodoSender;
extern uint16_t nodoSenderTWH;
extern uint16_t destinoFinal;

struct datosNodo{
    short ID;
    std::string IP;
    short puerto;
    uint8_t estado = 0;
    int16_t tiempoExpiracion = 0;
};

struct Mensaje{
    char buffer[200] = {};
};

struct Alcanzabilidad{
    uint16_t longitud;
    uint16_t idVecino;
    uint8_t distancia;
};

struct Forwarding{
    uint16_t idFuenteInicial;
    uint16_t longitud;
    char datos [1013] = {};
};

struct ArbolGenerador{
    uint8_t tipo;
    uint16_t SN;
    uint16_t RN;
};

// Estructuras especiales
struct Latido{
    uint8_t tipo_latido; 
};

// Agente azul -> Despachador azul
struct DatosMensaje{
    char buffer[200] = {};
    uint16_t idDestino;
    uint8_t tipo;               // Forwarding 1 || Broadcast 2
};

struct Broadcast{
    uint8_t tipo;              // Mensaje normal 1 || Alcanzabilidad 2
    uint16_t id_origen_inicial;
    uint16_t longitud;
    char datos[200] = {};
};

struct CapaRed{
    uint8_t tipo;
    uint16_t longitud;
    char datos [1017]={};
};

struct DatosForwarding{
    uint16_t idFuenteInicial;
    uint16_t idDestino;
    uint16_t longitud;
    char datos [1013]={};
};

struct DatosForwardingAplicacion{
    uint8_t tipo;
    uint8_t tipoAplicacion; 
    uint16_t longitud;
    char datos[1010] = {};
    uint16_t idDestino;
};

struct ForwardingAplicacion{
    uint8_t tipoAplicacion; 
    uint16_t longitud;
    char datos[1010] = {};
};

struct DatosArbolGenerador{
    uint8_t tipo;
    uint16_t SN;
    uint16_t RN;
    uint16_t id_destino_final;
};

struct CapaEnlace{
    uint8_t tipo;               // Latido 1 || Capa red 2
    uint16_t idDestinoFinal;
    uint16_t idFuenteInmediato;
    uint16_t longitud;
    char datos [1040];
};

template <typename T>

class Cola{

    private:
        std::queue<T> cola;
        std::mutex *m = new std::mutex();
        std::condition_variable *condicion =
        new std::condition_variable();

    public:

        Cola()=default;
        //Cola(const Cola&) = delete;

        T pop();
        void push(const T&);
};

#endif //COLA_H