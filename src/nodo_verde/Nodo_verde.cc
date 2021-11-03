#include "Nodo_verde.h"

std::vector<std::string> datosNodos;

void depositarDatos(std::vector<std::string>nuevosDatos){
   datosNodos = nuevosDatos;
}

/**
    @brief Rutina principal para el funcionamiento de la red.
    @details Esta rutina se encarga de la creación de los 3 agentes
    (hilos naranja, azul y verde) para que cada uno ejecute
    su rutina. Se reciben los datos del agente naranja y este finaliza
    (join), mientras que los agentes azul y verde deben
    mantenerse en ejecución durante toda la rutina.
    @param[in] argc Cantidad de parámetros al momento de la ejecución.
    Se espera que solo se reciban 2.
    @param[out] argv Lista de parámetros. El primero sería el programa
    en sí, mientras que el segundo es la posible entrada
    "--help", el cual da una pequeña descripción de ayuda sobre el
    programa.
    @pre En teoría, no habría ningún prerrequisito para la ejecución de
    esta rutina.
    @remark Ninguna observación destacable, al invocar esta rutina se
    invocan todas las demás.
    @return Si la rutina finaliza correctamente, debería retornar un 0,
    representando una salida exitosa.
    En caso de un error, es probable que el la rutina finalice de forma
    brusca.
    @exception La rutina podría terminar abruptamente en caso de un
    error.
    @author Isaac Herrera B43332 (Pandemic Plan Remastered)
    @date 13/9/20
*/

int main(int argc,char *argv[]){

    if(argc == 2){
        if(strcmp(argv[1],"--help") == 0){
            std::cout << "Argumentos: nombrePipeAgenteNaranja" <<
            " (a elegir), nombrePipeNodoNaranja (a elegir), " <<
            "ruta/datosNodosNodo.csv, ruta/vecinos.csv, " <<
            "nombrePipeAgenteAzul (a elegir), nombrePipeNodoAazul" <<
            "(a elegir), puerto del webSocket (a elegir), " <<
            "puertos #1 y #2 para el árbol (a elegir), " <<
			"IP del nodo" << std::endl;

            return 0;
        } else {
            std::cout << "Commando inválido, ejecute el programa " <<
            "con \"--help\" para recibir ayuda" << std::endl;
        }
    } else {
        if(argc != 11){
            std::cout << "Hacen falta argumentos, ejecute el " <<
            "programa con \"--help\" para recibir ayuda" << std::endl;
        }
    }

    char* rutaFifoAgenteNaranja = argv[1];
    char* rutaFifoNodoNaranja = argv[2];
    char* rutaNodoVerde = argv[3];
    char* rutaVecinos = argv[4];

    std::thread agenteNaranja(trabajoAgenteNaranja,
    rutaFifoAgenteNaranja,rutaFifoNodoNaranja,rutaNodoVerde,
    rutaVecinos);

    agenteNaranja.join();

    std::vector<int> nodosIDS;

    size_t i;

    std::vector<datosNodo> tablaVecinos;
    datosNodo datosPropios;
    tablaVecinos.push_back(datosPropios);
    tablaVecinos[0].ID = 0;

    char *linea = (char*)datosNodos[0].c_str();

    char* p;

    p = strtok(linea,",");

    tablaVecinos[0].ID = atoi(p);
    p = strtok(NULL, ",");
    tablaVecinos[0].puerto = atoi(p);

    for(i=1; i<datosNodos.size(); i++){
        datosNodo vecino;
        tablaVecinos.push_back(vecino);

        linea = (char*)datosNodos[i].c_str();
        p = strtok(linea,",");
        tablaVecinos[i].ID = atoi(p);

        p = strtok(NULL,",");
        tablaVecinos[i].IP = p;

        p = strtok(NULL,",");
        tablaVecinos[i].puerto = atoi(p);
    }

    int posicion,datos;
    for(i=0; i<datosNodos.size(); i++){
        posicion=datosNodos[i].find(",");
        datos= std::stoi(datosNodos[i].substr(0,posicion));
        nodosIDS.push_back(datos);
    }

    std::vector<Cola<struct CapaEnlace>> colasDeMensajes;
    for(size_t i = 0; i < datosNodos.size(); ++i){
        Cola<struct CapaEnlace> cola;
        colasDeMensajes.push_back(cola);
    }

    //Colas usadas para comunicación entre capas
    Cola<struct ForwardingAplicacion> colaAzul;
	Cola<struct ArbolGenerador> colaRosada;
	Cola<struct DatosForwardingAplicacion> colaDespachadorAzul;
	Cola<struct DatosArbolGenerador> colaDespachadorRosado;
	Cola<struct CapaRed> colaDespachadorVerde;
    Cola<std::string> despachadorMiembros;
    Cola<std::string> colaAlcanzabilidad;
    Cola<std::string> colaTablaForwarding;
    Cola<std::string>colaEnviarAlcanzabilidad;

    std::thread agenteAzul(hiloAzul, &nodosIDS, &colaAzul,
    argv[5], argv[6], argv[7], &colaDespachadorAzul);

    std::thread agenteVerde(hiloVerde, &tablaVecinos,
    &colasDeMensajes, &colaDespachadorVerde);

    std::thread agenteRed(capaRed, &colaAzul, &colaRosada,
    &colasDeMensajes, &colaDespachadorAzul, &colaDespachadorRosado,
    &colaDespachadorVerde, &nodosIDS, &despachadorMiembros,
    &colaAlcanzabilidad, &tablaVecinos, argv[10], &colaTablaForwarding,
    &colaEnviarAlcanzabilidad);

    std::thread agenteRosado(trabajoAgenteRosado, &tablaVecinos,
    &colaRosada, &colaDespachadorRosado, argv[8], argv[9],
    &despachadorMiembros,&colaAlcanzabilidad,&colaTablaForwarding,
    &colaEnviarAlcanzabilidad);

    agenteAzul.join();
    printf("Agente azul finalizado\n");
    agenteVerde.join();
    printf("Agente verde finalizado\n");
    agenteRed.join();
    printf("Agente red finalizado\n");
    agenteRosado.join();
    printf("Agente rosado finalizado\n");
    printf("FINALIZACIÓN DEL PROGRAMA\n");

    return 0;
}
