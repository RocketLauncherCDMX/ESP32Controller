// ----------------------------------------- ESP32 CONTROLLER -------------------------------------------------
//
// EJEMPLO DE LECTURA DIGITAL DE BOTONES Y PUBLICACION VIA Wi-Fi EN INTERFAZ WEB
// Autor: Rocket Launcher
// Fecha: 09 de junio de 2025
//
//
// ----------------------------------- FUNCIONES CLAVE DEL EJEMPLO --------------------------------------------
//
// - Controller.getAllButtons()      - Lee el estado interno de TODOS los botones.
// - updateButtonState(btn, state)   - Copia el estado de un boton concreto al arreglo buttonState[8] que
//                                     mas tarde se envia al navegador.
// - getHTML()                       - Genera dinamicamente el documento HTML con JavaScript que solicita,
//                                     cada 50 ms, el estado de los botones y actualiza los iconos en pantalla.
// - handleRoot()                    - Atiende la ruta "/" y devuelve el HTML generado por getHTML().
// - handleStates()                  - Devuelve, en formato JSON, el arreglo buttonState[] que el
//                                     navegador consulta mediante AJAX.
// - loop()                          - 1) Lee los botones con getAllButtons().
//                                     2) Actualiza buttonState[] con updateButtonState().
//                                     3) Atiende peticiones HTTP mediante server.handleClient().
//
//
// ------------------------------------------ CONSIDERACIONES -------------------------------------------------
//
// Este ejemplo requiere de conocimiento basico sobre paginas web, asi como acceso a un router inalambrico.
// Asi, el ESP32 actua como un servidor web que publica en tiempo real el estado de los
// botones del control ESP32 en cualquier navegador conectado a la misma red Wi-Fi.
//
// IMPORTANTE 1: Debido a que la comunicacion se hace a traves de AJAX, en algunas ocasiones el tiempo de
// reaccion para actualizar la pagina puede verse comprometida o en el peor caso, congelarse. Este es solo
// un codigo de ejemplo, y no es recomendable para control en tiempo real. Para ese caso se recomienda ver
// el ejemplo webSocketsDemo.
//
// IMPORTANTE 2: La conexion USB se detectara con el control apagado, pero para que el codigo se suba, 
// es vital que este se encuentre encendido (LED azul).
//
// IMPORTANTE 3: El ESP32 Controller solamente admite conexiones con WiFi de 2.4GHz
//
//
// ------------------------------------ PASOS PARA EJECUTAR EL EJEMPLO ------------------------------------------
//
// ESP32 CONTROLLER:
// 1. Conecta tu control a la computadora con un cable USB C. Se debera encender un LED rojo debajo
//    del boton P3, lo cual indica que la bateria esta cargando. Cuando el LED pase a verde, indica que la
//    carga se completo.
// 2. Presiona el boton P3 para encender o apagar el control. Un LED azul indica que el control esta encendido.
//    Recuerda que el codigo solo se subira si se encuentra encendido.
// 
// CODIGO:
// 1. En las Credenciales Wifi, cambia SSID por el nombre de tu red y PASS por el password.
// 4. Compila y sube este codigo a tu ESP32 controller, utilizando la placa ESP32 Dev Module para ello.    
// 5. Una vez corriendo, abre la consola serial, si ves una serie de puntos (................) que van
//    apareciendo sin fin, es por que el nombre de la red o la clave son incorrectos, revisalos y vuelve
//    a subir el codigo.
// 6. Una vez que se logre la conexion aparecera una IP, por ejemplo: 192.168.1.76 deberas poner ese
//    numero en tu navegador, lo cual abrira la pagina alojada en el ESP32 controller.
// 7. Listo! Ya puedes interactuar desde el control a la pagina web, incluso con el cable USB desconectado :)
//
// ---------------------------------------------------------------------------------------------------------------

#include <WiFi.h>
#include <WebServer.h>
#include <ESPController.h>         // Libreria del mando

ESPController Controller;          // Instancia del controlador

// ------------------------------------- Credenciales Wi-Fi ----------------------------------------------
const char* ssid     = "SSID";     // Sustituye por tu red
const char* password = "PASS";     // Sustituye por tu contrasena

WebServer server(80);              // Servidor HTTP en el puerto 80

// ---------- Estados de los 8 botones (Up, Down, Left, Right, Cross, Square, Circle, Triangle) ----------
bool buttonState[8] = {false, false, false, false, false, false, false, false};

// ---------------------- Generador de la pagina web (HTML + JavaScript) ---------------------------------
String getHTML() {
  String html  = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>ESP32 Control</title>";
  html += "<style>button{font-size:30px;padding:10px;margin:5px;}</style>";
  html += "<script>function updateButtons(){";
  html += "fetch('/states').then(r=>r.json()).then(states=>{";
  html += "const e=['🔼','🔽','◀️','▶️','❌','⏹️','⭕','🔺'];";
  html += "for(let i=0;i<8;i++){document.getElementById('btn'+i).innerText=states[i]?'🔵':e[i];}";
  html += "});}setInterval(updateButtons,50);</script></head>";
  html += "<body onload='updateButtons()'><h2>Control de botones</h2>";
  for (int i = 0; i < 8; i++) html += "<button id='btn" + String(i) + "'>⚪</button>";
  html += "</body></html>";
  return html;
}

// ----------------------------------- Rutas del servidor -------------------------------------------------
void handleRoot()   { server.send(200, "text/html", getHTML()); }
void handleStates() {
  String json = "[";
  for (int i = 0; i < 8; i++) { json += buttonState[i] ? "true" : "false"; if(i<7) json += ","; }
  json += "]";
  server.send(200, "application/json", json);
}

// ------------------------ Copia el estado de un boton al arreglo global ---------------------------------
void updateButtonState(int btn, bool state) {
  if (btn >= 0 && btn < 8) buttonState[btn] = state;
}

// --------------------------------- Configuracion inicial ------------------------------------------------
void setup() {
  Serial.begin(115200);   // Comienza la comunicacion Serial con la PC
  Controller.begin();     // Inicializa el control

  WiFi.begin(ssid, password);           // Inicializa la conexion WiFi
  Serial.print("Conectando a Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }  // Imprime (.) mientras conecta
  Serial.println("\nConectado! IP: " + WiFi.localIP().toString());  // muestra la IP a la que se conecto

  server.on("/", handleRoot);           // maneja la ruta despues de la IP
  server.on("/states", handleStates);   // detecta el comando y actua en consecuencia
  server.begin();                       // Inicializa el servidor en el ESP32 Controller
}

// ---------- Bucle principal ----------
void loop() {
  Controller.getAllButtons();  // Lee todos los botones

  // Boton 0 - Flecha arriba
  updateButtonState(0, Controller.Up.status == PRESSED);

  // Boton 1 - Flecha abajo
  updateButtonState(1, Controller.Down.status == PRESSED);

  // Boton 2 - Flecha izquierda
  updateButtonState(2, Controller.Left.status == PRESSED);

  // Boton 3 - Flecha derecha
  updateButtonState(3, Controller.Right.status == PRESSED);

  // Boton 4 - Boton Cross
  updateButtonState(4, Controller.Cross.status == PRESSED);

  // Boton 5 - Boton Square
  updateButtonState(5, Controller.Square.status == PRESSED);

  // Boton 6 - Boton Circle
  updateButtonState(6, Controller.Circle.status == PRESSED);

  // Boton 7 - Boton Triangle
  updateButtonState(7, Controller.Triangle.status == PRESSED);

  server.handleClient();  // Atiende peticiones HTTP
}
