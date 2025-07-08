/*****************************************************************
 *  ESP32-Controller
 *  -------------------------------------------------------------
 *  • Select          → cambia Player 1-4, lo guarda en NVS y reinicia.
 *  • Característica 1 (botones)      : 1 byte / PRESSED
 *  • Característica 2 (joysticks)    : 4 bytes / 200 Hz
 *  • Vibración       : tiempos y fuerza según tabla.
 *  • Neopixel        : destello BLANCO por NEO_BLINK_MS en cada botón.
 *  • LED Player      : parpadeo 2 Hz mientras anuncia, fijo al conectar.
 *****************************************************************/
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <Preferences.h>
#include "ESPController.h"

ESPController Controller;
Preferences    prefs;

/* ------------ TIEMPOS ------------ */
#define RUMBLE_WEAK_MS     40      // ms
#define RUMBLE_STRONG_MS   80      // ms
#define NEO_BLINK_MS       25      // ms
#define LED_BLINK_PERIOD  500      // ms → 2 Hz (250 ms ON + 250 ms OFF)

/* ------------ UUIDs BLE ---------- */
#define SVC_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define BTN_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define JOY_UUID "3ca7de79-4a16-4a62-b89d-136d2f020e4f"

/* ------------ BLE handles -------- */
BLECharacteristic *btnChar;
BLECharacteristic *joyChar;

/* ------------ Estado global ------ */
uint8_t playerID;                 // 1-4
bool    bleConnected   = false;   // estado de la conexión
bool    rumbleActive   = false;
uint32_t rumbleEnds    = 0;
bool    blinkActive    = false;
uint32_t blinkEnds     = 0;
uint32_t ledToggleT0   = 0;
bool    ledOn          = false;   // para parpadeo

/* ------------ Botones ------------ */
struct BtnMap {
  ESPController::button* b;
  char  code;
  bool  weak;                     // true = WEAK, false = STRONG
} btnMap[] = {
  /* WEAK */
  { &Controller.Up,    'U', true }, { &Controller.Down,  'D', true },
  { &Controller.Left,  'L', true }, { &Controller.Right, 'R', true },
  { &Controller.L1,    '1', true }, { &Controller.L3,    '3', true },
  { &Controller.R1,    '4', true }, { &Controller.R3,    '6', true },
  /* STRONG */
  { &Controller.Circle,'C', false },{ &Controller.Square,'Q', false },
  { &Controller.Triangle,'T',false},{ &Controller.Cross, 'X', false },
  { &Controller.Start, 'S', false },{ &Controller.P3,    'P', false },
  { &Controller.L2,    '2', false },{ &Controller.R2,    '5', false }
};
const uint8_t N_BTN = sizeof(btnMap) / sizeof(btnMap[0]);

/* ---------- Callbacks BLE -------- */
class ServerCB : public BLEServerCallbacks {
  void onConnect(BLEServer*) override {
    bleConnected = true;
    Controller.Player(playerID);        // LED fijo encendido
  }
  void onDisconnect(BLEServer* srv) override {
    bleConnected = false;
    ledToggleT0 = millis();             // reinicia parpadeo
    srv->getAdvertising()->start();     // vuelve a anunciar
  }
};

/* ---------- Prototipo ------------ */
void startBLE();

/* ================================================================ */
void setup() {
  Serial.begin(115200);
  Controller.begin();

  /* ---- Player almacenado en NVS ---- */
  prefs.begin("ctrl", false);
  playerID = prefs.getUChar("player", 1);
  if (playerID < 1 || playerID > 4) playerID = 1;
  Controller.Player(NO_PLAYER);         // apagado al inicio

  startBLE();
}

/* ================================================================ */
void loop() {
  Controller.getAllButtons();

  /* -------- SELECT: cambia Player y reinicia -------- */
  if (Controller.Select.status == PRESSED) {
    playerID = (playerID % 4) + 1;              // 1→2→3→4→1…
    prefs.putUChar("player", playerID);
    prefs.end();
    delay(50);
    ESP.restart();
  }

  /* -------- Resto de botones -------- */
  for (uint8_t i = 0; i < N_BTN; ++i) {
    if (btnMap[i].b->status == PRESSED) {
      /* 1) Notificación BLE */
      btnChar->setValue((uint8_t*)&btnMap[i].code, 1);
      btnChar->notify();

      /* 2) Vibración */
      bool weak = btnMap[i].weak;
      Controller.rumble(weak ? WEAK : STRONG);
      rumbleActive = true;
      rumbleEnds   = millis() + (weak ? RUMBLE_WEAK_MS : RUMBLE_STRONG_MS);

      /* 3) Parpadeo Neopixel blanco */
      Controller.Rgb.color(WHITE);
      blinkActive = true;
      blinkEnds   = millis() + NEO_BLINK_MS;
    }
  }

  /* -------- Joysticks a 200 Hz -------- */
  static uint32_t tJoy = 0;
  if (millis() - tJoy >= 5) {
    Controller.JoystickLeft.readSign();
    Controller.JoystickRight.readSign();

    int8_t lx = Controller.JoystickLeft.x  / 2;
    int8_t ly = Controller.JoystickLeft.y  / 2;
    int8_t rx = Controller.JoystickRight.x / 2;
    int8_t ry = Controller.JoystickRight.y / 2;

    uint8_t pkt[4] = { (uint8_t)lx, (uint8_t)ly, (uint8_t)rx, (uint8_t)ry };
    joyChar->setValue(pkt, 4);
    joyChar->notify();
    tJoy = millis();
  }

  /* -------- Fin de vibración -------- */
  if (rumbleActive && millis() >= rumbleEnds) {
    Controller.stopRumble();
    rumbleActive = false;
  }

  /* -------- Fin de destello Neopixel -------- */
  if (blinkActive && millis() >= blinkEnds) {
    Controller.Rgb.off();
    blinkActive = false;
  }

  /* -------- Gestión LED Player -------- */
  if (!bleConnected) {                              // anunciando
    if (millis() - ledToggleT0 >= LED_BLINK_PERIOD / 2) {
      ledToggleT0 = millis();
      ledOn = !ledOn;
      Controller.Player(ledOn ? playerID : NO_PLAYER);
    }
  }
}

/* =================  BLE SETUP  ================= */
void startBLE() {
  char name[24];
  snprintf(name, sizeof(name), "ESP32 Controller %u", playerID);
  BLEDevice::init(name);

  BLEServer *srv = BLEDevice::createServer();
  srv->setCallbacks(new ServerCB());

  BLEService *svc = srv->createService(SVC_UUID);

  btnChar = svc->createCharacteristic(
              BTN_UUID, BLECharacteristic::PROPERTY_READ |
                        BLECharacteristic::PROPERTY_NOTIFY);
  btnChar->addDescriptor(new BLE2902());

  joyChar = svc->createCharacteristic(
              JOY_UUID, BLECharacteristic::PROPERTY_READ |
                        BLECharacteristic::PROPERTY_NOTIFY);
  joyChar->addDescriptor(new BLE2902());

  svc->start();

  BLEAdvertising *adv = BLEDevice::getAdvertising();
  adv->addServiceUUID(SVC_UUID);
  adv->setScanResponse(true);
  adv->setMinPreferred(0x06);
  adv->setMinPreferred(0x12);
  adv->start();

  ledToggleT0 = millis();   // inicia parpadeo
  Serial.printf("BLE \"%s\" anunciándose (Player %u)\n", name, playerID);
}
