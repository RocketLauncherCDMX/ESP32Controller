// ------------------------------------------ ESP32 CONTROLLER --------------------------------------------------
//
// EJEMPLO DE LECTURA DIGITAL DE JOYSTICKS Y BOTONES DEL CONTROL ESP32
// Este ejemplo es de uso libre y esta pensado para dar una introduccion al uso del ESPController
// Autor: Rocket Launcher
// Fecha: 09 de junio de 2025
// --------------------------------------------------------------------------------------------------------------
//
// En este ejemplo conoceremos la forma de leer los valores analogicos de los joysticks izquierdo y derecho,
// asi como el estado digital de todos los botones del mando ESP32 controller.
//
// Para leer el estado de los botones, basta con llamar a la funcion getAllButtons().
// Para imprimir su valor en formato binario, se puede usar la funcion printButtonsBinary().
//
// Los Joysticks se pueden leer de 2 formas diferentes: 
//
// de 0 a 4095    - sirve para tener el valor directo del convertidor Analogico a Digital (ADC)
// de -255 a 255  - nos da un valor central 0 e incrementos a los lados, ideal para manejar motores, por ejemplo.
//
// Para leer el valor en crudo (raw) de los joysticks, se usan los metodos:
//
// JoystickLeft.readRaw();  // Lee el valor de los ejes X y Y del joystick izquierdo (0 a 4095).
// JoystickRight.readRaw(); // Lee el valor de los ejes X y Y del joystick derecho (0 a 4095).
//
// En esta forma, el valor en reposo de cada joystick es aproximadamente 2048.
//
// Para leer el valor con signo de los joysticks, se usan los metodos:
//
// JoystickLeft.readSign();  // Lee el valor de los ejes X y Y del joystick izquierdo (-255 a 255).
// JoystickRight.readSign(); // Lee el valor de los ejes X y Y del joystick izquierdo (-255 a 255).
//
// En esta forma, el valor en reposo de cada joystick es aproximadamente 0.
//
// --------------------------------------------------------------------------------------------------------------

#include <ESPController.h>     // Incluye la libreria del controlador

ESPController Controller;      // Crea una instancia del controlador

void setup() {
  Serial.begin(115200);        // Inicializa la comunicacion serial a 115200 baudios
  Controller.begin();          // Inicializa el controlador
}

void loop() {
  Controller.getAllButtons();          // Lee el estado digital de todos los botones
  //Controller.JoystickLeft.readRaw();  // joystick izquierdo (0 a 4095).
  //Controller.JoystickRight.readRaw(); // joystick derecho (0 a 4095).
  Controller.JoystickLeft.readSign();  // joystick izquierdo (-255 a 255).
  Controller.JoystickRight.readSign(); // joystick izquierdo (-255 a 255).

  // Imprime los valores X e Y de ambos joysticks por el monitor serial
  Serial.print(Controller.JoystickLeft.x);    // Izquierdo, eje horizontal
  Serial.print("\t");
  Serial.print(Controller.JoystickLeft.y);    // Izquierdo, eje vertical
  Serial.print("\t");
  Serial.print(Controller.JoystickRight.x);   // Derecho, eje horizontal
  Serial.print("\t");
  Serial.print(Controller.JoystickRight.y);   // Derecho, eje vertical
  Serial.print("\t ");

  // Imprime el estado de todos los botones en binario
  Controller.printButtonsBinary();
}
