/**********************************************************************
  BLE SERVER – Detección sencilla de PRESSED / RELEASED, sin bit-trucos
**********************************************************************/
#include <BLEDevice.h>
#include <BLEServer.h>

/* ---------- UUIDs ---------- */
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

/* ---------- Botones que vamos a usar aquí ---------- */
bool btnCross      = false;   // estado actual
bool btnCircle     = false;
bool btnSquare     = false;
bool btnTriangle   = false;

bool prevCross     = false;   // estado anterior (para detectar cambios)
bool prevCircle    = false;
bool prevSquare    = false;
bool prevTriangle  = false;

/* ---------- Función auxiliar: lee un bit ---------- */
inline bool bitIsSet(uint16_t value, uint8_t bit) {
  return (value >> bit) & 0x01;
}

/* ---------- Callback BLE ---------- */
class RxCB : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* ch) override {

    String v = ch->getValue();              // 2 bytes enviados por el cliente
    if (v.length() != 2) return;

    /* Little-endian → 1º byte = LSB (bits 0-7), 2º byte = MSB (bits 8-15) */
    uint16_t raw =  uint8_t(v[0]) | (uint8_t(v[1]) << 8);

    /* -------------- ACTUALIZA los estados actuales -------------- */
    btnCross    = bitIsSet(raw,  4);   // 1 = PRESSED, 0 = RELEASED
    btnTriangle = bitIsSet(raw,  5);
    btnSquare   = bitIsSet(raw, 13);
    btnCircle   = bitIsSet(raw, 14);

    /* -------------- (opcional) imprime el paquete completo ------ */
    for (int i = 15; i >= 0; --i) Serial.print(bitIsSet(raw, i));
    Serial.println();
  }
};

void setup() {
  Serial.begin(115200);
  BLEDevice::init("ble server");

  /* BLE boilerplate */
  auto srv = BLEDevice::createServer();
  auto svc = srv->createService(SERVICE_UUID);

  auto chr = svc->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ  |
      BLECharacteristic::PROPERTY_WRITE |
      BLECharacteristic::PROPERTY_NOTIFY);
  chr->setCallbacks(new RxCB());
  chr->setValue("0");                // valor inicial
  svc->start();

  auto adv = BLEDevice::getAdvertising();
  adv->addServiceUUID(SERVICE_UUID);
  adv->setScanResponse(true);
  BLEDevice::startAdvertising();

  Serial.println("Servidor listo (modo principiante)");
}

void loop() {
  /* ----------- Cruz (X) ----------- */
  if (btnCross && !prevCross) {            // ACABA DE PRESIONARSE
    Serial.print("X");
  }
  if (!btnCross && prevCross) {            // ACABA DE SOLTARSE
    Serial.print("x");                     // o pon lo que prefieras
  }
  prevCross = btnCross;                    // guarda estado para la próxima vuelta

  /* ----------- Círculo (○) --------- */
  if (btnCircle && !prevCircle) {
    Serial.print("○");
  }
  if (!btnCircle && prevCircle) {
    Serial.print("o");
  }
  prevCircle = btnCircle;

  /* ----------- Cuadrado (☐) -------- */
  if (btnSquare && !prevSquare) {
    Serial.print("☐");
  }
  if (!btnSquare && prevSquare) {
    Serial.print("□");
  }
  prevSquare = btnSquare;

  /* ----------- Triángulo (△) ------- */
  if (btnTriangle && !prevTriangle) {
    Serial.print("△");
  }
  if (!btnTriangle && prevTriangle) {
    Serial.print("▲");
  }
  prevTriangle = btnTriangle;

  /* Pequeña pausa para no saturar el puerto serie */
  delay(5);
}
