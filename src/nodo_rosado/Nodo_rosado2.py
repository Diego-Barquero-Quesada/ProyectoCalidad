import os, sys, time, socket, select, queue
from threading import Thread
import threading

# generate random integer values
from random import seed
from random import randint
# seed random number generator

s1 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s2 = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
hijosAg = []
vivo = 1

nodos = []  ###Variables de dijkstra
matriz = []
caminos = []
forwarding = []

def respuestaTCP(serverQueue):
    paquete = []
    mensajeRecibir=None
    mandarMensaje=0
    try:
        mensajeRecibir = serverQueue.get(block=True, timeout=20)
        
    except queue.Empty:
        mandarMensaje=1
        pass

    if(mensajeRecibir!= None and len(mensajeRecibir) > 5 and len(mensajeRecibir)!=0):
        print("esto es recibir mensaje")
        print("//////////////")
        print(mensajeRecibir)
        print("//////////////")

        for item in mensajeRecibir.split(","):
            print("Esto es item")
            print(item)
            paquete.append(int(item))

        print("Termino de hacer split")
    else:
        mensajeRecibir = "-1,-1,-1,-1,-1"

        for item in mensajeRecibir.split(","):
            paquete.append(int(item))

    return paquete

def solicitudTCP(paquete,clientQueue):

    if(paquete[0:2]=="70"):
        clientQueue.put(paquete)
    else:
        mensajeEnviar = str(paquete[0]) + "," + str(paquete[1]) +","+ \
        str(paquete[2]) + "," + str(paquete[3]) + "," + str(paquete[4])
        clientQueue.put(mensajeEnviar)

        return paquete

def reinicioPaquete():
    paquete = [0,0,0,0]

def TWHResponderPapa(numeroNodo,clientQueue,serverQueue,
reintentos,papaAg,miembroAg):

    seed(1)
    rn = randint(2,999)
    nuevoReintento = 0
    salirWhile = 0
    sn = 0
    exito = 0
    potencialHijo = 0
    while(nuevoReintento < reintentos and  salirWhile != 1):

        paquete = respuestaTCP(serverQueue)
        if (paquete[0]== 1 ):
            sn = paquete [1]
            salirWhile = 1
            exito = 1
        nuevoReintento = nuevoReintento + 1
    sn = sn + 1
    if(exito == 1):
        nuevoReintento = 0
        salirWhile = 0
        exito = 0
        destino = paquete [4]
        potencialHijo=destino
        paquete = [2,sn,rn,destino,numeroNodo]
        while(nuevoReintento < reintentos and  salirWhile != 1):
            solicitudTCP(paquete,clientQueue)
            paqueteR = respuestaTCP(serverQueue)
            if (paqueteR[0]== 4 and paqueteR[1] == sn
            and paqueteR[2] == rn +1 ):

                paquete = [5,sn+1,rn+1,destino,numeroNodo]
                solicitudTCP(paquete,clientQueue)
                salirWhile = 1
                exito = 1
                global hijosAg
                hijosAg.append(potencialHijo)
                enviarBroadcast(0,potencialHijo,clientQueue)
            nuevoReintento = nuevoReintento + 1

def enviarBroadcast(esPapa,miembroArbol,clientQueue):
    if (esPapa==1):
        paquete=[43,43,43,miembroArbol,43]
    else:
        paquete=[44,44,44,miembroArbol,44]

    solicitudTCP(paquete,clientQueue)

def enivarConfirmacionArbol(clientQueue):
    paquete=[45,45,45,45,45]
    solicitudTCP(paquete,clientQueue)

def TWHNoRespondePapa(numeroNodo,clientQueue,serverQueue,reintentos):
    paquete = reinicioPaquete()
    seed(1)
    rn = randint(2,999)
    nuevoReintento = 0
    salirWhile = 0
    sn = 0
    exito = 0
    potencialHijo = 0
    while(nuevoReintento < reintentos and  salirWhile != 1):
        paquete = respuestaTCP(serverQueue)
        if (paquete[0]== 1 and paquete[1] != 1 and paquete[1] != 0
        and paquete[2] != 1 and paquete[2] != 0):
            sn = paquete [1]
            salirWhile = 1
            exito = 1
        nuevoReintento = nuevoReintento + 1
    sn = sn + 1
    if(exito == 1):
        nuevoReintento = 0
        salirWhile = 0
        exito = 0
        destino = paquete [4]
        paquete = [3,sn,rn,destino,numeroNodo]
        while(nuevoReintento < reintentos and  salirWhile != 1):

            solicitudTCP(paquete,clientQueue)
            paqueteR = respuestaTCP(serverQueue)
            if (paqueteR[0]== 4 and paqueteR[1] == sn
            and paqueteR[2] == rn +1 ):
                paquete = [5,sn+1,rn+1,destino,numeroNodo]
                salirWhile = 1
                exito = 1

            nuevoReintento = nuevoReintento + 1


def TWHSolicitudPapa(numeroNodo,potencialPapa,clientQueue,
serverQueue,reintentos):

    seed(1)
    sn = randint(2,999)
    paquete = [1,sn,0,potencialPapa,numeroNodo]
    nuevoReintento = 0
    salirWhile = 0
    exito = 0
    viejoPaqueteOp = 0
    papaAg = -1
    miembroAg = 0
    while(nuevoReintento < reintentos and salirWhile != 1):
        solicitudTCP(paquete,clientQueue)
        paqueteR = respuestaTCP(serverQueue)
        if (paqueteR[0]== 2 and paqueteR[1] == sn + 1):
            salirWhile = 1
            exito = 1
            viejoPaqueteOp = paqueteR[0]
        else:
            if(paqueteR[0]== 3 and paqueteR[1] == sn + 1):
                salirWhile = 1
                exito = 1
                viejoPaqueteOp = paqueteR[0]

        nuevoReintento = nuevoReintento + 1
    sn = paqueteR[1]
    rn = paqueteR[2] + 1
    if(exito == 1):
        nuevoReintento = 0
        salirWhile = 0
        exito = 0
        paquete = [4,sn,rn,potencialPapa,numeroNodo]
        while(nuevoReintento < reintentos and salirWhile != 1):
            solicitudTCP(paquete,clientQueue)
            paqueteR = respuestaTCP(serverQueue)

            if (paqueteR[0]== 5 and paqueteR[1] == sn + 1):
                salirWhile = 1
                exito = 1
                if(viejoPaqueteOp == 2 ):
                    papaAg = potencialPapa
                    enviarBroadcast(1,papaAg,clientQueue)
                    miembroAg = 1
            nuevoReintento = nuevoReintento + 1

    vectorRespuestaP = [exito,papaAg,miembroAg]
    return vectorRespuestaP


def reconfiguracionPapa(serverQueue,clientQueue,reintentos):
    nuevoReintento = 0
    salirWhile = 0
    exito = 0
    potencialPapa = -1
    vectorRespuestaP = [exito,potencialPapa]
    paqueteL = [-101,-101,-101,-101,-101]
    while(nuevoReintento < reintentos and salirWhile != 1):
        solicitudTCP(paqueteL,clientQueue)

        paquete = respuestaTCP(serverQueue)

        if (paquete[0] ==  0 and paquete[1] == 1 and
        paquete[2] == 1 ):#[0,1,1,1,3]
            salirWhile = 1
            potencialPapa = paquete[4]
            paquete = [1,1,1,1,1]
            solicitudTCP(paquete,clientQueue)
            exito = 1
        nuevoReintento = nuevoReintento + 1
    vectorRespuestaP = [exito,potencialPapa]

    if(salirWhile == 0):
        paquete = [-20,-20,-20,-20,-20]
        solicitudTCP(paquete,clientQueue)
        time.sleep(0.3)
    return vectorRespuestaP

def obtenerReintentos(serverQueue,clientQueue):
    reintentos = 7
    nuevoReintento = 0
    salirWhile = 0
    exito = 0
    paqueteL = [-101,-101,-101,-101,-101]

    while(nuevoReintento < reintentos and salirWhile != 1):

        solicitudTCP(paqueteL,clientQueue)
        print("Desperte al hilo agente rosado")
        print("wwwwwwwwwwwwwwwwwwwwwwwwwwwwww")
        print("wwwwwwwwwwwwwwwwwwwwwwwwwwwwww")
        paquete = respuestaTCP(serverQueue)  #[0,0,0,1]
        print("Respuesta hilo rosado")
        print(paquete[0])
        print(paquete[1])
        print(paquete[2])
        print("wwwwwwwwwwwwwwwwwwwwwwwwwwwwww11")
        print("wwwwwwwwwwwwwwwwwwwwwwwwwwwwww11")

        if(paquete[0] ==  0 and paquete[1] == 0 and
            paquete[2] == 0 ):

            reintentos = paquete[4]
            salirWhile = 1
            paquete = [1,1,1,1,1]
            solicitudTCP(paquete,clientQueue)
            print("Respuesta hilo rosado22")
            print("wwwwwwwwwwwwwwwwwwwwwwwwwwwwww22")
            print("wwwwwwwwwwwwwwwwwwwwwwwwwwwwww22")
            exito = 1
        nuevoReintento = nuevoReintento + 1

    if(salirWhile == 0):
        paquete = [-20,-20,-20,-20,-20]
        solicitudTCP(paquete,clientQueue)
        time.sleep(0.3)
    vectorRespuesta = [exito,reintentos]
    return vectorRespuesta


def buscarPapa(numeroNodo,serverQueue,clientQueue,reintentos):
    exito = 0
    vectorRespuestaP = reconfiguracionPapa(serverQueue,
    clientQueue,reintentos)

    potencialPapa=vectorRespuestaP[1]
    exito = vectorRespuestaP[0]
    if(exito == 1):
        vectorRespuestaD = TWHSolicitudPapa(numeroNodo,potencialPapa,
        clientQueue,serverQueue,reintentos)

        vectorRespuestaP = vectorRespuestaD

    return vectorRespuestaP


def revisarNodosMuertos(numeroNodo,clientQueue,serverQueue,reintentos):
    muertos = 0
    reintentos = 7
    nuevoReintento = 0
    salirWhile = 0
    exito = 0
    while(nuevoReintento < reintentos or salirWhile != 1):
        paquete = respuestaTCP(serverQueue)
        if (paquete[0] >  -1 and paquete[1] == -1 and
        paquete[2] == -1 ):
            nodoMuerto = paquete[3]
            salirWhile = 1
            if(nodoMuerto == papaAg):
                muertos ==1
                for x in hijosAg:
                    paquete = [-1.-1.-1,x,numeroNodo]
                    solicitudTCP(paquete,clientQueue)
            else:
                i = 0
                while (i < len(hijosAg)):
                    if(hijosAg[i]==nodoMuerto):
                        hijosAg.remove(i)

        nuevoReintento = nuevoReintento + 1


def responderSolicitudesPapa(numeroNodo,clientQueue,serverQueue,
reintentos,papaAg,miembroAg):

    muertos = 0
    global vivo
    while(vivo == 1):
        print("Respondiendo solicitudes")
        enivarConfirmacionArbol(clientQueue)

        TWHResponderPapa(numeroNodo,clientQueue,serverQueue,
        reintentos,papaAg,miembroAg)


def generarMatriz(data,clientQueue):
    datos = data.split(sep=',')
    indice = 0

    while indice < len(datos):
      nuevoNodo = True
      j = 0
      while nuevoNodo and j < len(nodos):
        if nodos[j] == int(datos[indice]):
          nuevoNodo = False
        j += 1
      if nuevoNodo:
        nodos.append(int(datos[indice]))

      indice += 2

    while len(matriz) < len(nodos):
      matriz.append([])
      forwarding.append([])

    for i in range(len(matriz)):
      while len(matriz[i]) < len(matriz)-1:
        matriz[i].append(1000)
        forwarding[i].append(1000)

    nuevo = 0
    for i in range(len(nodos)):
      if int(datos[0]) == nodos[i]:
        nuevo = i

    indice = 2
    for i in range(int(datos[1])):
      for j in range(len(nodos)):
        if int(datos[indice]) == nodos[j]:
          if j == 0:
            matriz[0][nuevo-1] = int(datos[indice+1])
          else:
            matriz[nuevo][j-1] = int(datos[indice+1])
      indice += 2

    for i in range(len(nodos)-1):
      caminos.append([0, 1000])


    dijkstra(matriz, caminos, forwarding,clientQueue)


def dijkstra(pesos, caminos, forwarding,clientQueue):
    i = 0
    j = 0
    acumulado = []
    indiceI = []
    indiceJ = []
    padre = 0
   
    while i < len(matriz):
      if len(indiceI):
        i = indiceI[-1]
        j = indiceJ[-1]
        indiceI.pop(-1)
        indiceJ.pop(-1)
        acumulado.pop(-1)
        padre = 0
      while j < len(matriz)-1:
        if pesos[i][j]:
          acumuladoActual = 0
          if not padre:
              padre = j + 1
          if i <= j:

            acumulado.append(pesos[i][j])
            for n in range(len(acumulado)):
              acumuladoActual += acumulado[n]
            if acumuladoActual < forwarding[i][j]:
              forwarding[i][j] = acumuladoActual
              if caminos[j][1] > acumuladoActual:
                caminos[j][0] = padre
                caminos[j][1] = acumuladoActual
            indiceI.append(i)
            indiceJ.append(j+1)
            i = j + 1
            j = 0
          else:
            for n in range(len(acumulado)):
              acumuladoActual += acumulado[n]
            acumuladoActual += pesos[i][j]
            if acumuladoActual < forwarding[i][j]:
              forwarding[i][j] = acumuladoActual
              if caminos[j][1] > acumuladoActual:
                caminos[j][0] = padre
                caminos[j][1] = acumuladoActual
            j += 1
        else:
          forwarding[i][j] = 1000
          j += 1
      if not len(indiceI):
        i += 1

    paquete="70"
    for i in range(len(matriz)-1):
        paquete += "," + str(nodos[caminos[i][0]]) + "-" + \
        str(nodos[caminos[i][1]]) + "-" + str(nodos[i+1])

    solicitudTCP(paquete,clientQueue)

def serverTcp(serverQueue,clientQueue):
    bandera = 1
    while(bandera == 1):

        data = s1.recv(1024)
        data=data.decode("utf-8")


        prueba=data[0:2]
        if(prueba == "90"):
            data=data[3:]
            generarMatriz(data,clientQueue)
        elif (prueba != "90"):

            serverQueue.put(data)
            if(data == "-100,-100,-100,-100,-100"):
                global vivo
                vivo = 1


def clientTcp(clientQueue):

    global vivo
    while vivo == 1:
        data=None
        try:
            data = clientQueue.get(block = True,timeout=30)
        except queue.Empty:
            pass
        if data != None:
            s2.send(data.encode())


def arGe(serverQueue,clientQueue):

    numeroNodo = -1
    numeroNodo = int(sys.argv[1])
    miembroAg = 0
    m = str(numeroNodo) + "numero nodo Python"
    papaAg = -1

    reintentos = -1
    bandera = True
    global vivo
    while(bandera == True and vivo == 1):
        vectorRespuesta = obtenerReintentos(serverQueue,clientQueue)
        exito = vectorRespuesta[0]
        reintentos = vectorRespuesta[1]
        if(exito ==1 ):
            bandera = False
    while(vivo == 1):
        #numero de nodo se puede pasar por parametro
        papaAg = -1
        global hijosAg
        hijosAg = []
        numeroAg = 0
        if(numeroNodo == 1):
            miembroAg =1

        if(miembroAg == 0):
            vectorRespuestaP = buscarPapa(numeroNodo,serverQueue,
            clientQueue,reintentos)

            if(vectorRespuestaP[0] == 1):
                papaAg = vectorRespuestaP[1]
                miembroAg = vectorRespuestaP[1]

                responderSolicitudesPapa(numeroNodo,clientQueue,
                serverQueue,reintentos,papaAg,miembroAg)
        else:
            responderSolicitudesPapa(numeroNodo,clientQueue,
            serverQueue,reintentos,papaAg,miembroAg)

if __name__=='__main__':

    time.sleep(5)

    HOST,PORT = 'localhost', int(sys.argv[2])
    s1.connect((HOST, PORT))

    HOST2,PORT2 = 'localhost', int (sys.argv[3])

    s2.connect((HOST2, PORT2))

    clientQueue = queue.Queue()
    serverQueue = queue.Queue()
    serverSolicitudes = queue.Queue()

    hiloCliente = Thread(target = clientTcp, args = (clientQueue,))
 
    hiloCliente.start()

    hiloServer=Thread(target=serverTcp,args=(serverQueue,clientQueue,))

    hiloServer.start()

    aG = Thread(target = arGe, args = (serverQueue,clientQueue,))

    aG.start()