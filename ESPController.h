#ifndef _ESPCONTROLLER_H_
#define _ESPCONTROLLER_H_

#include <Arduino.h>
#include <Wire.h>

// ----------------------------------------------------------------------------------------------------
// ESPController classes
class ESPController {
private:
  // Global private constants
  #define RESET         21
  #define ADDRESS1      0x19
  #define ADDRESS2      0x1B

  uint8_t readRegister(uint8_t);
  uint8_t buttonsGroup1;
  uint8_t buttonsGroup2;

  bool buttonUp_new = LOW,       buttonUp_old = LOW,       :1;
  bool buttonDown_new = LOW,     buttonDown_old = LOW,     :1;
  bool buttonLeft_new = LOW,     buttonLeft_old = LOW,     :1;
  bool buttonRight_new = LOW,    buttonRight_old = LOW,    :1;
  bool buttonSelect_new = LOW,   buttonSelect_old = LOW,   :1;
  bool buttonStart_new = LOW,    buttonStart_old = LOW,    :1;
  bool buttonPs_new = LOW,       buttonPs_old = LOW,       :1;
  bool buttonTriangle_new = LOW, buttonTriangle_old = LOW, :1;
  bool buttonCross_new = LOW,    buttonCross_old = LOW,    :1;
  bool buttonSquare_new = LOW, buttonSquare_old = LOW, buttonSquare_pressed = LOW, buttonSquare_released = LOW, :1;
  bool buttonCircle_new = LOW, buttonCircle_old = LOW, buttonCircle_pressed = LOW, buttonCircle_released = LOW, :1;
  bool buttonL1_new = LOW,       buttonL1_old = LOW,       :1;
  bool buttonL2_new = LOW,       buttonL2_old = LOW,       :1;
  bool buttonR1_new = LOW,       buttonR1_old = LOW,       :1;
  bool buttonR2_new = LOW,       buttonR2_old = LOW,       :1;
  
public:
  #define false         0
  #define true          1
  
  #define RELEASED      0
  #define PRESSED       1
  #define HOLD_RELEASED 2
  #define HOLD_PRESSED  3

  #define NO_PLAYER     0
  #define PLAYER1       1
  #define PLAYER2       2
  #define PLAYER3       3
  #define PLAYER4       4
  
  #define NONE          0
  #define CIRCLE        1
  #define SQUARE        2
  #define TRIANGLE      3
  #define CROSS         4
  #define UP            5
  #define DOWN          6
  #define LEFT          7
  #define RIGHT         8

  #define WEAK          0
  #define STRONG        1

  #define OFF           0
  #define RED           1
  #define GREEN         2
  #define BLUE          3
  #define YELLOW        4
  #define CYAN          5
  #define MAGENTA       6
  #define WHITE         7

  #define NUM_READINGS  5
  // Global public  constants
  #define MOTOR_STRONG  16
  #define MOTOR_WEAK    17
  #define PUSH_LEFT     4
  #define PUSH_RIGHT    2
  #define LEFT_JOY_V    14
  #define LEFT_JOY_H    15
  #define RIGHT_JOY_V   12
  #define RIGHT_JOY_H   13
  #define LED1          26
  #define LED2          27
  #define LED3          32
  #define LED4          33
  
  class button {
  public:
    String name;
    uint8_t status = HOLD_RELEASED;
    boolean newStatus = RELEASED;
    boolean oldStatus = RELEASED;
  } Circle, Square, Triangle, Cross, Up, Down, Left, Right, Select, Start, Ps, L1, L2, R1, R2, JoyLeft, JoyRight;

  boolean buttonChanged = false, :1;
  boolean buttonPressed = false, :1;

  uint8_t player = NO_PLAYER;

  uint16_t leftJoyH_centre = 0;
  uint16_t leftJoyV_centre = 0;
  uint16_t rightJoyH_centre = 0;
  uint16_t rightJoyV_centre = 0;

  uint8_t readIndex = 0;
  uint16_t leftJoyH_readings[NUM_READINGS];   
  uint32_t leftJoyH_total = 0; 
  int16_t leftJoyH = 0;
  uint16_t leftJoyH_average = 0;
  uint16_t leftJoyV_readings[NUM_READINGS];   
  uint32_t leftJoyV_total = 0; 
  int16_t leftJoyV = 0;
  uint16_t rightJoyH_readings[NUM_READINGS];   
  uint32_t rightJoyH_total = 0; 
  int16_t rightJoyH = 0;
  uint16_t rightJoyV_readings[NUM_READINGS];   
  uint32_t rightJoyV_total = 0; 
  int16_t rightJoyV = 0;

  void begin();
  void readAxisRaw();
  void readAxisSign();
  void getButtons();
  void vibrate(boolean, uint16_t);
  void Player(uint8_t);

  class pixel {
  private:
    uint8_t value[3];  // GRB value
    uint8_t col, bit;
    uint8_t i;
  public:
    void color(uint8_t,uint8_t,uint8_t);
    void color(uint8_t);
    void fadeInOut(uint8_t, uint8_t);
    void fadeInOut(uint8_t);
    void off(void);

  } Neopixel;

  
};

#endif
