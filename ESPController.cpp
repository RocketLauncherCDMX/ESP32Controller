#include <Arduino.h>
#include "ESPController.h"

#ifndef _ESPCONTROLLER_CPP_
#define _ESPCONTROLLER_CPP_

Adafruit_NeoPixel dot(1, 18, NEO_GRB + NEO_KHZ800);  // 1 neopixel en pin18

bool flag = 1;

void ESPController :: begin() {
  pinMode(RESET, OUTPUT);
  pinMode(MOTOR_STRONG, OUTPUT);
  pinMode(MOTOR_WEAK, OUTPUT);
  pinMode(PUSH_LEFT, INPUT_PULLUP);
  pinMode(PUSH_RIGHT, INPUT_PULLUP);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(LED4, OUTPUT);
  digitalWrite(RESET, HIGH);
  
  Wire.begin();

  dot.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  dot.show();            // Turn OFF all pixels ASAP
  dot.setBrightness(255); // Set BRIGHTNESS to about 1/5 (max = 255)

  //
  for (uint8_t thisReading = 0; thisReading < NUM_READINGS; thisReading++) {
    leftJoyH_readings[thisReading] = 0;
    leftJoyV_readings[thisReading] = 0;
    rightJoyH_readings[thisReading] = 0;
    rightJoyV_readings[thisReading] = 0;
  }

  for(uint8_t readings=0; readings<100; readings++) {
    leftJoyH_total = leftJoyH_total - leftJoyH_readings[readIndex];
    leftJoyH_readings[readIndex] = analogRead(LEFT_JOY_H);
    leftJoyH_total = leftJoyH_total + leftJoyH_readings[readIndex];
    leftJoyV_total = leftJoyV_total - leftJoyV_readings[readIndex];
    leftJoyV_readings[readIndex] = analogRead(LEFT_JOY_V);
    leftJoyV_total = leftJoyV_total + leftJoyV_readings[readIndex];

    rightJoyH_total = rightJoyH_total - rightJoyH_readings[readIndex];
    rightJoyH_readings[readIndex] = analogRead(RIGHT_JOY_H);
    rightJoyH_total = rightJoyH_total + rightJoyH_readings[readIndex];
    rightJoyV_total = rightJoyV_total - rightJoyV_readings[readIndex];
    rightJoyV_readings[readIndex] = analogRead(RIGHT_JOY_V);
    rightJoyV_total = rightJoyV_total + rightJoyV_readings[readIndex];
    
    readIndex++;
    if (readIndex >= NUM_READINGS) readIndex = 0;
  }
  leftJoyH_centre = 4095 - (leftJoyH_total / NUM_READINGS);
  leftJoyV_centre = 4095 - (leftJoyV_total / NUM_READINGS);
  rightJoyH_centre = rightJoyH_total / NUM_READINGS;
  rightJoyV_centre = rightJoyV_total / NUM_READINGS;
};

void ESPController :: readAxisRaw() {
  leftJoyH_total = leftJoyH_total - leftJoyH_readings[readIndex];
  leftJoyH_readings[readIndex] = analogRead(LEFT_JOY_H);
  leftJoyH_total = leftJoyH_total + leftJoyH_readings[readIndex];
  leftJoyV_total = leftJoyV_total - leftJoyV_readings[readIndex];
  leftJoyV_readings[readIndex] = analogRead(LEFT_JOY_V);
  leftJoyV_total = leftJoyV_total + leftJoyV_readings[readIndex];
  
  rightJoyH_total = rightJoyH_total - rightJoyH_readings[readIndex];
  rightJoyH_readings[readIndex] = analogRead(RIGHT_JOY_H);
  rightJoyH_total = rightJoyH_total + rightJoyH_readings[readIndex];
  rightJoyV_total = rightJoyV_total - rightJoyV_readings[readIndex];
  rightJoyV_readings[readIndex] = analogRead(RIGHT_JOY_V);
  rightJoyV_total = rightJoyV_total + rightJoyV_readings[readIndex];
  
  readIndex++;
  if (readIndex >= NUM_READINGS) readIndex = 0;

  // calculate the average:
  leftJoyH = 4095 - (leftJoyH_total / NUM_READINGS);
  leftJoyV = 4095 - leftJoyV_total / NUM_READINGS;
  rightJoyH = rightJoyH_total / NUM_READINGS;
  rightJoyV = rightJoyV_total / NUM_READINGS;
};

void ESPController :: readAxisSign() {
  readAxisRaw();

  if(leftJoyH < leftJoyH_centre-1) leftJoyH = map(leftJoyH, 0, leftJoyH_centre, -255, 0);
  else if(leftJoyH > leftJoyH_centre+1) leftJoyH = map(leftJoyH, leftJoyH_centre, 4095, 0, 255);
  else leftJoyH = 0;

  if(leftJoyV < leftJoyV_centre-1) leftJoyV = map(leftJoyV, 0, leftJoyV_centre, -255, 0);
  else if(leftJoyV > leftJoyV_centre+1) leftJoyV = map(leftJoyV, leftJoyV_centre, 4095, 0, 255);
  else leftJoyV = 0;

  if(rightJoyH < rightJoyH_centre-1) rightJoyH = map(rightJoyH, 0, rightJoyH_centre, -255, 0);
  else if(rightJoyH > rightJoyH_centre+1) rightJoyH = map(rightJoyH, rightJoyH_centre, 4095, 0, 255);
  else rightJoyH = 0;
  
  if(rightJoyV < rightJoyV_centre-1) rightJoyV = map(rightJoyV, 0, rightJoyV_centre, -255, 0);
  else if(rightJoyV > rightJoyV_centre+1) rightJoyV = map(rightJoyV, rightJoyV_centre, 4095, 0, 255);
  else rightJoyV = 0;  
};

uint8_t ESPController :: readRegister(uint8_t address) {
  //start the communication with IC with the address xx
  Wire.beginTransmission(address); 
  //send a bit and ask for register zero
  Wire.write(0);
  //end transmission
  Wire.endTransmission();
  //request 1 byte from address xx
  Wire.requestFrom(address, 1);
  //wait for response
  while(Wire.available() == 0);
  //put the temperature in variable c
  int c = Wire.read();   
  return c;
}

void ESPController :: getButtons() {
  buttonsGroup1   = readRegister(ADDRESS1);
  buttonsGroup2   = readRegister(ADDRESS2);

  buttonR1        = (buttonsGroup1 & 0b10000000) >> 7;
  buttonCircle    = (buttonsGroup1 & 0b01000000) >> 6;
  buttonSquare    = (buttonsGroup1 & 0b00100000) >> 5;
  buttonStart     = (buttonsGroup1 & 0b00010000) >> 4;
  buttonPs        = !((buttonsGroup1 & 0b00001000) >> 3);
  buttonDown      = !((buttonsGroup1 & 0b00000100) >> 2);
  buttonUp        = !((buttonsGroup1 & 0b00000010) >> 1);
  buttonL2        = !((buttonsGroup1 & 0b00000001) >> 0);

  buttonR2        = (buttonsGroup2 & 0b01000000) >> 6;
  buttonTriangle  = (buttonsGroup2 & 0b00100000) >> 5;
  buttonCross     = (buttonsGroup2 & 0b00010000) >> 4;
  buttonSelect    = !((buttonsGroup2 & 0b00001000) >> 3);
  buttonRight     = !((buttonsGroup2 & 0b00000100) >> 2);
  buttonLeft      = !((buttonsGroup2 & 0b00000010) >> 1);
  buttonL1        = !((buttonsGroup2 & 0b00000001) >> 0);

  buttonCircle_new = buttonCircle;
  if(buttonCircle_new == LOW && buttonCircle_old == HIGH) buttonCircle_released = true;
  else if(buttonCircle_new == HIGH && buttonCircle_old == LOW) buttonCircle_pressed = true;
  buttonCircle_old = buttonCircle_new;

  buttonSquare_new = buttonSquare;
  if(buttonSquare_new == LOW && buttonSquare_old == HIGH) buttonSquare_released = true;
  else if(buttonSquare_new == HIGH && buttonSquare_old == LOW) buttonSquare_pressed = true;
  buttonSquare_old = buttonSquare_new;

  
};

boolean ESPController :: buttonReleased(uint8_t button) {
  switch(button) {
    case CIRCLE:
      if(buttonCircle_released == true) { buttonCircle_released = false; return(true); }
      else return(false);
      break;
    case SQUARE:
      if(buttonSquare_released == true) { buttonSquare_released = false; return(true); }
      else return(false);
      break;
    default:
      break;
  }
}

boolean ESPController :: buttonPressed(uint8_t button) {
  switch(button) {
    case CIRCLE:
      if(buttonCircle_pressed == true) { buttonCircle_pressed = false; return(true); }
      else return(false);
      break;
    case SQUARE:
      if(buttonSquare_pressed == true) { buttonSquare_pressed = false; return(true); }
      else return(false);
      break;
    
    default:
      break;
  }
}

#endif
