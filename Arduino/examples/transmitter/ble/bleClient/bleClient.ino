/**********************************************************************
  BLE CLIENT – Envía 16 bits con los botones y usa los LEDs de “player”
               para indicar estado BLE
**********************************************************************/
#include <BLEDevice.h>
#include "ESPController.h"

/* --- Ajustes BLE --- */
static const char* targetName = "ble server";
static BLEUUID serviceUUID("4fafc201-1fb5-459e-8fcc-c5c9c331914b");
static BLEUUID charUUID   ("beb5483e-36e1-4688-b7f5-ea07361b26a8");

ESPController Pad;

/* Estado de conexión */
BLERemoteCharacteristic* pChar = nullptr;
BLEAdvertisedDevice*      dev  = nullptr;
bool doConnect=false, connected=false, doScan=false;

/* ---------------- Conexión ---------------- */
class CliCB : public BLEClientCallbacks {
  void onConnect   (BLEClient*) override {}
  void onDisconnect(BLEClient*) override { connected=false; }
};
bool connectServer() {
  BLEClient* cli = BLEDevice::createClient();
  cli->setClientCallbacks(new CliCB());
  if (!cli->connect(dev)) return false;

  auto svc = cli->getService(serviceUUID);     if (!svc) return false;
  pChar = svc->getCharacteristic(charUUID);    if (!pChar) { cli->disconnect(); return false; }

  connected = true;
  Pad.Player(1);                               // LED1 ON  (conectado)
  return true;
}

/* ------------- Escaneo ------------- */
class ScanCB : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice d) override {
    if (d.haveName() && d.getName()==targetName) {
      BLEDevice::getScan()->stop();
      dev = new BLEAdvertisedDevice(d);
      doConnect = true;  doScan = true;
    }
  }
};

/* -------- Construir palabra de 16 bits usando newStatus (0/1) -------- */
#define B(x) ((x.newStatus)?1:0)    // 1 = PRESSED

uint16_t buildState() {
  Pad.getAllButtons();
  uint16_t s = 0;
  /* bits 8-15 */
  s |=  B(Pad.L2)      <<  8;
  s |=  B(Pad.Up)      <<  9;
  s |=  B(Pad.Down)    << 10;
  s |=  B(Pad.P3)      << 11;
  s |=  B(Pad.Start)   << 12;
  s |=  B(Pad.Square)  << 13;
  s |=  B(Pad.Circle)  << 14;
  s |=  B(Pad.R1)      << 15;
  /* bits 0-6 */
  s |=  B(Pad.L1)      << 0;
  s |=  B(Pad.Left)    << 1;
  s |=  B(Pad.Right)   << 2;
  s |=  B(Pad.Select)  << 3;
  s |=  B(Pad.Cross)   << 4;
  s |=  B(Pad.Triangle)<< 5;
  s |=  B(Pad.R2)      << 6;        // bit 7 queda 0
  return s;
}

/* ---------------- SETUP ---------------- */
void setup() {
  Serial.begin(115200);
  Pad.begin();
  Pad.Player(PLAYER_EFFECT1);               // efecto “buscando”

  BLEDevice::init("");
  auto s = BLEDevice::getScan();
  s->setAdvertisedDeviceCallbacks(new ScanCB());
  s->setActiveScan(true);
  s->start(5, nullptr, false);
}

/* ---------------- LOOP ----------------- */
void loop() {
  /* Gestión de conexión */
  if (doConnect) { connectServer();  doConnect = false; }

  /* LEDs cuando NO estamos conectados */
  if (!connected) Pad.Player(PLAYER_EFFECT1);     // mantiene el efecto

  /* Envío de botones */
  static uint16_t prev = 0xFFFF;
  uint16_t now = buildState();
  if (connected && now != prev) {
    uint8_t p[2] = { uint8_t(now), uint8_t(now>>8) };
    pChar->writeValue(p, 2, false);
    prev = now;
  }
  else if (!connected && doScan) {
    BLEDevice::getScan()->start(0, nullptr, false);
  }
  delay(10);
}
