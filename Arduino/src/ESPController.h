#ifndef _ESPCONTROLLER_H_
#define _ESPCONTROLLER_H_

#include <Arduino.h>
#include <Wire.h>
#include <driver/rmt.h> 
#include "accelerometer/kxtj3-1057.h" 

// -------------------------------------------------
// Ajuste de color GRB para los dos Neopixels:
static uint8_t value[6];  // GRB value for the two Neopixels

class ESPController {
private:
  // Pines y direcciones I2C
  #define RESET         21
  #define ADDRESS1      0x19
  #define ADDRESS2      0x1B

  // Motores (vibracion)
  #define MOTOR_STRONG  14
  #define MOTOR_WEAK    17

  // Botones
  #define PUSH_LEFT     4
  #define PUSH_RIGHT    12

  // Joysticks
  #define LEFT_JOY_V    35
  #define LEFT_JOY_H    34
  #define RIGHT_JOY_V   39
  #define RIGHT_JOY_H   36

  // LEDs player
  #define LED1          15
  #define LED2          27
  #define LED3          32
  #define LED4          33

  // Pin de Neopixel
  #define NEOP_PIN      19

  // Pin de apagado
  #define SHUTDOWN      18

  // Lectura y escritura a registros I2C
  uint8_t readRegister(uint8_t);
  void writeRegister(uint8_t, uint8_t, uint8_t);

  // Grupos de botones en los registros I2C
  uint8_t buttonsGroup1;
  uint8_t buttonsGroup2;

  // Acelerometro KXTJ3-1053
  KXTJ3 imu{0x0E};               // Dirección I2C por defecto del KXTJ3-1057
  bool  imuOK = false;           // true - IMU listo

  /// Arranca el IMU con parámetros internos (oculto al usuario)
  bool  beginAccel_(float sampleRate = 50,   // Hz
                    uint8_t range    = 2,    // +-2 g
                    bool highRes     = true,
                    bool debug       = false);

public:
  // Definiciones generales
  #define false         0
  #define true          1
  
  #define RELEASED      0
  #define PRESSED       1
  #define HOLD_RELEASED 2
  #define HOLD_PRESSED  3

  #define NO_PLAYER         0
  #define PLAYER1           1
  #define PLAYER2           2
  #define PLAYER3           3
  #define PLAYER4           4
  #define PLAYER_EFFECT1    5
  #define PLAYER_EFFECT2    6
  #define PLAYER_EFFECT3    7
  #define PLAYER_EFFECT4    8
  
  #define NONE          0
  #define CIRCLE        1
  #define SQUARE        2
  #define TRIANGLE      3
  #define CROSS         4
  #define UP            5
  #define DOWN          6
  #define LEFT          7
  #define RIGHT         8
  #define SELECT        9
  #define START         10

  #define WEAK          0
  #define STRONG        1

  // Colores neopixel
  #define OFF           0
  #define RED           1
  #define GREEN         2
  #define BLUE          3
  #define YELLOW        4
  #define CYAN          5
  #define MAGENTA       6
  #define WHITE         7

  // ----------------------------------------------
  // Clase interna para estado de cada boton
  class button {
  public:
    String name;
    uint8_t status = HOLD_RELEASED;
    bool newStatus = RELEASED;
    bool oldStatus = RELEASED;
  } Up, Down, Left, Right, Circle, Square, Triangle, Cross, Select, Start, P3, L1, L2, L3, R1, R2, R3;

  bool buttonChanged = false, :1;
  bool buttonPressed = false, :1;

  uint8_t player = NO_PLAYER;
  unsigned long previousMillis = 0;
  int step = 0;

  // ----------------------------------------------
  // Joystick
  class joystick {
  public:
    #define NUM_READINGS  10
    uint8_t side;         // LEFT o RIGHT
    int16_t horCentre = 0;
    int16_t verCentre = 0;
    uint8_t readIndex = 0;
    int32_t horReadings[NUM_READINGS];   
    int32_t verReadings[NUM_READINGS];
    int32_t horTotal = 0; 
    int32_t verTotal = 0; 
    int32_t x = 0;
    int32_t y = 0;
    
    void calibrate();
    void readRaw();
    void readSign();
  } JoystickLeft, JoystickRight;

  // ----------------------------------------------
  // Clase para controlar los dos neopixels a la vez
  class bothPixels {
  private:
    uint8_t col, bit;
    uint8_t i;
  public:
    uint8_t side; // no se usa realmente, pero lo dejamos
    void color(uint8_t r, uint8_t g, uint8_t b);
    void color(uint8_t colorName);
    void fadeInOut(uint8_t colorName, uint8_t speed);
    void fadeInOut(uint8_t colorName);
    void off(void);
  } Rgb;

  // ----------------------------------------------
  // Clase para controlar un neopixel individual (izq o der)
  class pixel {
  private:
    uint8_t col, bit;
    uint8_t i;
  public:
    uint8_t side; // LEFT o RIGHT
    void color(uint8_t r, uint8_t g, uint8_t b);
    void color(uint8_t colorName);
    void fadeInOut(uint8_t colorName, uint8_t speed);
    void fadeInOut(uint8_t colorName);
    void off(void);
  } ledLeft, ledRight;

  class Accelerometer {
    friend class ESPController;                 // solo ESPController crea instancias
  private:
    KXTJ3* imu;
    bool*  ok;
    Accelerometer(KXTJ3* imuPtr, bool* okPtr)   // ctor privado
      : imu(imuPtr), ok(okPtr) {}
  public:
    /* Lecturas en g */
    float x() const          { return *ok ? imu->axisAccel(X) : NAN; }
    float y() const          { return *ok ? imu->axisAccel(Y) : NAN; }
    float z() const          { return *ok ? imu->axisAccel(Z) : NAN; }
    
    // Orientaciones del sensor KXTJ-1053
    enum Pos : int8_t {
      POS_UNKNOWN   = -1,  ///< No encaja en ningun patron
      POS_REST      = 0,   ///< Plano, joysticks hacia arriba
      POS_FRONT     = 1,   ///< Frente (gatillos apuntan al suelo)
      POS_FACE_DOWN = 2,   ///< Boca abajo
      POS_LEFT      = 3,   ///< Inclinacion 90 grados a la izquierda
      POS_RIGHT     = 4,   ///< Inclinacion 90 grados a la derecha
      POS_FRONT_90  = 5    ///< Inclinacion 90 grados hacia el jugador
    };
    Pos detectPos() const;

    /* Utilidades extra */
    bool  dataReady() const  { return *ok && imu->dataReady();      }
    bool  motionDetected() const { return *ok && imu->motionDetected(); }
    wu_axis_t motionDir() const  { return *ok ? imu->motionDirection()
                                              : BLANK;               }
  };

  Accelerometer Accel{ &imu, &imuOK };

  // ----------------------------------------------
  // Metodos principales de ESPController
  void begin();
  void printButtonsBinary();
  void getAllButtons();
  void rumble();
  void rumble(bool strength);
  void stopRumble();
  void stopRumble(bool side);
  void Player(uint8_t player);
  void off();
} ;

#endif
