// ------------------------------------------- ESP32 CONTROLLER ---------------------------------------------
//
// EJEMPLO DE DETECCION DE ORIENTACION CON EL ACELEROMETRO DEL ESP32 CONTROLLER
// Este ejemplo es de uso libre y está pensado para dar una introducción al uso avanzado
// del acelerómetro interno del mando ESP32 Controller (mod de control PS3).
//
// Autor: Rocket Launcher
// Fecha: 15 de junio de 2025
// Visita https://github.com/RocketLauncherCDMX/ESP32Controller para más información
// -----------------------------------------------------------------------------------------------------------
//
// En este ejemplo aprenderás a:
//
// Detectar la orientación estática del mando usando:
// Controller.Accel.detectPos();
//
// ORIENTACIONES RECONOCIDAS (enum Controller.Accel.Pos):
// -------------------------------------------------------
//  POS_REST      → Mando “plano”, joysticks hacia arriba.
//  POS_FRONT     → Gatillos apuntan al suelo (frente).          (Pitch +90°)
//  POS_FACE_DOWN → Mando boca abajo, joysticks contra la mesa.
//  POS_LEFT      → Giro 90° a la izquierda (roll –90°).          L1 hacia el suelo.
//  POS_RIGHT     → Giro 90° a la derecha  (roll +90°).          R1 hacia el suelo.
//  POS_FRONT_90  → Gatillos apuntan al jugador (pitch –90°).
//
// Cada posición se identifica con una tolerancia del ±20 % respecto a los valores
// típicos de 1 g en el eje dominante.
// -----------------------------------------------------------------------------------------------------------

#include <ESPController.h>

ESPController Controller;          // Instancia global del mando

// Para evitar spam en la consola imprimiremos orientación solo cuando cambie
ESPController::Accelerometer::Pos lastPos = Controller.Accel.POS_UNKNOWN;

void setup()
{
  Serial.begin(115200);   // Comunicación Serial, puedes ver la posición en consola
  Controller.begin();     // Inicializa botones, LEDs y acelerometro
}

void loop()
{
  // Leemos la orientacion actual del mando
  // ---------------------------------------------------------------
  // detectPos() devuelve un valor del enum Accelerometer::Pos:
  // Guardamos el resultado en la variable local pos.
  
  ESPController::Accelerometer::Pos pos = Controller.Accel.detectPos();

  // Solo procesa una posicion si es nueva, esto para no repetir todo
  // el tiempo las acciones.
  
  if (pos != lastPos && pos != Controller.Accel.POS_UNKNOWN) {

    // Actualizamos el ultimo valor leido para compararlo en la siguiente lectura
    lastPos = pos;
    
    Serial.print("Orientación: ");
  
    // Evaluamos la posicion detectada y prendemos los LEDs de colores diferentes
      switch (pos) {
      case Controller.Accel.POS_REST:   // En reposo sobre una mesa
        Serial.println("REPOSO");
        Controller.Rgb.color(OFF);      // Apaga los LEDs
        break;
  
      case Controller.Accel.POS_FRONT:  // Gatillos apuntan al suelo
        Serial.println("FRENTE");
        Controller.Rgb.color(GREEN);    // LEDs en Verde
        break;
  
      case Controller.Accel.POS_FACE_DOWN:  // Mando boca abajo
        Serial.println("BOCA ABAJO");
        Controller.Rgb.color(CYAN);         // LEDs en Cyan
        break;
  
      case Controller.Accel.POS_LEFT:     // Mando ladeado a la izquierda
        Serial.println("IZQUIERDA 90°");  
        Controller.Rgb.color(BLUE);       // LEDs en Azul
        break;
  
      case Controller.Accel.POS_RIGHT:    // Mando ladeado a la derecha
        Serial.println("DERECHA 90°");    
        Controller.Rgb.color(MAGENTA);    // LEDs en Magenta
        break;
  
      
      case Controller.Accel.POS_FRONT_90:
        Serial.println("FRONTAL 90°");    // Mando de cabeza, con botones al frente
        Controller.Rgb.color(YELLOW);     // LEDs en Amarillo
        break;
    }
  }
}