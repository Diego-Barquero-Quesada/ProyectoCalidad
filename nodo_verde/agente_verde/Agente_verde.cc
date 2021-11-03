#include "Agente_verde.h"
pthread_mutex_t lock;
bool terminar = false;
uint16_t nodoSender;
uint16_t destinoFinal;
short destino;

/**
    @brief Metodo que se encarga de generar un socket para recibir los
    mensajes de los vecinos
    @details Este metodo se encarga de generar un socket para recibir
    los mensajes de los vecinos y luego pasarlos a la cola de mensajes
    para que el cliente lo reciba
    @param[in] el metodo recibe un vector de un struct con los datos
    para generar el socket
    @param[out] Recibe una vector de cola de mensajes y se usa para el
    paso de mensajes
    @pre Que el vector datosNodo tenga datos
    @remark Se modifica la cola de mensajes
    @return N/A
    @exception N/A
    @author Diego Barquero Quesada B80961
    @date 12-11-20
*/

void recibirV(std::vector<datosNodo>* tabla,
Cola<struct CapaRed>* colaDespachadorVerde,
Cola<struct Latido> *colaLatido){

    char buffer [1047];
     
    unsigned int sock, length, fromlen;
    struct sockaddr_in server;
    struct sockaddr_in from;
   
    struct CapaEnlace paquete;
    struct CapaRed capaRed;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    length = sizeof(server);

    bzero(&server,length);
    server.sin_family=AF_INET;
    server.sin_addr.s_addr=INADDR_ANY;
    server.sin_port=htons(tabla[0][0].puerto);
    if (bind(sock,(struct sockaddr *)&server,length)<0){
        std::cout << "Error en binding" << std::endl;
    }

    fromlen = sizeof(struct sockaddr_in);

    size_t i;
   
    while(!terminar){
    
        recvfrom(sock, buffer, 1047, 0,
        (struct sockaddr *)&from, &fromlen); //se recibe mensaje
                
        memmove(&(paquete.tipo), buffer,sizeof(paquete.tipo));
        memmove(&(paquete.idDestinoFinal), buffer +
        sizeof(paquete.tipo), sizeof(paquete.idDestinoFinal));

        memmove(&(paquete.idFuenteInmediato), buffer +
        sizeof(paquete.tipo)+sizeof(paquete.idDestinoFinal),
        sizeof(paquete.idFuenteInmediato));

        memmove(&(paquete.longitud), buffer + sizeof(paquete.tipo) +
        sizeof(paquete.idDestinoFinal) +
        sizeof(paquete.idFuenteInmediato), sizeof(paquete.longitud));

        memmove(&(paquete.datos), buffer + sizeof(paquete.tipo) +
        sizeof(paquete.idDestinoFinal) +
        sizeof(paquete.idFuenteInmediato) + sizeof(paquete.longitud),
        sizeof(paquete.datos));
               
        if(paquete.tipo == 0x02){ //paquete de tipo red
            for(i = 0; i < tabla[0].size(); i++){
                if(tabla[0][i].ID ==
                static_cast<short>(paquete.idFuenteInmediato)){

                    pthread_mutex_lock(&lock);
                    tabla[0][i].tiempoExpiracion = 30;
                    pthread_mutex_unlock(&lock);
                }
            }
              
            nodoSender = paquete.idFuenteInmediato;
            destinoFinal = paquete.idDestinoFinal;

            memmove(&capaRed.tipo, paquete.datos,
            sizeof(capaRed.tipo));

            memmove(&capaRed.longitud, paquete.datos +
            sizeof(paquete.tipo), sizeof(capaRed.longitud));

            memmove(&capaRed.datos,paquete.datos +
            sizeof(paquete.tipo) + sizeof(capaRed.longitud),
            sizeof(capaRed.datos));
               
            colaDespachadorVerde->push(capaRed);
        } else if(paquete.tipo == 0x01){ //paquete de tipo latido
            struct Latido paqueteLatido;
            memmove(&paqueteLatido.tipo_latido, paquete.datos,
            sizeof(paqueteLatido.tipo_latido));

            destino = static_cast<short> (paquete.idFuenteInmediato);

            colaLatido->push(paqueteLatido);
        }
    }
    printf("Hilo recibir salió del while\n");
}

/**
    @brief Metodo que genera sockets para poder conectarse con los
    vecinos
    @details Este metodo recibe la informacion de los vecinos del nodo
    y lo ejecutan varios hilos para conectarse a los diferentes vecinos
    @param[in] Recibe un vector datosNodo con la informacion de los
    vecinos
    @param[out] Recibe una referencia al vector de colas
    @pre Que el vector datosNodo tenga datos
    @remark se modifica la cola de mensajes
    @return N/A
    @exception N/A
    @author Diego Barquero Quesada B80961
    @date 12-11-20
*/

void enviarV(size_t identificador, std::vector<datosNodo>* tabla,
std::vector<Cola<struct CapaEnlace>>* colasDeMensajes){

    int sock, n;
    char buffer [1047];
    
    unsigned int length;
    struct sockaddr_in server;
    struct hostent *hp;
    std::string p;
   
    struct CapaEnlace datos;
    struct CapaEnlace paquete;

    sock= socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0){
        std::cout << "Error al crear el socket" << std::endl;
    }

    server.sin_family = AF_INET;
    hp = gethostbyname(tabla[0][identificador].IP.data());

    if (hp==0){
        std::cout << "Error hots desconocido" << std::endl;
    }

    bcopy((char *)hp->h_addr,(char *)&server.sin_addr,hp->h_length);
    server.sin_port = htons(tabla[0][identificador].puerto);
    length=sizeof(struct sockaddr_in);
    while(!terminar){

        datos = colasDeMensajes[0][identificador].pop();

        if(datos.idDestinoFinal == (uint16_t)-1){
            terminar = true;
        } else {

            if(datos.tipo == 0x01 ||datos.tipo == 0x02 ){
            //paquete de tipo red o latido
              
                memmove(buffer, &(datos.tipo), sizeof(datos.tipo));
                memmove(buffer + sizeof(datos.tipo),
                &(datos.idDestinoFinal), sizeof(datos.idDestinoFinal));

                memmove(buffer + sizeof(datos.tipo) +
                sizeof(datos.idDestinoFinal),
                &(datos.idFuenteInmediato),
                sizeof(datos.idFuenteInmediato));

                memmove(buffer + sizeof(datos.tipo) +
                sizeof(datos.idDestinoFinal) +
                sizeof(datos.idFuenteInmediato), &(datos.longitud),
                sizeof(datos.longitud));

                memmove(buffer + sizeof(datos.tipo) +
                sizeof(datos.idDestinoFinal) +
                sizeof(datos.idFuenteInmediato) +
                sizeof(datos.longitud), &(datos.datos),
                sizeof(datos.datos));

                memmove(&(paquete.tipo), buffer,sizeof(paquete.tipo));
                memmove(&(paquete.idDestinoFinal), buffer +
                sizeof(paquete.tipo), sizeof(paquete.idDestinoFinal));

                memmove(&(paquete.idFuenteInmediato), buffer +
                sizeof(paquete.tipo) + sizeof(paquete.idDestinoFinal),
                sizeof(paquete.idFuenteInmediato));

                memmove(&(paquete.longitud), buffer +
                sizeof(paquete.tipo) + sizeof(paquete.idDestinoFinal) +
                sizeof(paquete.idFuenteInmediato),
                sizeof(paquete.longitud));

                memmove(&(paquete.datos), buffer +
                sizeof(paquete.tipo) + sizeof(paquete.idDestinoFinal) +
                sizeof(paquete.idFuenteInmediato) +
                sizeof(paquete.longitud), sizeof(paquete.datos));
                
                n = sendto(sock, buffer, sizeof(buffer),
                0, (struct sockaddr*)&server, length); //envia paquete
                if(n < 0){
                    std::cout << "Error en envío" << std::endl;
                }
            } else {
                 std::cout << "Error!!!!\n";
            }
        }
    }
    printf("Hilo %ld salió del while\n", identificador);
}

/**
    @brief Metodo que genera solicitud de latido
    @details Metodo que se encarga de generar las solicitudes de latido
    @param[in] N/A
    @param[out] Recibe  colas para manejar mensajes 
    @pre Que las colas esten correctamente inicializadas
    @remark se modifican las colas
    @return N/A
    @exception N/A
    @author Diego Barquero Quesada B80961
    @date 12-11-20
*/
void latido(std::vector<datosNodo>* tabla,
std::vector<Cola<struct CapaEnlace>>* colasDeMensajes){

    struct CapaEnlace paquete;
    struct Latido paqueteLatido;
    size_t i;

    int tiempoLatido = 0;

    while(!terminar){
        for(i=1; i<tabla[0].size(); i++){
           
            if(tiempoLatido == 0){ //se genera solicitud
                paqueteLatido.tipo_latido = 0x01;
                paquete.tipo = 0x01;
                paquete.idDestinoFinal =
                static_cast<uint16_t>(tabla[0][i].ID);

                paquete.idFuenteInmediato =
                static_cast<uint16_t>(tabla[0][0].ID);

                char buffer2 [1040];
                memmove(buffer2, &paqueteLatido.tipo_latido,
                sizeof(paqueteLatido.tipo_latido));

                memmove(&paquete.datos, buffer2,
                sizeof(paquete.datos));

                paquete.longitud = sizeof(paqueteLatido.tipo_latido);
                tiempoLatido = 5;
                colasDeMensajes[0][i].push(paquete);
            } else {
                pthread_mutex_lock(&lock);
                tabla[0][i].tiempoExpiracion--;
                pthread_mutex_unlock(&lock);
            } 
        }
        
        tiempoLatido--;

        sleep(1);
        for(i=1; i<tabla[0].size(); i++){
        //ciclo para revisar si esta muerto o vivo

            if(tabla[0][i].tiempoExpiracion <= 0 ){
                std::cout << "Vecino " << tabla[0][i].ID <<
                " muerto" << std::endl;
                tabla[0][i].estado = 0;
            }
            else{
                std::cout << "Vecino " << tabla[0][i].ID <<
                " vivo" << std::endl;

                std::cout << "Tiempo" <<
                tabla[0][i].tiempoExpiracion << std::endl;
            }
        }
    }
}

/**
    @brief Metodo que verifica latido
    @details Metodo que se encarga de verificar las solicitudes que
    recibe de solicitudes de latido
    @param[in] N/A
    @param[out] Recibe  colas para manejar mensajes y un vector con
    informacion de los nodos
    @pre Que las colas esten correctamente inicializadas
    @remark se modifican las colas
    @return N/A
    @exception N/A
    @author Diego Barquero Quesada B80961
    @date 12-11-20
*/

void verificarLatido(
std::vector<Cola<struct CapaEnlace>>* colasDeMensajes,
std::vector<datosNodo>* tabla,Cola<struct Latido> * colaLatido){
    
    while(!terminar){
        struct Latido paqueteLatido = colaLatido->pop();
        //recibimos solicitud de latido

        if(paqueteLatido.tipo_latido == 0X01 ){
        //se genera solicitud de respuesta que si estoy vivo

            for(size_t i = 0; i<tabla[0].size(); i ++){
                if(tabla[0][i].ID == destino){
                    struct Latido nuevoPaqueteLatido;
                    nuevoPaqueteLatido.tipo_latido = 0X02;
                    struct CapaEnlace paquete;
                    paquete.tipo = 0x01;
                    paquete.idDestinoFinal =
                    static_cast<uint16_t>(tabla[0][i].ID);

                    paquete.idFuenteInmediato =
                    static_cast<uint16_t>(tabla[0][0].ID);

                    char buffer2 [1040];
                    memmove(buffer2, &nuevoPaqueteLatido.tipo_latido,
                    sizeof(nuevoPaqueteLatido.tipo_latido));

                    memmove(&paquete.datos, buffer2,
                    sizeof(paquete.datos));
                
                    paquete.longitud =
                    sizeof(nuevoPaqueteLatido.tipo_latido);
                    
                    colasDeMensajes[0][i].push(paquete);
                }
            }
        } else if(paqueteLatido.tipo_latido == 0X02 ){
            //se recibe solicitud de confirmacion que si

            for(size_t i = 0; i<tabla[0].size(); i ++){
                if(tabla[0][i].ID == destino){
                    tabla[0][i].estado = 1;
                    pthread_mutex_lock(&lock);
                    tabla[0][i].tiempoExpiracion = 30;
                    pthread_mutex_unlock(&lock);
                }
            }
        }
    }
}

/**
    @brief Metodo del agente verde
    @details Metodo que ejecuta el agente verde que se encarga de
    generar los hilos de enviar y recibir y controlar latido
    @param[in] Recibe una cola para controlar latido
    @param[out] Recibe  colas para manejar mensajes y un vector con
    informacion de los nodos
    @pre Que las colas esten correctamente inicializadas
    @remark se generan los hilos
    @return N/A
    @exception N/A
    @author Diego Barquero Quesada B80961
    @date 12-11-20
*/
void hiloVerde(std::vector<datosNodo>* tablaVecinos,
std::vector<Cola<struct CapaEnlace>>* colasDeMensajes,
Cola<struct CapaRed>* colaDespachadorVerde){

    Cola<struct Latido> colaLatido;
    
    std::thread conexiones[tablaVecinos[0].size()-1];
    std::thread recibir = std::thread(recibirV, tablaVecinos,
    colaDespachadorVerde,&colaLatido); //hilo para recibir 
    
    pthread_mutex_init(&lock, NULL);

    size_t i;

    for(i=0; i<tablaVecinos[0].size()-1; i++){
        conexiones[i] = std::thread(enviarV, i+1,
        tablaVecinos, colasDeMensajes);
        //hilos para conectar con vecinos
    }

    std::thread hiloLatido = std::thread(latido, tablaVecinos,
    colasDeMensajes); //hilo para generar solicitudes de latido

    std::thread verificarLatidoHilo = std::thread(verificarLatido,
    colasDeMensajes,tablaVecinos,&colaLatido);
    //hilo para verificar solicitudes de latido

    for(i=0; i<tablaVecinos[0].size()-1; i++){
        conexiones[i].join();
    }

    recibir.join();
    verificarLatidoHilo.join();
    hiloLatido.join();
    pthread_mutex_destroy(&lock);
}