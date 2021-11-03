let webSocketServer = null;
const TAM_MAX_ARCHIVO = 25000000; //bytes
const caracteres = 'abcdefghijklmnopqrstuvwxyz0123456789 ';
//Caracteres para crear mensajes aleatorios

// WebSocket ----------------------------------------------------------

/**
 * @brief Crea un elemento 'p' y lo coloca en la bitácora de mensajes.
 * @details Se crea un elemento párrafo y se le asignan las clases
 * 'mensaje' y 'clase'(parámetro), así como su texto respectivo, para
 * luego ser colocado como hijo en el div 'mensajes'.
 * @param[in] texto Mensaje textual que se coloca en 'innerText'.
 * @param[in] clase Se añade como segunda clase del mensaje creado.
 * @pre Haber recibido/enviado un mensaje, o que un error haya ocurrido
 * son sucesos que harán que esta subrutina sea invocada.
 * @remark La página web verá actualizada su bitácora
 * @author Jeisson Hidaldo, Isaac Herrera B43332
 */
function agregarMensaje(texto, clase) {
  const mensaje = document.createElement('p');
  mensaje.classList.add('mensaje');
  mensaje.classList.add(clase);
  if (typeof texto === 'string') {
    mensaje.innerText = texto;
  } else if (typeof texto === 'object') {
    mensaje.appendChild(texto);
  }

  const mensajes = document.getElementById('mensajes');
  mensajes.appendChild(mensaje);
}

/**
 * @brief Añade link del archivo recibido a la bitácora
 * @details Se añade un mensaje de tipo "link" a la bitácora de
 * mensajes, mediante el cual el usuario puede accedar directamente
 * a él y abrir algunos archivos directamente desde el navegador
 * (como imágenes).
 * @param[in] path: Contiene el link del archivo
 * @pre Que un archivo llegue por el web socket
 * @author Jeisson Hidalgo, Isaac Herrera B43332
 */
function agregarEnlace(path){
  const link = document.createElement('a');
  link.setAttribute('href', path);
  link.setAttribute('target', '_blank');
  link.innerText = path.substr('/archivosRecibidos/'.length);
  //Agrega el link sin el "/files/"
  agregarMensaje(link, 'link');
}

/**
 * @brief Agrega los vecinos inmediatos del nodo a la página web.
 * @details Se recibe un mensaje por parte del web socket que contiene
 * la lista de vecinos inmediatos del nodo, para así incluirlos en la
 * página 
 * @param[in] payload: El mensaje que contiene los vecinos inmediatos
 * @pre Que el websocket haya enviado el mensaje respectivo
 * @remark La pagina web agrega campos tipo "radio" referentes
 * a sus vecinos
 * @author Isaac Herrera B43332
 */
function agregarVecinos(payload){
  vecinos = payload.substring(8);
  listaVecinos = vecinos.split(/\W+/);
  const campo_vecinos = document.getElementById('lista_vecinos');

  for(i=0; i<listaVecinos.length; i++){
    const vecino = document.createElement('input');
    vecino.type = "radio";
    vecino.name = "vecinos";
    vecino.id = "v" + listaVecinos[i];
    vecino.value = listaVecinos[i];

    const etiqueta = document.createElement('label');
    etiqueta.for = "v" + listaVecinos[i];
    etiqueta.innerText = "Vecino " + listaVecinos[i];
    campo_vecinos.appendChild(vecino);
    campo_vecinos.appendChild(etiqueta);
    
    salto_linea = document.createElement("br");
    campo_vecinos.appendChild(salto_linea);
  }

  const broadcast = document.createElement('input');
  broadcast.type = "radio";
  broadcast.name = "vecinos";
  broadcast.id = "v0";
  broadcast.value = "0";

  const etiquetaB = document.createElement('label');
  etiquetaB.for = "v0";
  etiquetaB.innerText = "Broadcast";

  campo_vecinos.appendChild(broadcast);
  campo_vecinos.appendChild(etiquetaB);
  salto_linea = document.createElement("br");
  campo_vecinos.appendChild(salto_linea);

  const aleatorio = document.createElement('input');
  aleatorio.type = "radio";
  aleatorio.name = "vecinos";
  aleatorio.id = "vA";
  aleatorio.value = "-1";

  const etiquetaA = document.createElement('label');
  etiquetaA.for = "vA";
  etiquetaA.innerText = "Aleatorio";

  campo_vecinos.appendChild(aleatorio);
  campo_vecinos.appendChild(etiquetaA);
  salto_linea = document.createElement("br");
  campo_vecinos.appendChild(salto_linea);
}

/**
 * @brief Cambia el estado de la página a 'Conectado'.
 * @details Obtiene un elemento 'p' mediante su id para así cambiar su
 * clase a 'ok' y su texto a 'Conectado', dando a entender que
 * la página mantiene una conexión con el web socket.
 * @pre Que el socket se haya abierto correctamente.
 */
function webSocketOpened() {
  const status = document.getElementById('estado');
  status.className = 'ok';
  status.innerText = 'Conectado';
}

/**
 * @brief Cambia el estado de la página a 'Desconectado'.
 * @details Obtiene un elemento 'p' mediante su id para así cambiar su
 * clase a 'error' y su texto a 'Desconectado', dando a entender que
 * la página ya no posee una conexión con el web socket.
 * @param[in] evento Posible razón de la desconexión, usada para
 * agregar un mensaje a la bitácora.
 * @pre Que el web socket se haya cerrado.
 */
function webSocketClosed(evento) {
  const status = document.getElementById('estado');
  status.className = 'error';
  status.innerText = 'Desconectado';

  agregarMensaje(`Desconectado del servidor ${evento.reason}`,'error');
  // agregarMensaje('Se intentará reconectar en 10 segundos');
  // setTimeout(function(){ setupWebSocket();}, 10000);
}

/**
 * @brief Verifica mensajes recibidos por el web socket y realiza una
 * acción pertinente.
 * @details Se espera a la llegada de un mensaje entrante (evento) para
 * así añadirlo a la bitácora mediante la subrutina 'agregarMensaje'.
 * En caso de que el mensaje sea especial, como los vecinos o el número
 * del nodo, estos son añadidos a la página.
 * @param[in] evento: El mensaje recibido que se añade a la bitácora.
 * @pre Que un mensaje haya llegado por el web socket.
 * @author Jeisson Hidaldo, Isaac Herrera B43332
 */
function webSocketMessageReceived(evento) {
  payload = evento.data;
  verificacion = payload.substring(0,1);
  if(isNaN(verificacion) == false){
    agregarMensaje(evento.data, 'normal');

  } else if (verificacion.localeCompare('i') == 0){ //verifica si se
                                  //recibe información sobre el nodo
    id = payload.substring(3);
    const header1 = document.getElementById('head1');
    header1.innerText = 'Nodo ' + id;
    const title1 = document.getElementById('title1');
    title1.innerText = 'Nodo ' + id;

  } else if(verificacion.localeCompare('v') == 0){ //verifica si se
                                  //recibe información de los vecinos
    agregarVecinos(payload);

  } else {

    division = evento.data.split(' ');
    if(division.length === 2){
      const tipo = division[0];
      const valor = division[1];
      if(tipo == 'file'){ //verifica si el mensaje es un enlace
        agregarEnlace(valor);
        document.getElementById('file').disable = false;
      }
    }
  }
}

/**
 * @brief Notifica de un error relacionado con el web socket.
 * @details Mediante la subrutina 'agregarMensaje' se notifica mediante
 * la bitácora que ha ocurrido un error de conexión.
 * @pre Que haya ocurrido un error con el web socket.
 */
function webSocketErrorOcurrido() {
  agregarMensaje('Error de conexión con el nodo verde', 'error');
}

/**
 * @brief Subrutina encargada de iniciar el web socket y de responder
 * a eventos relacionados con este.
 * @details Se crea un 'WebSocket' con una dirección ip y un puerto,
 * que luego deberán ser obtenidos mediante el agente azul. Luego se
 * añaden subrutinas para responder a eventos específicos, como un
 * mensaje entrante o un error relacionado con el web socket.
 * @pre El único requisito para la invocación de esta subrutina es que
 * la página se haya cargado.
 */
function setupWebSocket() {
  const datosWebSocket = document.body.dataset.ws;
  webSocketServer = new WebSocket(`ws://${datosWebSocket}`);

  webSocketServer.addEventListener('open', webSocketOpened);
  webSocketServer.addEventListener('message',webSocketMessageReceived);
  webSocketServer.addEventListener('error', webSocketErrorOcurrido);
  webSocketServer.addEventListener('close', webSocketClosed);
}

// Agregar Archivos --------------------------------------------------

/**
 * @brief Verifica que se cumplan ciertas condiciones para enviar
 * un archivo mediante el web socket.
 * @details Cuando el usuario ha seleccionado un archivo, se verifica
 * si este posee información para ser enviada, basado en su peso, así
 * como si no sobrepasa el límite total de peso.
 * @author Jeisson Hidaldo, Isaac Herrera B43332
 */
function archivoCambio(){
  let vecino =
  document.querySelector('input[name="vecinos"]:checked').value;

  const entradaArchivos = document.getElementById('file');
  if(entradaArchivos.files.length > 0){
    const archivo = entradaArchivos.files[0];
    if(archivo.size < TAM_MAX_ARCHIVO){
      agregarMensaje(`Enviando archivo: "${archivo.name}"\
      (${archivo.size} Bytes) a vecino ${vecino}`, 'enviado');
      if(webSocketServer){
        document.getElementById('file').disable = true;
        let vecino =
        document.querySelector('input[name="vecinos"]:checked').value;
        webSocketServer.send(`__filename ${vecino} ${archivo.name}`);
        webSocketServer.send(archivo);
      }
    } else {
      agregarMensaje(`Error: El archivo "${archivo.name}" (Tamaño: \
      ${archivo.size} Bytes) excede límite de tamaño \ 
      (${TAM_MAX_ARCHIVO} Bytes)`, 'error');
    }
  }
}

/**
 * @brief Función encargada de permitir al usuario seleccionar
 * archivos para enviarlos por el web socket
 * @details Se habilita un boton para el usuario para seleccionar
 * un archivo que se encuentre en su PC, para, acto seguido,
 * enviarlo por el web socket, verificando un par de condiciones,
 * mediante el método "archivoCambio".
 * @author Jeisson Hidaldo, Isaac Herrera B43332
 */
function adjuntarArchivos(){
  const entradaArchivos = document.getElementById('file');
  entradaArchivos.addEventListener('change', archivoCambio);
}

// Form ---------------------------------------------------------------

/**
 * @brief Envía mensajes provenientes de la página hacía el web socket
 * @details Toma los mensajes que vienen de la página web, y los
 * analiza para realizar la acción correspondiente. Se basa en el
 * primer carácter del mensaje, para así saber si es un mensaje normal,
 * de broadcast, de estrés o aleatorio.
 * @pre Que un mensaje del usuario llegue desde la página web
 * @author Jeisson Hidaldo, Isaac Herrera B43332
 * @date 11/12/20
 */
function sendMessage() {

  let numero = 1;
  let plantilla, tipo, mensajeAleatorio, cantidadCaracteres;
  const messageText = document.getElementById('message_field').value;
  primerCaracter = messageText.charAt(0);
  if(primerCaracter == '/' || primerCaracter == '#'){
    numero = parseInt(messageText.substring(1));
  }

  let neighbor =
  document.querySelector('input[name="vecinos"]:checked').value;

  if(neighbor == -1){
    indice = Math.floor(Math.random() * listaVecinos.length);
    neighbor = listaVecinos[indice];
    plantilla = `Para ${neighbor}: `;
    tipo = 'enviado';
  } else if(neighbor == 0){
    plantilla = `Mensaje por Broadcast: `;
    tipo = 'broadcast';
  } else {
    plantilla = `Para ${neighbor}: `;
    tipo = 'enviado';
  }

  if(primerCaracter == '/'){ //verificación para comprobar si se desean
    for(i=0; i<numero; i++){ //enviar muchos mensajes a un vecino
      mensajeAleatorio = '';
      cantidadCaracteres = Math.floor(Math.random() * 200);
      for(j=0; j<cantidadCaracteres; j++){
        mensajeAleatorio += caracteres.charAt(Math.floor(Math.random()
        * caracteres.length));
      }
      agregarMensaje(plantilla + mensajeAleatorio, tipo);
      webSocketServer.send(`${neighbor}::${mensajeAleatorio}`);
    }
  } else if(primerCaracter == '#'){ //verificación para comprobar si se
      mensajeAleatorio = '';        //enviar mensaje vía forwarding
      cantidadCaracteres = Math.floor(Math.random() * 200);
      for(j=0; j<cantidadCaracteres; j++){
        mensajeAleatorio += caracteres.charAt(Math.floor(Math.random()
        * caracteres.length));
      }
      agregarMensaje(`Para ${numero}: ` + mensajeAleatorio, tipo);
      webSocketServer.send(`${numero}::${mensajeAleatorio}`);
  } else {
    agregarMensaje(plantilla + messageText, tipo);
    webSocketServer.send(`${neighbor}::${messageText}`);
  }
}

/**
 * @brief Función encargada de inicializar funciones para el envío de
 * mensajes
 * @details Añade "listeners" al campo de texto para detectar los
 * mensajes y al botón para enviarlos.
 * @author Jeisson Hidaldo, Isaac Herrera
 * @date 11/12/20
 */
function setupClient() {
  setupWebSocket();
  adjuntarArchivos();

  const boton = document.getElementById('send_button');
  boton.addEventListener('click', sendMessage);
  const campoTexto = document.getElementById('message_field');
  campoTexto.addEventListener('keyup', function(evento){
    if(evento.key === 'Enter'){
      evento.preventDefault();
      sendMessage();
    }
  });
}

window.addEventListener('load', setupClient);