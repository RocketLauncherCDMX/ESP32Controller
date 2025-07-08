// ----------------------------------------- ESP32 CONTROLLER -------------------------------------------------
//
// EJEMPLO DE PRUEBA DE VIBRADORES (RUMBLE) DEL CONTROL ESP32
// Autor: Rocket Launcher
// Fecha: 09 de junio de 2025
//
//
// ----------------------------------- FUNCIONES CLAVE DEL EJEMPLO --------------------------------------------
//
// - Controller.getAllButtons()         - Actualiza el estado de TODOS los botones.
//
// - Controller.rumble()                - Si no se especifica cual, enciende ambos motores de vibracion.
// - Controller.rumble(side)            - side = WEAK   -> enciende motor izquierdo
//                                      - side = STRONG -> enciende motor derecho
//
// - Controller.stopRumble()            - Si no se especifica cual, apaga ambos motores de vibracion.
// - Controller.stopRumble(side)        - side = WEAK   -> apaga motor izquierdo
//                                      - side = STRONG -> apaga motor derecho
//
//
// ------------------------------------------ CONSIDERACIONES --------------------------------------------------
//
// - Si el programa no sube, recuerda que el control debe estar ENCENDIDO (LED azul) antes de cargar el programa.
// 
//
// ----------------------------------- MAPEO DE BOTONES EN ESTE EJEMPLO ---------------------------------------
//
// - Boton TRIANGLE  -> Enciende el vibrador STRONG
// - Boton CROSS     -> Apaga el vibrador STRONG   (stopRumble)
// - Boton UP        -> Enciende el vibrador WEAK
// - Boton DOWN      -> Apaga el vibrador WEAK     (stopRumble)
//
// ------------------------------------------------------------------------------------------------------------

#include <ESPController.h>   // Biblioteca del ESP32 Controller

ESPController Controller;    // Instancia del controlador

// --------------------------------------------------
// CONFIGURACION INICIAL
// --------------------------------------------------
void setup() {
  Controller.begin();    // Inicializa hardware del control
}

// --------------------------------------------------
// BUCLE PRINCIPAL
// --------------------------------------------------
void loop() {
  // Actualiza estados de todos los botones
  Controller.getAllButtons();

  // Control del vibrador derecho STRONG (motor fuerte)
  if (Controller.Triangle.status == PRESSED) {     // Encender con TRIANGLE
    Controller.rumble(STRONG);
  }
  if (Controller.Cross.status == PRESSED) {        // Apagar con CROSS
    Controller.stopRumble(STRONG);                 // Detiene ambos motores
  }

  // Control del vibrador izquierdo WEAK (motor debil)
  if (Controller.Up.status == PRESSED) {           // Encender con UP
    Controller.rumble(WEAK);
  }
  if (Controller.Down.status == PRESSED) {         // Apagar con DOWN
    Controller.stopRumble(WEAK);                   // Detiene ambos motores
  }

  // Control de ambos motores
  if (Controller.R3.status == PRESSED) {           // Encender con R3
    Controller.rumble();                           // Enciende ambos motores
  }
  if (Controller.L3.status == PRESSED) {           // Apagar con L3
    Controller.stopRumble();                       // Detiene ambos motores
  }
}
