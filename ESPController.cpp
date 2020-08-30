#include <Arduino.h>
#include "ESPController.h"

#ifndef _ESPCONTROLLER_CPP_
#define _ESPCONTROLLER_CPP_

rmt_data_t led_data[24];
rmt_obj_t* rmt_send = NULL;

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

  Player(NO_PLAYER);

  Circle.name = "circle";
  Square.name = "square";
  Triangle.name = "triangle";
  Cross.name = "cross";
  Up.name = "up";
  Down.name = "down";
  Left.name = "left";
  Right.name = "right";
  Select.name = "select";
  Start.name = "start";
  Ps.name = "ps";
  R1.name = "r1";
  R2.name = "r2";
  L1.name = "l1";
  L2.name = "l2";
  JoyLeft.name = "joystick left";
  JoyRight.name = "joystick right";

  rmt_send = rmtInit(18, true, RMT_MEM_64);
  rmtSetTick(rmt_send, 100);
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
  //put the byte in variable c
  int c = Wire.read();   
  return c;
}

void ESPController :: getButtons() {
  buttonsGroup1   = readRegister(ADDRESS1);
  buttonsGroup2   = readRegister(ADDRESS2);

  // Register 1
  R1.newStatus        = (buttonsGroup1 & 0b10000000) >> 7;
  Circle.newStatus    = (buttonsGroup1 & 0b01000000) >> 6;
  Square.newStatus    = (buttonsGroup1 & 0b00100000) >> 5;
  Start.newStatus     = (buttonsGroup1 & 0b00010000) >> 4;
  Ps.newStatus        = !((buttonsGroup1 & 0b00001000) >> 3);
  Down.newStatus      = !((buttonsGroup1 & 0b00000100) >> 2);
  Up.newStatus        = !((buttonsGroup1 & 0b00000010) >> 1);
  L2.newStatus        = !((buttonsGroup1 & 0b00000001) >> 0);

  // Register 2
  R2.newStatus        = (buttonsGroup2 & 0b01000000) >> 6;
  Triangle.newStatus  = (buttonsGroup2 & 0b00100000) >> 5;
  Cross.newStatus     = (buttonsGroup2 & 0b00010000) >> 4;
  Select.newStatus    = !((buttonsGroup2 & 0b00001000) >> 3);
  Right.newStatus     = !((buttonsGroup2 & 0b00000100) >> 2);
  Left.newStatus      = !((buttonsGroup2 & 0b00000010) >> 1);
  L1 .newStatus       = !((buttonsGroup2 & 0b00000001) >> 0);

  JoyLeft.newStatus   = !digitalRead(PUSH_LEFT);
  JoyRight.newStatus   = !digitalRead(PUSH_RIGHT);

  if(Circle.newStatus == PRESSED && Circle.oldStatus == RELEASED) { Circle.status = PRESSED; buttonChanged = true; buttonPressed = true; }
  else if(Circle.newStatus == RELEASED && Circle.oldStatus == PRESSED) { Circle.status = RELEASED; buttonChanged = true; }
  else if(Circle.newStatus == PRESSED && Circle.oldStatus == PRESSED) Circle.status = HOLD_PRESSED;
  else if(Circle.newStatus == RELEASED && Circle.oldStatus == RELEASED) Circle.status = HOLD_RELEASED;
  Circle.oldStatus = Circle.newStatus;

  if(Square.newStatus == PRESSED && Square.oldStatus == RELEASED) { Square.status = PRESSED; buttonChanged = true; buttonPressed = true; }
  else if(Square.newStatus == RELEASED && Square.oldStatus == PRESSED) { Square.status = RELEASED; buttonChanged = true; }
  else if(Square.newStatus == PRESSED && Square.oldStatus == PRESSED) Square.status = HOLD_PRESSED;
  else if(Square.newStatus == RELEASED && Square.oldStatus == RELEASED) Square.status = HOLD_RELEASED;
  Square.oldStatus = Square.newStatus;

  if(Triangle.newStatus == PRESSED && Triangle.oldStatus == RELEASED) { Triangle.status = PRESSED; buttonChanged = true; buttonPressed = true; }
  else if(Triangle.newStatus == RELEASED && Triangle.oldStatus == PRESSED) { Triangle.status = RELEASED; buttonChanged = true; }
  else if(Triangle.newStatus == PRESSED && Triangle.oldStatus == PRESSED) Triangle.status = HOLD_PRESSED;
  else if(Triangle.newStatus == RELEASED && Triangle.oldStatus == RELEASED) Triangle.status = HOLD_RELEASED;
  Triangle.oldStatus = Triangle.newStatus;

  if(Cross.newStatus == PRESSED && Cross.oldStatus == RELEASED) { Cross.status = PRESSED; buttonChanged = true; buttonPressed = true; }
  else if(Cross.newStatus == RELEASED && Cross.oldStatus == PRESSED) { Cross.status = RELEASED; buttonChanged = true; }
  else if(Cross.newStatus == PRESSED && Cross.oldStatus == PRESSED) Cross.status = HOLD_PRESSED;
  else if(Cross.newStatus == RELEASED && Cross.oldStatus == RELEASED) Cross.status = HOLD_RELEASED;
  Cross.oldStatus = Cross.newStatus;

  if(Up.newStatus == PRESSED && Up.oldStatus == RELEASED) { Up.status = PRESSED; buttonChanged = true; buttonPressed = true; }
  else if(Up.newStatus == RELEASED && Up.oldStatus == PRESSED) { Up.status = RELEASED; buttonChanged = true; }
  else if(Up.newStatus == PRESSED && Up.oldStatus == PRESSED) Up.status = HOLD_PRESSED;
  else if(Up.newStatus == RELEASED && Up.oldStatus == RELEASED) Up.status = HOLD_RELEASED;
  Up.oldStatus = Up.newStatus;

  if(Down.newStatus == PRESSED && Down.oldStatus == RELEASED) { Down.status = PRESSED; buttonChanged = true; buttonPressed = true; }
  else if(Down.newStatus == RELEASED && Down.oldStatus == PRESSED) { Down.status = RELEASED; buttonChanged = true; }
  else if(Down.newStatus == PRESSED && Down.oldStatus == PRESSED) Down.status = HOLD_PRESSED;
  else if(Down.newStatus == RELEASED && Down.oldStatus == RELEASED) Down.status = HOLD_RELEASED;
  Down.oldStatus = Down.newStatus;

  if(Left.newStatus == PRESSED && Left.oldStatus == RELEASED) { Left.status = PRESSED; buttonChanged = true; buttonPressed = true; }
  else if(Left.newStatus == RELEASED && Left.oldStatus == PRESSED) { Left.status = RELEASED; buttonChanged = true; }
  else if(Left.newStatus == PRESSED && Left.oldStatus == PRESSED) Left.status = HOLD_PRESSED;
  else if(Left.newStatus == RELEASED && Left.oldStatus == RELEASED) Left.status = HOLD_RELEASED;
  Left.oldStatus = Left.newStatus;

  if(Right.newStatus == PRESSED && Right.oldStatus == RELEASED) { Right.status = PRESSED; buttonChanged = true; buttonPressed = true; }
  else if(Right.newStatus == RELEASED && Right.oldStatus == PRESSED) { Right.status = RELEASED; buttonChanged = true; }
  else if(Right.newStatus == PRESSED && Right.oldStatus == PRESSED) Right.status = HOLD_PRESSED;
  else if(Right.newStatus == RELEASED && Right.oldStatus == RELEASED) Right.status = HOLD_RELEASED;
  Right.oldStatus = Right.newStatus;  

  if(Select.newStatus == PRESSED && Select.oldStatus == RELEASED) { Select.status = PRESSED; buttonChanged = true; buttonPressed = true; }
  else if(Select.newStatus == RELEASED && Select.oldStatus == PRESSED) { Select.status = RELEASED; buttonChanged = true; }
  else if(Select.newStatus == PRESSED && Select.oldStatus == PRESSED) Select.status = HOLD_PRESSED;
  else if(Select.newStatus == RELEASED && Select.oldStatus == RELEASED) Select.status = HOLD_RELEASED;
  Select.oldStatus = Select.newStatus;

  if(Start.newStatus == PRESSED && Start.oldStatus == RELEASED) { Start.status = PRESSED; buttonChanged = true; buttonPressed = true; }
  else if(Start.newStatus == RELEASED && Start.oldStatus == PRESSED) { Start.status = RELEASED; buttonChanged = true; }
  else if(Start.newStatus == PRESSED && Start.oldStatus == PRESSED) Start.status = HOLD_PRESSED;
  else if(Start.newStatus == RELEASED && Start.oldStatus == RELEASED) Start.status = HOLD_RELEASED;
  Start.oldStatus = Start.newStatus;

  if(Ps.newStatus == PRESSED && Ps.oldStatus == RELEASED) { Ps.status = PRESSED; buttonChanged = true; buttonPressed = true; }
  else if(Ps.newStatus == RELEASED && Ps.oldStatus == PRESSED) { Ps.status = RELEASED; buttonChanged = true; }
  else if(Ps.newStatus == PRESSED && Ps.oldStatus == PRESSED) Ps.status = HOLD_PRESSED;
  else if(Ps.newStatus == RELEASED && Ps.oldStatus == RELEASED) Ps.status = HOLD_RELEASED;
  Ps.oldStatus = Right.newStatus;  

  if(L1.newStatus == PRESSED && L1.oldStatus == RELEASED) { L1.status = PRESSED; buttonChanged = true; buttonPressed = true; }
  else if(L1.newStatus == RELEASED && L1.oldStatus == PRESSED) { L1.status = RELEASED; buttonChanged = true; }
  else if(L1.newStatus == PRESSED && L1.oldStatus == PRESSED) L1.status = HOLD_PRESSED;
  else if(L1.newStatus == RELEASED && L1.oldStatus == RELEASED) L1.status = HOLD_RELEASED;
  L1.oldStatus = L1.newStatus;

  if(L2.newStatus == PRESSED && L2.oldStatus == RELEASED) { L2.status = PRESSED; buttonChanged = true; buttonPressed = true; }
  else if(L2.newStatus == RELEASED && L2.oldStatus == PRESSED) { L2.status = RELEASED; buttonChanged = true; } 
  else if(L2.newStatus == PRESSED && L2.oldStatus == PRESSED) L2.status = HOLD_PRESSED;
  else if(L2.newStatus == RELEASED && L2.oldStatus == RELEASED) L2.status = HOLD_RELEASED;
  L2.oldStatus = L2.newStatus;

  if(R1.newStatus == PRESSED && R1.oldStatus == RELEASED) { R1.status = PRESSED; buttonChanged = true; buttonPressed = true; }
  else if(R1.newStatus == RELEASED && R1.oldStatus == PRESSED) { R1.status = RELEASED; buttonChanged = true; }
  else if(R1.newStatus == PRESSED && R1.oldStatus == PRESSED) R1.status = HOLD_PRESSED;
  else if(R1.newStatus == RELEASED && R1.oldStatus == RELEASED) R1.status = HOLD_RELEASED;
  R1.oldStatus = R1.newStatus;

  if(R2.newStatus == PRESSED && R2.oldStatus == RELEASED) { R2.status = PRESSED; buttonChanged = true; buttonPressed = true; }
  else if(R2.newStatus == RELEASED && R2.oldStatus == PRESSED) { R2.status = RELEASED; buttonChanged = true; }
  else if(R2.newStatus == PRESSED && R2.oldStatus == PRESSED) R2.status = HOLD_PRESSED;
  else if(R2.newStatus == RELEASED && R2.oldStatus == RELEASED) R2.status = HOLD_RELEASED;
  R2.oldStatus = R2.newStatus;

  if(JoyLeft.newStatus == PRESSED && JoyLeft.oldStatus == RELEASED) { JoyLeft.status = PRESSED; buttonChanged = true; buttonPressed = true; }
  else if(JoyLeft.newStatus == RELEASED && JoyLeft.oldStatus == PRESSED) { JoyLeft.status = RELEASED; buttonChanged = true; }
  else if(JoyLeft.newStatus == PRESSED && JoyLeft.oldStatus == PRESSED) JoyLeft.status = HOLD_PRESSED;
  else if(JoyLeft.newStatus == RELEASED && JoyLeft.oldStatus == RELEASED) JoyLeft.status = HOLD_RELEASED;
  JoyLeft.oldStatus = JoyLeft.newStatus;

  if(JoyRight.newStatus == PRESSED && JoyRight.oldStatus == RELEASED) { JoyRight.status = PRESSED; buttonChanged = true; buttonPressed = true; }
  else if(JoyRight.newStatus == RELEASED && JoyRight.oldStatus == PRESSED) { JoyRight.status = RELEASED; buttonChanged = true; }
  else if(JoyRight.newStatus == PRESSED && JoyRight.oldStatus == PRESSED) JoyRight.status = HOLD_PRESSED;
  else if(JoyRight.newStatus == RELEASED && JoyRight.oldStatus == RELEASED) JoyRight.status = HOLD_RELEASED;
  JoyRight.oldStatus = JoyRight.newStatus;
};

void ESPController :: vibrate(boolean strengh, uint16_t time) {
  if(strengh == WEAK) {
    digitalWrite(MOTOR_WEAK, HIGH);
    delay(time);
    digitalWrite(MOTOR_WEAK, LOW);
    delay(time);
  }
  else if(strengh == STRONG) {
    digitalWrite(MOTOR_STRONG, HIGH);
    delay(time);
    digitalWrite(MOTOR_STRONG, LOW);
    delay(time);
  }
}

void ESPController :: Player(uint8_t player) {
  switch(player) {
    case NO_PLAYER:
      digitalWrite(LED1, LOW);
      digitalWrite(LED2, LOW);
      digitalWrite(LED3, LOW);
      digitalWrite(LED4, LOW);
      player = NO_PLAYER;
      break;
    case 1:
      digitalWrite(LED1, HIGH);
      digitalWrite(LED2, LOW);
      digitalWrite(LED3, LOW);
      digitalWrite(LED4, LOW);
      player = PLAYER1;
      break;
    case 2:
      digitalWrite(LED1, LOW);
      digitalWrite(LED2, HIGH);
      digitalWrite(LED3, LOW);
      digitalWrite(LED4, LOW);
      player = PLAYER2;
      break;
    case 3:
      digitalWrite(LED1, LOW);
      digitalWrite(LED2, LOW);
      digitalWrite(LED3, HIGH);
      digitalWrite(LED4, LOW);
      player = PLAYER3;
      break;
    case 4:
      digitalWrite(LED1, LOW);
      digitalWrite(LED2, LOW);
      digitalWrite(LED3, LOW);
      digitalWrite(LED4, HIGH);
      player = PLAYER4;
      break;
  }
}

void ESPController :: pixel :: color(uint8_t red, uint8_t green, uint8_t blue) {
  value[0] = green;
  value[1] = red;
  value[2] = blue;
  i=0;

  for (col=0; col<3; col++ ) {
    for (bit=0; bit<8; bit++){
      if ( (value[col] & (1<<(7-bit)))) {
        led_data[i].level0 = 1;
        led_data[i].duration0 = 8;
        led_data[i].level1 = 0;
        led_data[i].duration1 = 4;
      } else {
        led_data[i].level0 = 1;
        led_data[i].duration0 = 4;
        led_data[i].level1 = 0;
        led_data[i].duration1 = 8;
      }
      i++;
    }
  }
  // Send the data
  rmtWrite(rmt_send, led_data, 24);
}

void ESPController :: pixel :: color(uint8_t colorName) {
  switch(colorName) {
    case OFF:       color(0,0,0);        break;
    case RED:       color(255,0,0);      break;
    case GREEN:     color(0,255,0);      break;
    case BLUE:      color(0,0,255);      break;
    case YELLOW:    color(255,255,0);    break;
    case CYAN:      color(0,255,255);    break;
    case MAGENTA:   color(255,0,255);    break;
    case WHITE:     color(255,255,255);  break;
  }
}

void ESPController :: pixel :: fadeInOut(uint8_t colorName, uint8_t speed) {
  for(int i=0; i<=255; i++) {
    switch(colorName) {
      case RED:     color(i,0,0); break;
      case GREEN:   color(0,i,0); break;
      case BLUE:    color(0,0,i); break;
      case YELLOW:  color(i,i,0); break;
      case CYAN:    color(0,i,i); break;
      case MAGENTA: color(i,0,i); break;
      case WHITE:   color(i,i,i); break;
    }
    delay(speed);
  }
  for(int i=255; i>=0; i--) {
    switch(colorName) {
      case RED:     color(i,0,0); break;
      case GREEN:   color(0,i,0); break;
      case BLUE:    color(0,0,i); break;
      case YELLOW:  color(i,i,0); break;
      case CYAN:    color(0,i,i); break;
      case MAGENTA: color(i,0,i); break;
      case WHITE:   color(i,i,i); break;
    }
    delay(speed);
  }
}

void ESPController :: pixel :: fadeInOut(uint8_t colorName) {
  for(int i=0; i<=255; i++) {
    switch(colorName) {
      case RED:     color(i,0,0); break;
      case GREEN:   color(0,i,0); break;
      case BLUE:    color(0,0,i); break;
      case YELLOW:  color(i,i,0); break;
      case CYAN:    color(0,i,i); break;
      case MAGENTA: color(i,0,i); break;
      case WHITE:   color(i,i,i); break;
    }
    delay(1);
  }
  for(int i=255; i>=0; i--) {
    switch(colorName) {
      case RED:     color(i,0,0); break;
      case GREEN:   color(0,i,0); break;
      case BLUE:    color(0,0,i); break;
      case YELLOW:  color(i,i,0); break;
      case CYAN:    color(0,i,i); break;
      case MAGENTA: color(i,0,i); break;
      case WHITE:   color(i,i,i); break;
    }
    delay(1);
  }
}

void ESPController :: pixel :: off() {
  color(0,0,0);
}

#endif
