/**************************************************************************
 *  ESP32 Controller – RECEPTOR BLE 
 *
 *  Ejemplo de uso libre que muestra cómo recibir TODOS los datos
 *  del ESP32 Controller por Bluetooth (botones, joysticks,
 *  inclinación) y dejarlos listos en funciones simples.
 *
 *  Autor : Rocket Launcher
 *  Fecha : 07-Jul-2025
 *  Codigo: https://github.com/RocketLauncherCDMX/ESP32Controller
 *  
 *  --------------------------------------------------------------------
 *  
 *  Formato de la línea de depuración (separado por tabuladores):
 *     LX   LY   RX   RY   lastBtn   dir-lvl
 *
 *  *** DÓNDE ESCRIBIR TU CÓDIGO ***
 *
 *  1) BOTONES → Dentro de cada onButtonXxx()
 *     Ejemplo: encender un LED al pulsar Círculo
 *
 *       void onButtonCircle() {
 *         digitalWrite(LED_BUILTIN, HIGH);
 *       }
 *
 *  2) JOYSTICK IZQUIERDO → onJoystickLeft(int8_t x, int8_t y)
 *     JOYSTICK DERECHO   → onJoystickRight(int8_t x, int8_t y)
 *
 *     Los valores van de −128 a +127.
 *     Ejemplo: variar el brillo de un LED con el eje X del joystick izquierdo
 *
 *       void onJoystickLeft(int8_t x, int8_t y) {
 *         int brillo = map(x, -128, 127, 0, 255);
 *         analogWrite(LED_PIN, brillo);
 *       }
 *
 *  3) INCLINACIÓN → onInclination(char dir, uint8_t level)
 *     • dir  = 'U', 'D', 'L', 'R' o '-' (plano)
 *     • level = 0…10   (0 = plano, 10 ≈ 90°)
 *
 *     Ejemplo: imprimir la inclinación
 *
 *       void onInclination(char dir, uint8_t lvl) {
 *         Serial.print("Inclinado hacia "); Serial.print(dir);
 *         Serial.print(" nivel "); Serial.println(lvl);
 *       }
 *
 *  NOTA: Si no necesitas la depuración, comenta la llamada a printLog()
 *        dentro de los callbacks.
 **************************************************************************/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEClient.h>
#include <BLERemoteCharacteristic.h>

/* ---------- CAMBIA AQUÍ EL NOMBRE DE TU MANDO ---------- */
const char CONTROLLER_NAME[] = "ESP32 Controller 1";   // ó 2, 3, 4

/* UUIDs (no modificar) */
#define SVC_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define BTN_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define JOY_UUID "3ca7de79-4a16-4a62-b89d-136d2f020e4f"
#define ORI_UUID "71ed9f07-e1f6-44e2-8e0c-0b7f31a78520"

/* ---------- OBJETOS BLE ---------- */
BLEScan*  pScan   = nullptr;
BLEClient* pClient = nullptr;
BLERemoteCharacteristic* pBtnChar = nullptr;
BLERemoteCharacteristic* pJoyChar = nullptr;
BLERemoteCharacteristic* pOriChar = nullptr;
bool deviceConnected = false;

/* ---------- VARIABLES PARA DEPURACIÓN ---------- */
volatile int8_t  joyLX = 0, joyLY = 0, joyRX = 0, joyRY = 0;
volatile char    lastBtn = '-';
volatile char    incDir  = '-';
volatile uint8_t incLvl  = 0;

/* ---------- SUBCLASE PARA EVENTOS DE CONEXIÓN ---------- */
class DummyCallbacks : public BLEClientCallbacks {
  void onConnect(BLEClient*)   override { Serial.println("   -> Conectado"); }
  void onDisconnect(BLEClient*) override { Serial.println("   -> Desconectado"); }
};

/* ---------- PROTOTIPOS ---------- */
void tryScanAndConnect();
bool connectToController(BLEAdvertisedDevice adv);
void onDisconnect();
void printLog();

/* ======================================================== */
/*                SECCIÓN PARA EL USUARIO                    */
/* ======================================================== */

/* --- BOTONES -------------------------------------------- */
void onButtonUp()       { /* tu código */ }
void onButtonDown()     { /* tu código */ }
void onButtonLeft()     { /* tu código */ }
void onButtonRight()    { /* tu código */ }
void onButtonL1()       { /* tu código */ }
void onButtonL2()       { /* tu código */ }
void onButtonL3()       { /* tu código */ }
void onButtonR1()       { /* tu código */ }
void onButtonR2()       { /* tu código */ }
void onButtonR3()       { /* tu código */ }
void onButtonCircle()   { /* tu código */ }
void onButtonSquare()   { /* tu código */ }
void onButtonTriangle() { /* tu código */ }
void onButtonCross()    { /* tu código */ }
void onButtonStart()    { /* tu código */ }
void onButtonPS()       { /* tu código */ }

/* --- JOYSTICKS ------------------------------------------ */
void onJoystickLeft (int8_t x, int8_t y) { /* tu código */ }
void onJoystickRight(int8_t x, int8_t y) { /* tu código */ }

/* --- INCLINACIÓN ---------------------------------------- */
void onInclination(char dir, uint8_t level) { /* tu código */ }

/* ======================================================== */
/*             CALLBACKS DE NOTIFICACIÓN BLE                */
/* ======================================================== */

void btnNotifyCB(BLERemoteCharacteristic*, uint8_t* d, size_t l, bool)
{
  if (l != 1) return;
  lastBtn = (char)d[0];

  if      (lastBtn=='U') onButtonUp();
  else if (lastBtn=='D') onButtonDown();
  else if (lastBtn=='L') onButtonLeft();
  else if (lastBtn=='R') onButtonRight();
  else if (lastBtn=='1') onButtonL1();
  else if (lastBtn=='2') onButtonL2();
  else if (lastBtn=='3') onButtonL3();
  else if (lastBtn=='4') onButtonR1();
  else if (lastBtn=='5') onButtonR2();
  else if (lastBtn=='6') onButtonR3();
  else if (lastBtn=='C') onButtonCircle();
  else if (lastBtn=='Q') onButtonSquare();
  else if (lastBtn=='T') onButtonTriangle();
  else if (lastBtn=='X') onButtonCross();
  else if (lastBtn=='S') onButtonStart();
  else if (lastBtn=='P') onButtonPS();

  printLog();          // comenta si no quieres depuración
}

void joyNotifyCB(BLERemoteCharacteristic*, uint8_t* d, size_t l, bool)
{
  if (l == 4) {
    joyLX = (int8_t)d[0];
    joyLY = (int8_t)d[1];
    joyRX = (int8_t)d[2];
    joyRY = (int8_t)d[3];

    /* Llama a los ganchos del usuario */
    onJoystickLeft(joyLX, joyLY);
    onJoystickRight(joyRX, joyRY);

    printLog();
  }
}

void oriNotifyCB(BLERemoteCharacteristic*, uint8_t* d, size_t l, bool)
{
  if (l == 2) {
    incDir = (char)d[0];
    incLvl = d[1];
    if (incDir == ' ') incDir = '-';   // neutro → '-'

    onInclination(incDir, incLvl);
    printLog();
  }
}

/* ---------- IMPRESIÓN UNIFICADA ---------- */
void printLog()
{
  Serial.printf("%d\t%d\t%d\t%d\t%c\t%c-%u\n",
                joyLX, joyLY, joyRX, joyRY,
                lastBtn,
                incDir, incLvl);
}

/* ======================================================== */
/*                       SETUP                              */
/* ======================================================== */
void setup()
{
  Serial.begin(115200);
  Serial.println("\nESP32 Receptor – iniciando…");

  BLEDevice::init("");
  pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(new DummyCallbacks());

  tryScanAndConnect();
}

/* ======================================================== */
/*                        LOOP                              */
/* ======================================================== */
void loop()
{
  if (deviceConnected && !pClient->isConnected()) {
    Serial.println("Conexión perdida.");
    onDisconnect();
  }

  if (!deviceConnected) {
    tryScanAndConnect();
    delay(1000);
  }

  delay(20);
}

/* ======================================================== */
/*          FUNCIONES DE CONEXIÓN / ESCANEO BLE             */
/* ======================================================== */
void tryScanAndConnect()
{
  Serial.println("Escaneando…");
  pScan = BLEDevice::getScan();
  pScan->setActiveScan(true);
  pScan->setInterval(100);
  pScan->setWindow(99);

  BLEScanResults* results = pScan->start(5, false);
  int count = results->getCount();

  for (int i = 0; i < count; i++) {
    BLEAdvertisedDevice dev = results->getDevice(i);
    if (dev.haveName() &&
        String(dev.getName().c_str()) == CONTROLLER_NAME) {

      Serial.print("Encontrado: "); Serial.println(CONTROLLER_NAME);
      if (connectToController(dev)) {
        pScan->clearResults();
        return;
      }
    }
  }

  Serial.println("Control no encontrado.");
  pScan->clearResults();
}

bool connectToController(BLEAdvertisedDevice adv)
{
  Serial.print("Conectando a "); Serial.println(adv.getAddress().toString().c_str());
  if (!pClient->connect(&adv)) {
    Serial.println("  Fallo de conexión");
    return false;
  }

  BLERemoteService* svc = pClient->getService(SVC_UUID);
  if (!svc) {
    Serial.println("  Servicio no disponible.");
    pClient->disconnect();
    return false;
  }

  pBtnChar = svc->getCharacteristic(BTN_UUID);
  pJoyChar = svc->getCharacteristic(JOY_UUID);
  pOriChar = svc->getCharacteristic(ORI_UUID);
  if (!pBtnChar || !pJoyChar || !pOriChar) {
    Serial.println("  Características faltantes.");
    pClient->disconnect();
    return false;
  }

  pBtnChar->registerForNotify(btnNotifyCB);
  pJoyChar->registerForNotify(joyNotifyCB);
  pOriChar->registerForNotify(oriNotifyCB);

  Serial.println("  Conexión y notificaciones OK");
  deviceConnected = true;
  return true;
}

void onDisconnect()
{
  Serial.println("  Conexión perdida");
  pClient->disconnect();
  deviceConnected = false;
}
