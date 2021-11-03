#include "Agente_naranja.h"


/**
\brief Llamar a los métodos para
crear los pipes de lectura y escritura, crear al nodo naranja,
borrar los pipes y enviar la información recibida al nodo verde
\details la subrutina crea los pipes de r-l para que puedan ser
utilizados por el nodo y el agente naranja. Además crea al proceso
naranja para que se pueda comunicar con el nodo naranja. Por último
se envían los datos de lo vecinos y del nodo verde al nodo verde y
se cierran los pipes.
\param[in] Los nombres que tendrán los pipes a utilizar por el nodo y
el agente naranja, además de las rutas de los archivos del nodo verde
y sus vercinos
\param[out] No aplica
\pre No hay precondiciones
\remark No aplica
\return No retorna ningún valor
\exception No aplica
\author Juan José Herrera
\date 30-9-20
*/
int fdl;
int fde;

void trabajoAgenteNaranja(char*caminoFifoAgente,
char *caminoFifoNodo,char* caminoNodoVerde,char* caminoVecinos){
    if(mkfifo(caminoFifoAgente, PERMISOS_FIFO) == -1){
        throw std::runtime_error(ERR_PIPE_CREAR_E);
    }
    if(mkfifo(caminoFifoNodo, PERMISOS_FIFO) == -1){
        quitarPipe(caminoFifoAgente,ERR_PIPE_BORRAR_E);
        throw std::runtime_error(ERR_PIPE_CREAR_L);
    }

    int pid = fork();//Se crea el nodo naranja
    if(pid == 0){
        execlp(INTERPRETADOR,INTERPRETADOR,
        CAMINO_NODO_NARANJA,
        caminoFifoAgente,caminoFifoNodo,
        caminoNodoVerde,caminoVecinos,(char*)NULL);
        exit(1);
    }
    else{
        if(pid > 0){//Nodo naranja
            std::vector < std::string > infoVerde;
            infoVerde = comunicarConNodoNaranja(pid,caminoFifoAgente,
            caminoFifoNodo);
            depositarDatos(infoVerde);
    }
    else{
        quitarPipe(caminoFifoAgente,ERR_PIPE_BORRAR_E);
        quitarPipe(caminoFifoNodo,ERR_PIPE_BORRAR_L);
        throw std::runtime_error(ERR_CREAR_NN);
    }
  }
  waitpid(pid,NULL,0);//Se espera por del nodo naranja
  quitarPipe(caminoFifoAgente,ERR_PIPE_BORRAR_E);
  quitarPipe(caminoFifoNodo,ERR_PIPE_BORRAR_L);
}

/**
\brief Eliminar los pipes seleccionados
\details Se pretende eliminar los pipes seleccionados para
no tener archivos basura para el usuario una vez que
la ejecución haya terminado
\param[in] El camino del pipe a borrar y su respectivo mensaje
de error
\param[out] No aplica
\pre Que el pipe que se quiera borrar exista
\remark No aplica
\return No retorna ningún valor
\exception Cuando no se elimina el pipe correctamente
se tira mensaje de de error
\author Juan José Herrera
\date 30-9-20
*/
void quitarPipe(char * camino,std::string e){
    if(unlink(camino) == -1){
        throw std::runtime_error(e);
    }
}

/**
\brief Recibir información leída de un archivo por un proceso
externo y introducirla en un vector de strings
\details Por medio de los pipes, se reciben mensajes del nodo
naranja para que puedan ser almacendos en un vector de strings
\param[in] El código para parar de recibir mensajes, las
rutas de los archivos de los pipes, el vector de
mensaje y el código de error en caso que la lectura
por los pipes fuera errónea
\param[out] No aplica
\pre Que exista el pipe de lectura
\remark No aplica
\return El vector de mensajes, para que pueda ser manipulado
por el agente naranja
\exception Cuando la lectura por el pipe de lectura fue errónea
\author Juan José Herrera
\date 30-9-20
*/
std::vector<std::string> recibirInfoArchivo(
std::string codigo,std::string e,
char *caminoFifoAgente,
char *caminoFifoNodo,std::vector < std::string > infoVerde){

    std::string mensaje;
    while(strcmp(mensaje.data(),codigo.data()) != 0){
        mensaje = leerPipe(e,
        caminoFifoAgente,caminoFifoNodo);

        if(strcmp(mensaje.data(),
            codigo.data()) != 0){
            infoVerde.push_back(mensaje);
        }
    }
    return infoVerde;
}

/**
\brief Enviar el aviso para recibir la información de los vecinos y
recibir la información de los vecinos
\details Se envía el código al nodo naranja para avisar que se va a
recibir la información de los vecinos y luego se recibe la
ción de los vecinos
\param[in] El vector de strings y las rutas de los pipes
\param[out] No aplica
\pre Que exista el pipe de lectura y el pipe de escritura
\remark No aplica
\return El vector de mensajes actualizado,
para que pueda ser manipulado por el agente naranja
\exception Cuando la lectura por el pipe de lectura fue errónea o
la escritura por el pipe de escritura fue errónea
\author Juan José Herrera
\date 30-9-20
*/
std::vector < std::string > comunicacionVecinos(
char *caminoFifoAgente,char *caminoFifoNodo,
std::vector < std::string > infoVerde){

    escribirPipe(CODIGO_AVISO_ENV_VEC,
    "No se envio aviso para envio de vecinos",
    caminoFifoAgente,caminoFifoNodo);

    infoVerde = recibirInfoArchivo(CODIGO_AVISO_REC_VEC,
    "No se envio aviso para recibo de vecinos",
    caminoFifoAgente,
    caminoFifoNodo,infoVerde);

    return infoVerde;
  }


/**
\brief Enviar el aviso para recibir la información del nodo verde y
recibir la información del nodo verde
\details Se envía el código al nodo naranja para avisar que se va a
recibir la información del nodo verde y luego se recibe la
información del nodo verde
\param[in] El vector de strings y las rutas de los pipes
\param[out] No aplica
\pre Que exista el pipe de lectura y el pipe de escritura
\remark No aplica
\return El vector de mensajes actualizado,
para que pueda ser manipulado por el agente naranja
\exception Cuando la lectura por el pipe de lectura fue errónea o
la escritura por el pipe de escritura fue errónea
\author Juan José Herrera
\date 30-9-20
*/
std::vector < std::string > comunicacionInfoVerde(
char *caminoFifoAgente,char *caminoFifoNodo,
std::vector < std::string > infoVerde){

    escribirPipe(CODIGO_AVISO_ENV_VER,
    "No se envio aviso para envio de info ver",
    caminoFifoAgente,caminoFifoNodo);

    infoVerde = recibirInfoArchivo(CODIGO_AVISO_REC_VER,
    "No se envio aviso para recibo de info ver",
    caminoFifoAgente
    ,caminoFifoNodo,infoVerde);

    return infoVerde;
  }


/**
\brief Realizar las operaciones de comunicación con el nodo naranja,
para obtener la información del nodo naranja y los vecinos
\details se envía el pid al nodo narnaja y luego se procede a
recibir por los pipes la información del nodo verde y los vecinos
\param[in] Las rutas de los archivos de pipes y el pid
\param[out] No aplica
\pre Que exista el pipe de lectura y el pipe de escritura
\remark No aplica
\return El vector de mensajes actualizado,
para que pueda ser manipulado por el agente naranja
\exception Cuando la lectura por el pipe de lectura fue errónea o
la escritura por el pipe de escritura fue errónea
\author Juan José Herrera
\date 30-9-20
*/
std::vector < std::string > comunicarConNodoNaranja(int pid,
char *caminoFifoAgente,char *caminoFifoNodo){

    std::vector < std::string > infoVerde;
    fdl = open(caminoFifoNodo, O_RDONLY);
    if(fdl == -1){
        quitarPipe(caminoFifoNodo,ERR_PIPE_BORRAR_L);
        throw std::runtime_error(ERR_PIPE_ABRIR_L);
    }
    fde = open(caminoFifoAgente, O_WRONLY);
    if(fde == -1){
        quitarPipe(caminoFifoNodo,ERR_PIPE_BORRAR_L);
        quitarPipe(caminoFifoAgente,ERR_PIPE_BORRAR_E);
        throw std::runtime_error(ERR_PIPE_ABRIR_E);
    }
    comunicacionPid(caminoFifoAgente,caminoFifoNodo,
    pid);

    infoVerde = comunicacionInfoVerde(caminoFifoAgente
    ,caminoFifoNodo,infoVerde);

    infoVerde = comunicacionVecinos(caminoFifoAgente
    ,caminoFifoNodo,infoVerde);
    cerrarPipe(fdl,ERR_PIPE_CERRAR_L,
    caminoFifoNodo,ERR_PIPE_BORRAR_L);
    cerrarPipe(fde,ERR_PIPE_CERRAR_E,
    caminoFifoAgente,ERR_PIPE_BORRAR_E);
    return infoVerde;
}

/**
\brief Comunicarse con en nodo naranja para enviarle su pid
\details Se envía el del aviso de emvío de pid, luego se envía
el pid al nodo naranja y por último se recibe el mensaje de
finalización de este sector
\param[in] Las rutas de los archivos de pipes y el pid
\param[out] No aplica
\pre Que exista el pipe de lectura y el pipe de escritura
\remark No aplica
\return No retorna
\exception Cuando la lectura por el pipe de lectura fue errónea o
la escritura por el pipe de escritura fue errónea
\author Juan José Herrera
\date 30-9-20
*/
void comunicacionPid(char *caminoFifoAgente,
char *caminoFifoNodo,int pid){

    escribirPipe(CODIGO_AVISO_ENVIO_PID,
    "No se envió el aviso del pid",
    caminoFifoAgente,caminoFifoNodo);
    std::string stringPid = std::to_string(pid);

    escribirPipe(stringPid,"No se envió del pid",
    caminoFifoAgente,caminoFifoNodo);

    std::string codFinalizacion =
    leerPipe("confirmación de pid inválida",
    caminoFifoAgente,caminoFifoNodo);

    revisarCodigos(codFinalizacion,
    CODIGO_FINALIZACION_PID ,"No se envió correctamente el pid");
}


/**
\brief Cerrar el file descriptor del pipe seleccionado
\details Se cierra el file desciptro del pipe seleccionado
para evitar envío de datos sucios y evitat dejarlo
abierto sin razón
\param[in] Las ruta del pipe a cerrrar, el file descriptor,
el mensaje de error en caso de borrar le pipe y el error en
caso de cerrar el file descriptor
\param[out] El file descriptor ya que se cierra
\pre Que exista el pipe de lectura y el pipe de escritura y que
el fd esté abierto
\remark No aplica
\return No retorna
\exception Cuando no se pudo borrar el pipe
\author Juan José Herrera
\date 30-9-20
*/
void cerrarPipe(int fd,std::string e,char *caminoFifo,
std::string eBorrar){

    if(close(fd)== -1){
        quitarPipe(caminoFifo,eBorrar);
        throw std::runtime_error(e);
    }
}

/**
\brief Mandar por el pipe de escritura el tamaño del mensaje
y el mensaje a enviar
\details Se envía la cantidad de caracteres que tendrá el mensaje
a enviar y posteriormente se envía el mensaje. Entre estas
operaciones hay lecturas para sincronizar los procesos
\param[in] El mensaje a enviar, el error de escritura, y las
rutas de los pipes
\param[out] Ninguno
\pre Que exista el pipe de lectura y el pipe de escritura y que
el fd esté abierto
\remark No aplica
\return No retorna
\exception Cuando la lectura por el pipe de lectura fue errónea o
la escritura por el pipe de escritura fue errónea
\author Juan José Herrera
\date 30-9-20
*/
void escribirPipe(std::string msj,std::string e,
char * caminoFifoAgente,char * caminoFifoNodo){

    escribirPipeTam(msj,e,caminoFifoAgente,caminoFifoNodo);

    size_t tamano = leerPipeTam(e,
    caminoFifoAgente,caminoFifoNodo);

    escribirPipeMen(msj,e,caminoFifoAgente,caminoFifoNodo);

    std::string men = leerPipeMen(e,caminoFifoAgente,caminoFifoNodo,
    tamano);

}

/**
\brief Leer por el pipe de lectura el tamaño del mensaje
y el mensaje recibidos
\details Se recibe la cantidad de caracteres que tendrá el mensaje
a enviar y posteriormente se envía el mensaje. Entre estas
operaciones hay escrituras para sincronizar los procesos
\param[in] El error de escritura, y las
rutas de los pipes
\param[out] Ninguno
\pre Que exista el pipe de lectura y el pipe de escritura y que
el fd esté abierto
\remark No aplica
\return El mesaje leído
\exception Cuando la lectura por el pipe de lectura fue errónea o
la escritura por el pipe de escritura fué errónea
\author Juan José Herrera
\date 30-9-20
*/
std::string leerPipe(std::string e,
char *caminoFifoAgente,char *caminoFifoNodo){

    size_t tamano = leerPipeTam(e,
    caminoFifoAgente,caminoFifoNodo);

    escribirPipeTam(CODIGO_CONFIRMACION
    ,e,caminoFifoAgente,caminoFifoNodo);

    std::string msj = leerPipeMen(e,
    caminoFifoAgente,caminoFifoNodo,tamano);

    escribirPipeMen(CODIGO_CONFIRMACION
    ,e,caminoFifoAgente,caminoFifoNodo);

    return msj;

}

/**
\brief Mandar por el pipe de escritura el tamaño del mensaje
\details Se envía la cantidad de caracteres que tendrá el mensaje
a enviar. Debe se ser 2 dos caracters. Si no lo es se le
agrega un caracter extra. Se crea y se abre el file descriptor
para facilitar una ecritura más limpia de la información
\param[in] El mensaje a enviar, el error de escritura y
el camino del pipe de escritura
\param[out] Ninguno
\pre Que exista el pipe de lectura y el pipe de escritura y que
el fd esté abierto
\remark No aplica
\return No retorna
\exception Cuando la lectura por el pipe de lectura fue errónea o
la escritura por el pipe de escritura fue errónea
\author Juan José Herrera
\date 30-9-20
*/
void escribirPipeTam(std::string msj,std::string e,
char *caminoFifoAgente,char *caminoFifoNodo){

    std::string tamanoMen = std::to_string(strlen(msj.data()));

    if(strlen(msj.data())<MIN_BYTES_VER){//Debe ser de 2 caracteres
        tamanoMen = COMP_2_BYTES+tamanoMen;//Si no lo es se le agrega
                                       //un caracter extra
    }

    size_t size = 2;

    if(write(fde, tamanoMen.data(),size) == -1){
        cerrarPipe(fde,ERR_PIPE_CERRAR_E,
        caminoFifoAgente,ERR_PIPE_BORRAR_E);
        quitarPipe(caminoFifoAgente,ERR_PIPE_BORRAR_E);
        cerrarPipe(fdl,ERR_PIPE_CERRAR_L,
        caminoFifoNodo,ERR_PIPE_BORRAR_L);
        quitarPipe(caminoFifoNodo,ERR_PIPE_BORRAR_L);
        throw std::runtime_error(e);
    }
}

/**
\brief Mandar por el pipe de escritura el mensaje que se va a enviar
\details Se envía el mensaje deseado por el pipe a otro porceso.
Se crea y se abre el file descriptor
para facilitar una ecritura más limpia de la información
\param[in] El mensaje a enviar,el error de escritura y el camino
del pipe de escritura
\param[out] Ninguno
\pre Que exista el pipe de lectura y el pipe de escritura y que
el fd esté abierto
\remark No aplica
\return No retorna
\exception Cuando la lectura por el pipe de lectura fue errónea o
la escritura por el pipe de escritura fué errónea
\author Juan José Herrera
\date 30-9-20
*/
void escribirPipeMen(std::string msj,std::string e,
char *caminoFifoAgente ,char *caminoFifoNodo){

    std::string tamanoMen = std::to_string(strlen(msj.data()));
    if(write(fde, msj.data(),strlen(msj.data())) == -1){
        cerrarPipe(fde,ERR_PIPE_CERRAR_E,
        caminoFifoAgente,ERR_PIPE_BORRAR_E);
        quitarPipe(caminoFifoAgente,ERR_PIPE_BORRAR_E);
        cerrarPipe(fdl,ERR_PIPE_CERRAR_L,
        caminoFifoNodo,ERR_PIPE_BORRAR_L);
        quitarPipe(caminoFifoNodo,ERR_PIPE_BORRAR_L);
        throw std::runtime_error(e);
    }
}

/**
\brief Leer por el pipe de lectura el tamaño del mensaje que se va
a enviar
\details Se lee la cantidad de caracteres que tendrá el mensaje
a enviar. Debe se ser 2 dos caracters. Si no lo es se le
agrega un caracter extra. Se crea y se abre el file descriptor
para facilitar una lectura más limpia de la información
\param[in] El mensaje a enviar, el error de lectura y
el camino del pipe de lectura
\param[out] Ninguno
\pre Que exista el pipe de lectura y el pipe de escritura y que
el fd esté abierto
\remark No aplica
\return No retorna
\exception Cuando la lectura por el pipe de lectura fue errónea o
la escritura por el pipe de escritura fue errónea
\author Juan José Herrera
\date 30-9-20
*/
size_t leerPipeTam(std::string e,
char *caminoFifoAgente,char *caminoFifoNodo){

    std::string arr;
    arr.resize(STD_SIZE);

    if(read(fdl, (void*)arr.data(), STD_SIZE) == -1){
        cerrarPipe(fde,ERR_PIPE_CERRAR_E,
        caminoFifoAgente,ERR_PIPE_BORRAR_E);
        quitarPipe(caminoFifoAgente,ERR_PIPE_BORRAR_E);
        cerrarPipe(fdl,ERR_PIPE_CERRAR_L,
        caminoFifoNodo,ERR_PIPE_BORRAR_L);
        quitarPipe(caminoFifoNodo,ERR_PIPE_BORRAR_L);
        throw std::runtime_error(e);
    }
    int tempSize = atoi(arr.data());
    size_t tamanoMenReal = (size_t)tempSize;

    return tamanoMenReal;
}

/**
\brief Leer por el pipe de lectura el mensaje que se va a enviar
\details Se lee el mensaje enviado por el pipe. Además
 Se crea y se abre el file descriptor
para facilitar una lectura más limpia de la información
\param[in] El mensaje a enviar, el error de lectura y
el camino del pipe de escritura y ep tamañdo dek mensaje
\param[in] El mensaje a enviar,el error de escritura y el camino
del pipe de lectura
\param[out] Ninguno
\pre Que exista el pipe de lectura y el pipe de escritura y que
el fd esté abierto
\remark No aplica
\return No retorna
\exception Cuando la lectura por el pipe de lectura fue errónea o
la escritura por el pipe de escritura fue errónea
\author Juan José Herrera
\date 30-9-20
*/
std::string leerPipeMen(std::string e,
char *caminoFifoAgente,char *caminoFifoNodo,
size_t tamanoMenReal){

    std::string arr;
    arr.resize(tamanoMenReal);//consistencia de string con su tamaño

    if(read(fdl, (void*)(arr.data()),tamanoMenReal) == -1){
        cerrarPipe(fde,ERR_PIPE_CERRAR_E,
        caminoFifoAgente,ERR_PIPE_BORRAR_E);
        quitarPipe(caminoFifoAgente,ERR_PIPE_BORRAR_E);
        cerrarPipe(fdl,ERR_PIPE_CERRAR_L,
        caminoFifoNodo,ERR_PIPE_BORRAR_L);
        quitarPipe(caminoFifoNodo,ERR_PIPE_BORRAR_L);
        throw std::runtime_error(e);
    }
    std::string msjNuevo = std::string(arr.data());

    return msjNuevo;
}

/**
\brief Se comparan dos códigos para asegurarse que son iguales
\details Se procede a comparar 2 códigos para asegurarse que
se pueda tener una lectura más limpia de la información
\param[in] El mensaje a enviar, el error de lectura y el
código a comparar
\param[out] Ninguno
\pre Que exista el pipe de lectura y el pipe de escritura y que
el fd esté abierto
\remark No aplica
\return No retorna
\exception Cuando la lectura por el pipe de lectura fue errónea o
la escritura por el pipe de escritura fue errónea
\author Juan José Herrera
\date 30-9-20
*/
void revisarCodigos(std::string codEnv, std::string codCor,
std::string e){
    
    if(strcmp(codEnv.data(),codCor.data()) != 0){
        throw std::runtime_error(e);
    }
}
