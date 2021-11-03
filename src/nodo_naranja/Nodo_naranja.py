import os
import sys
import csv
import time
import constantes
from csv import reader

fde = 0
fdl = 0

"""
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
"""
def borrarPipe(caminoPipe,mensajeError):
    try:
        os.remove(caminoPipe)
    except OSError:
        print(mensajeError)

"""
\brief Cerrar el file descriptor del pipe seleccionado
\details Se cierra el file desciptro del pipe seleccionado
para evitar envío de datos sucios y evitat dejarlo
abierto sin razón
\param[in] Las ruta del pipe a cerrrar, el file descriptor,
el mensaje de error en caso de borrar le pipe y el errpr en
caso de cerrar el file descriptor
\param[out] El file descriptor ya que se cierra
\pre Que exista el pipe de lectura y el pipe de escritura y que
el fd esté abierto
\remark No aplica
\return No retorna
\exception Cuando no se pudo borrar el pipe
\author Juan José Herrera
\date 30-9-20
"""
def cerrarPipe(fifo,mensajeError,caminoFifo,
mensajeErrorBorrar):
    try:
        fifo.close()
    except OSError:
        borrarPipe(caminoFifo,
        mensajeErrorBorrar)
        print(mensajeError)

"""
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
"""
def escribirPipe(mensaje,mensajeError,
caminoFifoAgente,caminoFifoNodo):

    escribirPipeTam(mensaje,mensajeError,
    caminoFifoAgente,caminoFifoNodo)
    tamano = leerPipeTam(
    mensajeError,caminoFifoAgente,caminoFifoNodo)
    escribirPipeMen(mensaje,mensajeError,
    caminoFifoAgente,caminoFifoNodo)
    msj = leerPipeMen(
    mensajeError,caminoFifoAgente,caminoFifoNodo,tamano)

"""
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
la escritura por el pipe de escritura fue errónea
\author Juan José Herrera
\date 30-9-20
"""
def leerPipe(mensajeError,
caminoFifoAgente,caminoFifoNodo):

    tamano = leerPipeTam(
    mensajeError,caminoFifoAgente,caminoFifoNodo)
    escribirPipeTam(constantes.CODIGO_CONFIRMACION,
    mensajeError,
    caminoFifoAgente,caminoFifoNodo)
    mensaje = leerPipeMen(
    mensajeError,caminoFifoAgente,caminoFifoNodo,tamano)
    escribirPipeTam(constantes.CODIGO_CONFIRMACION,
    mensajeError,
    caminoFifoAgente,caminoFifoNodo)
    return mensaje

"""
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
"""
def leerPipeTam(mensajeError,caminoFifoAgente,caminoFifoNodo):

    try:
        tamano = str(fdl.read(constantes.STD_SIZE))

        tamanoInt = int(tamano)

    except OSError:
        cerrarPipe(fdl,constantes.ERR_PIPE_CERRAR_L,
        caminoFifoAgente,constantes.ERR_PIPE_BORRAR_L)
        borrarPipe(caminoFifoAgente,
        constantes.ERR_PIPE_BORRAR_L)
        cerrarPipe(fde,constantes.ERR_PIPE_CERRAR_E,
        caminoFifoNodo,constantes.ERR_PIPE_BORRAR_E)
        borrarPipe(caminoFifoNodo,
        constantes.ERR_PIPE_BORRAR_E)
        print(mensajeError)

    return tamanoInt

"""
\brief Leer por el pipe de lectura el mensaje que se va a enviar
\details Se lee el mensaje enviado por el pipe. Además
 Se crea y se abre el file descriptor
para facilitar una lectura más limpia de la información
\param[in] El mensaje a enviar, el error de lectura y
el camino del pipe de escritura y ep tamaño de mensaje
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
"""
def leerPipeMen(mensajeError,caminoFifoAgente,caminoFifoNodo,tamano):

    try:
        mensaje= str(fdl.read(tamano))


    except OSError:
        cerrarPipe(fdl,constantes.ERR_PIPE_CERRAR_L,
        caminoFifoAgente,constantes.ERR_PIPE_BORRAR_L)
        borrarPipe(caminoFifoAgente,
        constantes.ERR_PIPE_BORRAR_L)
        cerrarPipe(fde,constantes.ERR_PIPE_CERRAR_E,
        caminoFifoNodo,constantes.ERR_PIPE_BORRAR_E)
        borrarPipe(caminoFifoNodo,
        constantes.ERR_PIPE_BORRAR_E)
        print(mensajeError)

    return mensaje

"""
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
"""
def escribirPipeTam(mensaje,mensajeError,
     caminoFifoAgente,caminoFifoNodo):

    try:
        nuevoMensaje = str(len(mensaje))

        if(len(mensaje)<constantes.MIN_BYTES_VER):

            nuevoMensaje = constantes.COMP_2_BYTES + nuevoMensaje

        fde.write(nuevoMensaje.encode("ASCII","ignore"))

        fde.flush()
    except OSError:
        cerrarPipe(fdl,constantes.ERR_PIPE_CERRAR_L,
        caminoFifoAgente,constantes.ERR_PIPE_BORRAR_L)
        borrarPipe(caminoFifoAgente,
        constantes.ERR_PIPE_BORRAR_L)
        cerrarPipe(fde,constantes.ERR_PIPE_CERRAR_E,
        caminoFifoNodo,constantes.ERR_PIPE_BORRAR_E)
        borrarPipe(caminoFifoNodo,
        constantes.ERR_PIPE_BORRAR_E)
        print(mensajeError)

"""
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
la escritura por el pipe de escritura fue errónea
\author Juan José Herrera
\date 30-9-20
"""
def escribirPipeMen(mensaje,mensajeError,
     caminoFifoAgente,caminoFifoNodo):

    try:
        fde.write(mensaje.encode("ASCII","ignore"))
        fde.flush()
    except OSError:
        cerrarPipe(fdl,constantes.ERR_PIPE_CERRAR_L,
        caminoFifoAgente,constantes.ERR_PIPE_BORRAR_L)
        borrarPipe(caminoFifoAgente,
        constantes.ERR_PIPE_BORRAR_L)
        cerrarPipe(fde,constantes.ERR_PIPE_CERRAR_E,
        caminoFifoNodo,constantes.ERR_PIPE_BORRAR_E)
        borrarPipe(caminoFifoNodo,
        constantes.ERR_PIPE_BORRAR_E)
        print(mensajeError)

"""
\brief Se comparan dos códigos para asegurarse que son iguales
\details Se procede a comparar 2 códigos para asegurarse que
se pueda tener una lectura más limpia de la información
\param[in] El mensaje a enviar, el error de lectura y el código a comparar
\param[out] Ninguno
\pre Que exista el pipe de lectura y el pipe de escritura y que
el fd esté abierto
\remark No aplica
\return No retorna
\exception Cuando la lectura por el pipe de lectura fue errónea o
la escritura por el pipe de escritura fue errónea
\author Juan José Herrera
\date 30-9-20
"""
def revisarCodigos(codEnv,codCor,mensajeError):
    if(codEnv != codCor):
        raise Exception(mensajeError)

"""
\brief Enviar información leída de un archivo para
que se pueda introducir en un vector de strings
\details Por medio de los pipes, se envían mensajes del nodo
naranja para que puedan ser almacendos en un vector de strings
\param[in] El código para parar de recibir mensajes, las
rutas de los archivos de los pipes y el código de error
en caso que la lectura
por los pipes fuera errónea
\param[out] No aplica
\pre Que exista el pipe de escritura
\remark No aplica
\return El vector de mensajes, para que pueda ser manipulado
por el agente naranja
\exception Cuando la lectura por el pipe de lectura fue errónea
\author Juan José Herrera
\date 30-9-20
"""
def enviarInfoArchivo(caminoFifoAgente,caminoFifoNodo,
caminoNodo,mensajeError):
    with open(caminoNodo
    ,constantes.PERMISO_LEER) as obj_esc:
        csv_lector = reader(obj_esc)
        for linea in csv_lector:
            mensaje = ','.join(linea)
            escribirPipe(mensaje,mensajeError,
            caminoFifoAgente,caminoFifoNodo)

"""
\brief Recibir el aviso para recibir la información del nodo verde y
enviar la información del nodo verde
\details Se lee el código del agente naranja para avisar que se va a
recibir la información del nodo verde y luego se envía la
información del nodo verde
\param[in] las rutas de los pipes y la ruta de la información del
nodo verde
\param[out] No aplica
\pre Que exista el pipe de lectura y el pipe de escritura
\remark No aplica
\return No retorna nada
\exception Cuando la lectura por el pipe de lectura fue errónea o
la escritura por el pipe de escritura fue errónea
\author Juan José Herrera
\date 30-9-20
"""
def comunicacionInfoVerde(caminoFifoAgente,caminoFifoNodo,
caminoNodoVerde):
    verAviso=leerPipe("No se recibió aviso para envio de info ver",
    caminoFifoAgente,caminoFifoNodo)

    revisarCodigos(verAviso,constantes.CODIGO_AVISO_ENV_VER ,
    constantes.DIFF_AVISO_ENV_VER)

    enviarInfoArchivo(caminoFifoAgente,caminoFifoNodo,
    caminoNodoVerde,"No se envió info ver")

    escribirPipe(constantes.CODIGO_AVISO_REC_VER,
    "No se envió aviso para recibo de info ver",
    caminoFifoAgente,caminoFifoNodo)

"""
\brief Recibir el aviso para recibir la información de los vecinos y
enviar la información de los vecinos
\details Se lee el código del agente naranja para avisar que se va a
recibir la información del nodo verde y luego se envía la
información de los vecinos
\param[in] las rutas de los pipes y la ruta de la información de los vecinos
\param[out] No aplica
\pre Que exista el pipe de lectura y el pipe de escritura
\remark No aplica
\return No retorna nada
\exception Cuando la lectura por el pipe de lectura fue errónea o
la escritura por el pipe de escritura fue errónea
\author Juan José Herrera
\date 30-9-20
"""
def comunicacionVecinos(caminoFifoAgente,caminoFifoNodo,
caminoVecinos):

    vecinosAviso=leerPipe("No se envio aviso para envio de vecinos",
    caminoFifoAgente,caminoFifoNodo)

    revisarCodigos(vecinosAviso,constantes.CODIGO_AVISO_ENV_VEC ,
    constantes.DIFF_AVISO_ENV_VEC)
    enviarInfoArchivo(caminoFifoAgente,caminoFifoNodo,
    caminoVecinos,"No se envió info vecinos")
    escribirPipe(constantes.CODIGO_AVISO_REC_VEC,
    "No se envio aviso para recibo de vecinos",
    caminoFifoAgente,caminoFifoNodo)

"""
\brief Comucicarse con en nodo naranja para recibir su pid
\details Se recibe el del aviso de emvío de pid, luego se recibe
el pid al nodo naranja y por último se envía el mensaje de
finalización de este sector
\param[in] Las rutas de los archivos de pipes
\param[out] No aplica
\pre Que exista el pipe de lectura y el pipe de escritura
\remark No aplica
\return el pid
\exception Cuando la lectura por el pipe de lectura fue errónea o
la escritura por el pipe de escritura fue errónea
\author Juan José Herrera
\date 30-9-20
"""
def comunicacionPid(caminoFifoAgente,caminoFifoNodo):

    prePid = leerPipe("No se recibió aviso envio de pid",
    caminoFifoAgente,caminoFifoNodo)

    revisarCodigos(prePid,constantes.PREENVIO_PID,
    constantes.DIFF_PREENVIO_PID)
    pid = leerPipe("No se recibió el pid",
    caminoFifoAgente,caminoFifoNodo)

    escribirPipe(constantes.CODIGO_FINALIZACION_PID,
    "No se envió correctamente el pid",
    caminoFifoAgente,caminoFifoNodo)

    return pid

if __name__ == "__main__":
    caminoFifoAgente=sys.argv[1] #caminoFifoAgente
    caminoFifoNodo=sys.argv[2] #caminoFifoNodo
    caminoNodoVerde = sys.argv[3] #archivoInfoNodVer
    caminoVecinos = sys.argv[4] #archivovecinos
    try:
        fde = open(caminoFifoNodo,
        constantes.PERMISO_ESCRIBIR)

    except OSError:
        borrarPipe(caminoFifoNodo,
        constantes.ERR_PIPE_BORRAR_E)
        print(constantes.ERR_PIPE_ABRIR_E)

    try:
        fdl = open(caminoFifoAgente,
        constantes.PERMISO_LEER)

    except OSError:
        borrarPipe(caminoPipe,
        constantes.ERR_PIPE_BORRAR_L)
        print(constantes.ERR_PIPE_ABRIR_L)

    pid = comunicacionPid(caminoFifoAgente,caminoFifoNodo)
    comunicacionInfoVerde(caminoFifoAgente,caminoFifoNodo,
    caminoNodoVerde)
    comunicacionVecinos(caminoFifoAgente,caminoFifoNodo,
    caminoVecinos)
    cerrarPipe(fdl,constantes.ERR_PIPE_CERRAR_L,
    caminoFifoAgente,constantes.ERR_PIPE_BORRAR_L)
    cerrarPipe(fde,constantes.ERR_PIPE_CERRAR_E,
    caminoFifoNodo,constantes.ERR_PIPE_BORRAR_E)
