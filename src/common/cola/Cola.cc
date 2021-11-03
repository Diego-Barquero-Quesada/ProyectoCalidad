//Modificado de: https://gist.github.com/ictlyh/f8473ad0cb1008c6b32c41f3dea98ef5#file-producer-consumer-L25
//Autor Original: Yuanhao Luo "ictlyh"
//Fecha de última modificación: 10/11/20

#include "Cola.h"

/**
    @brief Método encargado de sacar un elemento de la cola y
    retornarlo.
    @details Se aplica un lock para que solo un hilo pueda entrar en la
    cola y evitar condiciones de carrera.
    Si la cola está vacía, se esperará de forma pasiva (wait) a que un
    elemento llegue a esta.
    Cuando la cola posea al menos un elemento, este se guardará en una
    variable auto  y seguidamente se elimina de la cola (pop).
    La cola entonces se desbloquea y se retorna la variable que creada.
    @param[in] N/A
    @param[out] N/A
    @pre No hay ningún prerrequisito, aparte de que la cola exista
    @remark Ninguna observación destacable.
    @return El método retorna una variable de tipo auto, la cual se
    pretente que pueda ser cualquier tipo de variable
    (int, string, struct, ..) para así darle versatilidad a la cola.
    @exception Este método no puede lanzar ninguna clase de excepción.
    @author Isaac Herrera B43332 (Pandemic Plan Remastered)
    @date 9/9/20
*/

template <typename T>

T Cola<T>::pop(){
    std::unique_lock<std::mutex> mlock(*m);
    if(this->cola.empty()){
        this->condicion->wait(mlock);
    }
    auto mensaje = this->cola.front();
    this->cola.pop();
    mlock.unlock();
    return mensaje;
}

/**
    @brief Método encargado de ingresar un elemento a la cola.
    @details Se recibe una variable "mensaje" que puede contener
    cualquier tipo de dato. Se ejecuta un mutex antes
    de que el dato sea almacenado en la cola, para evitar así
    condiciones de carrera. Se realiza un push a la cola
    y esta es desbloqueada inmediatamente después. 
    @param[in] N/A
    @param[out] Se recibe la dirección de memoria donde se encuentra la
    variable mensaje que se desea almacenar
    en la cola. La cola no modifica dicha dirección.
    @pre Se espera que es parámetro "mensaje" exista y posea un dato,
    para que este se almacene en la cola.
    @remark Ninguna observación destacable.
    @return Este método no retorna ningún valor (void).
    @exception 
    @author Isaac Herrera B43332 (Pandemic Plan Remastered)
    @date 9/9/20
*/

template <typename T>

void Cola<T>::push(const T& mensaje){
    std::unique_lock<std::mutex> mlock(*m);
    this->cola.push(mensaje);
    mlock.unlock();
    this->condicion->notify_one();
}

/*Estas son plantillas que se deben definir para que la cola, a nivel
de compilación, pueda "prepararse" para almacenar estos tipos de datos,
ya que esta es emplantillada. Si se define con anticipación una
estructura, esta también puede ser usada como plantilla para la cola,
como es el caso de las estructuras "datosNodo" y de las capas.
*/

template class Cola<int>;
template class Cola<std::string>;
template class Cola<const char*>;
template class Cola<struct datosNodo>;

template class Cola<struct Mensaje>;
template class Cola<struct Alcanzabilidad>;
template class Cola<struct Forwarding>;
template class Cola<struct ArbolGenerador>;
template class Cola<struct Latido>;
template class Cola<struct DatosMensaje>;
template class Cola<struct Broadcast>;
template class Cola<struct CapaRed>;
template class Cola<struct DatosForwarding>;
template class Cola<struct DatosForwardingAplicacion>;
template class Cola<struct ForwardingAplicacion>;
template class Cola<struct DatosArbolGenerador>;
template class Cola<struct CapaEnlace>;