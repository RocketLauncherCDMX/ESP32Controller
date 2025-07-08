#include <Arduino.h>
#include "ESPController.h"

// Incluimos las cabeceras del RMT
#include <driver/rmt.h>
#include <soc/rmt_reg.h>

// ------------------------------------------------------------------------------------
// Variables estáticas/globales de RMT
// ------------------------------------------------------------------------------------
static rmt_item32_t led_data[48];          // Sustituye al antiguo rmt_data_t
static const rmt_channel_t RMT_CHANNEL = RMT_CHANNEL_0; 
// Puedes usar otro canal si RMT_CHANNEL_0 entra en conflicto con tu proyecto

// ------------------------------------------------------------------------------------
// Inicializar el canal RMT para enviar datos a WS2812 (Neopixels)
// Este reemplaza al antiguo rmtInit(...) y rmtSetTick(...)
// ------------------------------------------------------------------------------------
static void initRMT(int pin)
{
  rmt_config_t config = RMT_DEFAULT_CONFIG_TX((gpio_num_t)pin, RMT_CHANNEL);
  config.clk_div = 8; 
  config.tx_config.loop_en = false;
  config.tx_config.carrier_en = false;
  config.tx_config.idle_level = RMT_IDLE_LEVEL_LOW;
  config.tx_config.idle_output_en = true;

  rmt_config(&config);
  rmt_driver_install(config.channel, 0, 0);
}

// ------------------------------------------------------------------------------------
// Función para enviar los 48 items por RMT
// (equivalente a rmtWrite(rmt_send, led_data, 48);)
// ------------------------------------------------------------------------------------
static void sendPixels()
{
  // true = bloqueo hasta terminar,
  // si no quieres bloquear, pon false y luego rmt_wait_tx_done(...)
  rmt_write_items(RMT_CHANNEL, led_data, 48, true);
}

// ------------------------------------------------------------------------------------
// Implementación de métodos de ESPController
// ------------------------------------------------------------------------------------
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
  pinMode(NEOP_PIN, OUTPUT);
  pinMode(SHUTDOWN, OUTPUT);

  digitalWrite(SHUTDOWN, LOW);   // pin de apagado debe permanecer bajo = encendido.
  digitalWrite(RESET, HIGH);

  // Iniciar I2C
  Wire.begin(23,22);
  writeRegister(ADDRESS1, 2, 0xFF);
  writeRegister(ADDRESS2, 2, 0xFF);

  // Configurar joysticks
  JoystickLeft.side = LEFT;
  JoystickRight.side = RIGHT;
  JoystickLeft.calibrate();
  JoystickRight.calibrate();

  // Encender sin jugador
  Player(NO_PLAYER);

  // Nombrar botones
  Circle.name    = "circle";
  Square.name    = "square";
  Triangle.name  = "triangle";
  Cross.name     = "cross";
  Up.name        = "up";
  Down.name      = "down";
  Left.name      = "left";
  Right.name     = "right";
  Select.name    = "select";
  Start.name     = "start";
  P3.name        = "p3";
  R1.name        = "r1";
  R2.name        = "r2";
  R3.name        = "r3";
  L1.name        = "l1";
  L2.name        = "l2";
  L3.name        = "l3";

  // Neopixel side
  ledLeft.side = LEFT;
  ledRight.side = RIGHT;

  // Inicializa el canal RMT
  initRMT(NEOP_PIN);

  // Apagar neopixels
  Rgb.off();

  // Acelerometro 50 Hz, +-2 g, 12 bit, sin debug
  beginAccel_();
}

// ------------------------------------------------------------------------------------
void ESPController :: joystick :: calibrate() {
  horTotal = 0;
  verTotal = 0;
  for(uint8_t readings=0; readings<NUM_READINGS; readings++) {
    if(side == LEFT) {
      horReadings[readings] = 4095 - analogRead(LEFT_JOY_H);
      verReadings[readings] = 4095 - analogRead(LEFT_JOY_V);
    } else {
      horReadings[readings] = analogRead(RIGHT_JOY_H);
      verReadings[readings] = analogRead(RIGHT_JOY_V);
    }
    horTotal += horReadings[readings];
    verTotal += verReadings[readings];
  }

  y = horTotal / NUM_READINGS;
  x = verTotal / NUM_READINGS;
  horCentre = y;
  verCentre = x;
}

void ESPController :: joystick :: readRaw() {
  horTotal -= horReadings[readIndex];
  verTotal -= verReadings[readIndex];
  if(side == LEFT) {
    horReadings[readIndex] = 4095 - analogRead(LEFT_JOY_H);
    verReadings[readIndex] = 4095 - analogRead(LEFT_JOY_V);
  } else {
    horReadings[readIndex] = analogRead(RIGHT_JOY_H);
    verReadings[readIndex] = analogRead(RIGHT_JOY_V);
  }
  horTotal += horReadings[readIndex];
  verTotal += verReadings[readIndex];
    
  readIndex++;
  if (readIndex >= NUM_READINGS) readIndex = 0;

  // Promedio
  y = horTotal / NUM_READINGS;
  x = verTotal / NUM_READINGS;
}

void ESPController :: joystick :: readSign() {
  readRaw();
  if(y < horCentre) {
    y = map(y, 0, horCentre, -255, 0);
  } else if(y > horCentre) {
    y = map(y, horCentre, 4095, 0, 255);
  } else {
    y = 0; 
  }

  if(x < verCentre) {
    x = map(x, 0, verCentre, -255, 0);
  } else if(x > verCentre) {
    x = map(x, verCentre, 4095, 0, 255);
  } else {
    x = 0;
  }
}

// ------------------------------------------------------------------------------------
uint8_t ESPController :: readRegister(uint8_t address) {
  Wire.beginTransmission(address); 
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(address, 1);
  while(Wire.available() == 0);
  int c = Wire.read();   
  return c;
}

void ESPController :: writeRegister(uint8_t deviceAdd, uint8_t registerAdd, uint8_t data) {
  Wire.beginTransmission(deviceAdd); 
  Wire.write(registerAdd);
  Wire.write(data);
  Wire.endTransmission();
}

// ------------------------------------------------------------------------------------
void ESPController :: printButtonsBinary() {
  buttonsGroup1   = readRegister(ADDRESS1);
  buttonsGroup2   = readRegister(ADDRESS2);

  uint8_t b;
  uint8_t sizeOfByte = 8;
  char result[10];
  for (b=1<<(sizeof(buttonsGroup1)*sizeOfByte-1); b; b=b>>1) { 
    sprintf(result, "%c", '0'+((buttonsGroup1&b)!=0));
    Serial.print(result);
  }
  for (b=1<<(sizeof(buttonsGroup2)*sizeOfByte-1); b; b=b>>1) { 
    if(b == 128) Serial.print("X");
    else {
      sprintf(result, "%c", '0'+((buttonsGroup2&b)!=0));
      Serial.print(result);
    }
  }
  // L3 y R3:
  Serial.print(L3.newStatus   = !digitalRead(PUSH_LEFT));
  Serial.print(R3.newStatus   = !digitalRead(PUSH_RIGHT));
  Serial.println();
}

// ------------------------------------------------------------------------------------
void ESPController :: getAllButtons() {
  buttonsGroup1   = readRegister(ADDRESS1);
  buttonsGroup2   = readRegister(ADDRESS2);

  // Register 1
  R1.newStatus        = (buttonsGroup1 & 0b10000000) >> 7;
  Circle.newStatus    = (buttonsGroup1 & 0b01000000) >> 6;
  Square.newStatus    = (buttonsGroup1 & 0b00100000) >> 5;
  Start.newStatus     = (buttonsGroup1 & 0b00010000) >> 4;
  P3.newStatus        = (buttonsGroup1 & 0b00001000) >> 3;
  Down.newStatus      = (buttonsGroup1 & 0b00000100) >> 2;
  Up.newStatus        = (buttonsGroup1 & 0b00000010) >> 1;
  L2.newStatus        = (buttonsGroup1 & 0b00000001) >> 0;

  // Register 2
  R2.newStatus        = (buttonsGroup2 & 0b01000000) >> 6;
  Triangle.newStatus  = (buttonsGroup2 & 0b00100000) >> 5;
  Cross.newStatus     = (buttonsGroup2 & 0b00010000) >> 4;
  Select.newStatus    = (buttonsGroup2 & 0b00001000) >> 3;
  Right.newStatus     = (buttonsGroup2 & 0b00000100) >> 2;
  Left.newStatus      = (buttonsGroup2 & 0b00000010) >> 1;
  L1.newStatus        = (buttonsGroup2 & 0b00000001) >> 0;

  L3.newStatus   = !digitalRead(PUSH_LEFT);
  R3.newStatus   = !digitalRead(PUSH_RIGHT);

  // Actualizar estados y disparar eventos
  auto updateButton = [&](button &btn){
    if(btn.newStatus == PRESSED && btn.oldStatus == RELEASED) {
      btn.status = PRESSED; 
      buttonChanged = true; 
      buttonPressed = true; 
    }
    else if(btn.newStatus == RELEASED && btn.oldStatus == PRESSED) {
      btn.status = RELEASED; 
      buttonChanged = true; 
    }
    else if(btn.newStatus == PRESSED && btn.oldStatus == PRESSED) {
      btn.status = HOLD_PRESSED;
    }
    else {
      btn.status = HOLD_RELEASED;
    }
    btn.oldStatus = btn.newStatus;
  };

  updateButton(Circle);
  updateButton(Square);
  updateButton(Triangle);
  updateButton(Cross);
  updateButton(Up);
  updateButton(Down);
  updateButton(Left);
  updateButton(Right);
  updateButton(Select);
  updateButton(Start);
  updateButton(P3);
  updateButton(L1);
  updateButton(L2);
  updateButton(L3);
  updateButton(R1);
  updateButton(R2);
  updateButton(R3);
}

// ------------------------------------------------------------------------------------
void ESPController :: rumble() {
  // both
  digitalWrite(MOTOR_WEAK, HIGH);
  digitalWrite(MOTOR_STRONG, HIGH);
}

void ESPController :: rumble(bool strength) {
  // strength == WEAK o STRONG
  if(strength == WEAK) {
    digitalWrite(MOTOR_WEAK, HIGH);
  } else {
    digitalWrite(MOTOR_STRONG, HIGH);
  }
}

void ESPController :: stopRumble() {
  digitalWrite(MOTOR_WEAK, LOW);
  digitalWrite(MOTOR_STRONG, LOW);
}

void ESPController :: stopRumble(bool side) {
  side ? digitalWrite(MOTOR_STRONG, LOW) : digitalWrite(MOTOR_WEAK, LOW);
}


// ------------------------------------------------------------------------------------
void ESPController :: Player(uint8_t playerSel) {
  switch(playerSel) {
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

    // Ejemplo de efectos de “parpadeo” de los 4 leds
    case PLAYER_EFFECT1:
      switch(step) {
        case 0:
          Player(1);
          previousMillis = millis();
          step++;
          break;
        case 1:
          if (millis() - previousMillis >= 500) {
            Player(2);
            previousMillis = millis();
            step++;
          }
          break;
        case 2:
          if (millis() - previousMillis >= 50) {
            Player(3);
            previousMillis = millis();
            step++;
          }
          break;
        case 3:
          if (millis() - previousMillis >= 50) {
            Player(4);
            previousMillis = millis();
            step++;
          }
          break;
        case 4:
          if (millis() - previousMillis >= 300) {
            Player(3);
            previousMillis = millis();
            step++;
          }
          break;
        case 5:
          if (millis() - previousMillis >= 50) {
            Player(2);
            previousMillis = millis();
            step++;
          }
          break;
        case 6:
          if (millis() - previousMillis >= 50) {
            Player(1);
            previousMillis = millis();
            step = 0;
          }
          break;
      }
      break;
    case PLAYER_EFFECT2: {
      /*--------------------------------------------------------------------
        Estado interno del efecto (se mantiene entre iteraciones)
      --------------------------------------------------------------------*/
      static uint8_t  pattern      = 0b0000;   // Bits 0-3 → LED1-LED4 encendidos
      static uint8_t  latchedCount = 0;        // LEDs ya fijos (0-4)
      static uint8_t  pos          = 4;        // LED que se desplaza (4→1)
      static bool     filling      = true;     // true=llenado, false=vacío
      static bool     shiftPhase   = false;    // false=apagar LED1, true=shift
      static bool     inPause      = false;    // true durante las pausas de 1 s
      static uint32_t pauseStart   = 0;        // marca de tiempo para la pausa

      const uint16_t STEP_MS  = 100;           // intervalo entre acciones
      const uint16_t PAUSE_MS = 1000;          // pausa cuando está lleno o vacío
      uint32_t now = millis();

      /*------------------------- 1. GESTIÓN DE PAUSAS -------------------------*/
      if (inPause) {
        if (now - pauseStart >= PAUSE_MS) {    // terminó la pausa de 1 s
          inPause = false;
          if (filling) {                       // pasamos de llenado → vacío
            filling     = false;
            shiftPhase  = false;               // empezamos el ciclo apagar/shift
          } else {                             // terminamos el vacío → reinicio
            filling      = true;
            pattern      = 0b0000;
            latchedCount = 0;
            pos          = 4;
          }
          previousMillis = now;                // reinicia temporizador de paso
        }
        break;                                 // quedamos en pausa
      }

      /*--------------------- 2. ESPERA ENTRE ACCIONES ------------------------*/
      if (now - previousMillis < STEP_MS) break;
      previousMillis = now;

      /*------------------------- 3. FASE DE LLENADO ---------------------------*/
      if (filling) {

        uint8_t current = pattern | (1 << (pos - 1));   // mezcla LED fijo + móvil

        // Aplica a los pines físicos
        digitalWrite(LED1, (current & 0x01) ? HIGH : LOW);
        digitalWrite(LED2, (current & 0x02) ? HIGH : LOW);
        digitalWrite(LED3, (current & 0x04) ? HIGH : LOW);
        digitalWrite(LED4, (current & 0x08) ? HIGH : LOW);

        if (pos == latchedCount + 1) {         // LED móvil llegó al borde llenado
          pattern      = current;              // lo fijamos
          latchedCount++;
          if (latchedCount == 4) {             // ¡llenamos los 4 LEDs!
            inPause    = true;                 // pausa de 1 s
            pauseStart = now;
          } else {
            pos = 4;                           // reinicia corrimiento
          }
        } else {
          pos--;                               // avanza LED móvil 4→1
        }
      }

      /*------------------------- 4. FASE DE VACÍO ----------------------------*/
      else {

        if (!shiftPhase) {
          // 4.1  Apagamos LED1
          pattern &= 0b1110;                   // bit0 = 0
          shiftPhase = true;                   // siguiente paso será corrimiento
        } else {
          // 4.2  Corrimiento hacia LED1 (registro)
          pattern >>= 1;                       // bits 3→2→1→0
          shiftPhase = false;                  // siguiente paso apagar LED1
        }

        // Aplica el patrón resultante
        digitalWrite(LED1, (pattern & 0x01) ? HIGH : LOW);
        digitalWrite(LED2, (pattern & 0x02) ? HIGH : LOW);
        digitalWrite(LED3, (pattern & 0x04) ? HIGH : LOW);
        digitalWrite(LED4, (pattern & 0x08) ? HIGH : LOW);

        if (pattern == 0) {                    // Todos apagados → pausa final
          inPause    = true;
          pauseStart = now;
          // Al terminar la pausa se reiniciará al modo llenado
        }
      }
      break;
    }
    case PLAYER_EFFECT3: {
      static uint32_t lastUpdate = 0;        // marca de tiempo del último refresco
      const uint16_t  STEP_MS    = 80;       // periodo de actualización (80 ms)

      if (millis() - lastUpdate < STEP_MS) break;  // aún no toca refrescar
      lastUpdate = millis();

      /*-----------------------------------------
          Genera nivel 0‥4  → 0 = todo apagado,
          4 = los cuatro LEDs encendidos.
      ------------------------------------------*/
      uint8_t level = random(0, 5);          // 0,1,2,3,4 (inclusive)

      // Enciende o apaga cada LED según el nivel
      digitalWrite(LED1, (level >= 1) ? HIGH : LOW);   // LED1 = paso 1
      digitalWrite(LED2, (level >= 2) ? HIGH : LOW);   // LED2 = paso 2
      digitalWrite(LED3, (level >= 3) ? HIGH : LOW);   // LED3 = paso 3
      digitalWrite(LED4, (level >= 4) ? HIGH : LOW);   // LED4 = paso 4
      break;
    }
    case PLAYER_EFFECT4:
      if (millis() - previousMillis >= 75) {
        int rnd = random(1, 5);  // Aleatorio entre 1 y 4
        Player(rnd);
        previousMillis = millis();
      }
      break;
  }
}

// ------------------------------------------------------------------------------------
void ESPController :: off() {
  digitalWrite(SHUTDOWN, HIGH);   // MAX16054 goes disabled, which powers off the whole 3.3V rail.
}

// ------------------------------------------------------------------------------------
// Metodos de la clase pixels (controla AMBOS neopixels a la vez)
// ------------------------------------------------------------------------------------
void ESPController :: bothPixels :: color(uint8_t red, uint8_t green, uint8_t blue) {
  // value[] es estatico y global en ESPController.h
  value[0] = green;
  value[1] = red;
  value[2] = blue;
  value[3] = green;
  value[4] = red;
  value[5] = blue;
  i = 0;

  // Construye la señal RMT bit a bit
  for (col = 0; col < 6; col++ ) {
    for (bit = 0; bit < 8; bit++){
      if ( (value[col] & (1<<(7-bit))) ) {
        // bit = 1
        led_data[i].level0 = 1;
        led_data[i].duration0 = 8;
        led_data[i].level1 = 0;
        led_data[i].duration1 = 4;
      } else {
        // bit = 0
        led_data[i].level0 = 1;
        led_data[i].duration0 = 4;
        led_data[i].level1 = 0;
        led_data[i].duration1 = 8;
      }
      i++;
    }
  }
  // Enviar los 48 items via RMT
  sendPixels();
}

void ESPController :: bothPixels :: color(uint8_t colorName) {
  switch(colorName) {
    case OFF:       color(0,0,0);       break;
    case RED:       color(255,0,0);     break;
    case GREEN:     color(0,255,0);     break;
    case BLUE:      color(0,0,255);     break;
    case YELLOW:    color(255,255,0);   break;
    case CYAN:      color(0,255,255);   break;
    case MAGENTA:   color(255,0,255);   break;
    case WHITE:     color(255,255,255); break;
  }
}

void ESPController :: bothPixels :: fadeInOut(uint8_t colorName, uint8_t speed) {
  // Fade in
  for(int k=0; k<=255; k++) {
    switch(colorName) {
      case RED:     color(k,0,0); break;
      case GREEN:   color(0,k,0); break;
      case BLUE:    color(0,0,k); break;
      case YELLOW:  color(k,k,0); break;
      case CYAN:    color(0,k,k); break;
      case MAGENTA: color(k,0,k); break;
      case WHITE:   color(k,k,k); break;
    }
    delay(speed);
  }
  // Fade out
  for(int k=255; k>=0; k--) {
    switch(colorName) {
      case RED:     color(k,0,0); break;
      case GREEN:   color(0,k,0); break;
      case BLUE:    color(0,0,k); break;
      case YELLOW:  color(k,k,0); break;
      case CYAN:    color(0,k,k); break;
      case MAGENTA: color(k,0,k); break;
      case WHITE:   color(k,k,k); break;
    }
    delay(speed);
  }
}

void ESPController :: bothPixels :: fadeInOut(uint8_t colorName) {
  // Fade in
  for(int k=0; k<=255; k++) {
    switch(colorName) {
      case RED:     color(k,0,0); break;
      case GREEN:   color(0,k,0); break;
      case BLUE:    color(0,0,k); break;
      case YELLOW:  color(k,k,0); break;
      case CYAN:    color(0,k,k); break;
      case MAGENTA: color(k,0,k); break;
      case WHITE:   color(k,k,k); break;
    }
    delay(1);
  }
  // Fade out
  for(int k=255; k>=0; k--) {
    switch(colorName) {
      case RED:     color(k,0,0); break;
      case GREEN:   color(0,k,0); break;
      case BLUE:    color(0,0,k); break;
      case YELLOW:  color(k,k,0); break;
      case CYAN:    color(0,k,k); break;
      case MAGENTA: color(k,0,k); break;
      case WHITE:   color(k,k,k); break;
    }
    delay(1);
  }
}

void ESPController :: bothPixels :: off() {
  color(0,0,0);
}

// ------------------------------------------------------------------------------------
// Metodos de la clase pixel (controla UNO de los neopixels - izq o der)
// ------------------------------------------------------------------------------------
void ESPController :: pixel :: color(uint8_t red, uint8_t green, uint8_t blue) {
  // Si es el pixel derecho, ocupa los tres primeros bytes de value
  // Si es el pixel izquierdo, ocupa los 3 últimos.
  if(side == RIGHT) {
    value[0] = green;  // G
    value[1] = red;    // R
    value[2] = blue;   // B
  } else {
    value[3] = green;
    value[4] = red;
    value[5] = blue;
  }
  i = 0;
  // Montamos de nuevo los bits en led_data:
  for (col=0; col<6; col++ ) {
    for (bit=0; bit<8; bit++){
      if ( value[col] & (1<<(7-bit)) ) {
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
  // Enviar
  sendPixels();
}

void ESPController :: pixel :: color(uint8_t colorName) {
  switch(colorName) {
    case OFF:       color(0,0,0);       break;
    case RED:       color(255,0,0);     break;
    case GREEN:     color(0,255,0);     break;
    case BLUE:      color(0,0,255);     break;
    case YELLOW:    color(255,255,0);   break;
    case CYAN:      color(0,255,255);   break;
    case MAGENTA:   color(255,0,255);   break;
    case WHITE:     color(255,255,255); break;
  }
}

void ESPController :: pixel :: fadeInOut(uint8_t colorName, uint8_t speed) {
  for(int k=0; k<=255; k++) {
    switch(colorName) {
      case RED:     color(k,0,0); break;
      case GREEN:   color(0,k,0); break;
      case BLUE:    color(0,0,k); break;
      case YELLOW:  color(k,k,0); break;
      case CYAN:    color(0,k,k); break;
      case MAGENTA: color(k,0,k); break;
      case WHITE:   color(k,k,k); break;
    }
    delay(speed);
  }
  for(int k=255; k>=0; k--) {
    switch(colorName) {
      case RED:     color(k,0,0); break;
      case GREEN:   color(0,k,0); break;
      case BLUE:    color(0,0,k); break;
      case YELLOW:  color(k,k,0); break;
      case CYAN:    color(0,k,k); break;
      case MAGENTA: color(k,0,k); break;
      case WHITE:   color(k,k,k); break;
    }
    delay(speed);
  }
}

void ESPController :: pixel :: fadeInOut(uint8_t colorName) {
  for(int k=0; k<=255; k++) {
    switch(colorName) {
      case RED:     color(k,0,0); break;
      case GREEN:   color(0,k,0); break;
      case BLUE:    color(0,0,k); break;
      case YELLOW:  color(k,k,0); break;
      case CYAN:    color(0,k,k); break;
      case MAGENTA: color(k,0,k); break;
      case WHITE:   color(k,k,k); break;
    }
    delay(1);
  }
  for(int k=255; k>=0; k--) {
    switch(colorName) {
      case RED:     color(k,0,0); break;
      case GREEN:   color(0,k,0); break;
      case BLUE:    color(0,0,k); break;
      case YELLOW:  color(k,k,0); break;
      case CYAN:    color(0,k,k); break;
      case MAGENTA: color(k,0,k); break;
      case WHITE:   color(k,k,k); break;
    }
    delay(1);
  }
}

void ESPController :: pixel :: off() {
  color(0,0,0);
}

bool ESPController::beginAccel_(float sampleRate,
                                uint8_t range,
                                bool    highRes,
                                bool    debug)
{
  imuOK = (imu.begin(sampleRate, range, highRes, debug) == IMU_SUCCESS);

  if (imuOK) {
    imu.standby(false);                     //  Despierta el IMU

    // Activa la interrupcion de dato listo (DRDY) *sin* usar el pin fisico
    // threshold, moveDur, naDur = 0 - sin funcion wake-up
    // polarity = HIGH         - bit DRDY se pone a '1' cuando hay dato
    // wuRate  = -1            - ignora (solo vale para wake-up)
    // latched/pulsed/motion   - false
    // dataReady = true        - habilita DRDY
    // intPin    = false       - no queremos el pin INT fisico
    imu.intConf(0, 0, 0, HIGH, -1,
                false, false, false,
                true,  false);

    Wire.begin(23, 22);    // Asegura los pines SDA/SCL correctos
  }

  return imuOK;
}

ESPController::Accelerometer::Pos
ESPController::Accelerometer::detectPos() const
{
    if (!*ok) return POS_UNKNOWN;             // IMU no listo

    const float xg = imu->axisAccel(X);
    const float yg = imu->axisAccel(Y);
    const float zg = imu->axisAccel(Z);

    constexpr float MAIN_THR  = 0.8f;         // 80 %  ≈ 1 g ±20 %
    constexpr float SIDE_THR  = 0.4f;         // 40 %  zona “casi cero”

    /*--- Cara arriba ---------------------------------------------------*/
    if ( zg >  MAIN_THR && fabsf(xg) < SIDE_THR && fabsf(yg) < SIDE_THR )
        return POS_REST;

    /*--- Boca abajo ----------------------------------------------------*/
    if ( zg < -MAIN_THR && fabsf(xg) < SIDE_THR && fabsf(yg) < SIDE_THR )
        return POS_FACE_DOWN;

    /*--- Inclinaciones laterales --------------------------------------*/
    if ( yg >  MAIN_THR && fabsf(xg) < SIDE_THR && fabsf(zg) < SIDE_THR )
        return POS_RIGHT;                     // Giro 90° a la derecha

    if ( yg < -MAIN_THR && fabsf(xg) < SIDE_THR && fabsf(zg) < SIDE_THR )
        return POS_LEFT;                      // Giro 90° a la izquierda

    /*--- Pitch 90° -----------------------------------------------------*/
    if ( xg >  MAIN_THR && fabsf(yg) < SIDE_THR && fabsf(zg) < SIDE_THR )
        return POS_FRONT;                     // Gatillos hacia el suelo

    if ( xg < -MAIN_THR && fabsf(yg) < SIDE_THR && fabsf(zg) < SIDE_THR )
        return POS_FRONT_90;                  // Gatillos hacia el jugador

    return POS_UNKNOWN;                       // Fuera de tolerancia
}