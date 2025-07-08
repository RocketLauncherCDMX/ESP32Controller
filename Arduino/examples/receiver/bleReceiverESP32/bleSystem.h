/* =====================================================================
 *  bleSystem.h  ·  Motor BLE para ESP32 Controller (NO EDITAR)
 * ---------------------------------------------------------------------
 *  API que ofrece al sketch:
 *
 *    • void  bleSys::begin(const char* name);
 *    • void  bleSys::task();               (llamar en loop)
 *
 *    • char  bleSys::readButton();         // 0 si no hay nuevo botón
 *
 *    • bool  bleSys::joystickAvailable();  // true una sola vez por cambio
 *      int8  bleSys::joyLX, joyLY, joyRX, joyRY;
 *
 *    • bool  bleSys::orientationAvailable();
 *      char  bleSys::incDir;   uint8_t bleSys::incLvl;
 *
 *  El núcleo NO imprime nada: todo el log se maneja en el sketch.
 * ====================================================================*/
#ifndef _BLE_SYSTEM_H_
#define _BLE_SYSTEM_H_

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEClient.h>
#include <BLERemoteCharacteristic.h>

/* ---------- UUIDs del mando (NO tocar) ---------- */
#define SVC_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define BTN_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define JOY_UUID "3ca7de79-4a16-4a62-b89d-136d2f020e4f"
#define ORI_UUID "71ed9f07-e1f6-44e2-8e0c-0b7f31a78520"

namespace bleSys {

/* --------- variables públicas --------- */
inline volatile int8_t  joyLX = 0, joyLY = 0, joyRX = 0, joyRY = 0;
inline volatile char    incDir = '-';
inline volatile uint8_t incLvl = 0;

/* --------- funciones públicas --------- */
void begin(const char* controllerName);
void task();

char readButton();                 // 0 si no hay evento nuevo
bool joystickAvailable();          // true sólo una vez tras cada cambio
bool orientationAvailable();       // idem

/* ================================================================ */
/* ==============  IMPLEMENTACIÓN –  NO modificar  ================ */
/* ================================================================ */
namespace {                        /* área interna anónima ---------- */
  const char* _ctrlName = nullptr;
  BLEClient*  _cli      = nullptr;
  BLEScan*    _scan     = nullptr;
  BLERemoteCharacteristic *_btn=nullptr,*_joy=nullptr,*_ori=nullptr;
  bool _connected = false;

  /* ---- cola de eventos ---- */
  volatile char _btnEvt = 0;
  volatile bool _btnNew = false;
  volatile char _btnLastReported = '-';   // <-- evita repeticiones
  volatile bool _joyNew = false, _oriNew = false;

  /* ---------- callbacks BLE ---------- */
  void _btnCB(BLERemoteCharacteristic*, uint8_t* d, size_t l, bool) {
    if (l != 1) return;
    char cur = (char)d[0];

    /* Si llega el mismo botón que ya se informó, ignorar */
    if (cur == _btnLastReported) return;

    _btnEvt = cur;
    _btnNew = true;
    _btnLastReported = cur;               // bloquea mientras se mantenga
  }

  void _joyCB(BLERemoteCharacteristic*, uint8_t* d, size_t l, bool) {
    if (l != 4) return;
    joyLX=d[0]; joyLY=d[1]; joyRX=d[2]; joyRY=d[3];
    _joyNew = true;
  }

  void _oriCB(BLERemoteCharacteristic*, uint8_t* d, size_t l, bool) {
    if (l != 2) return;
    incDir = (char)d[0]; if (incDir==' ') incDir='-';
    incLvl = d[1];
    _oriNew = true;
  }

  /* ---------- conexión ---------- */
  void _lost() { _cli->disconnect(); _connected=false; }

  bool _connect(BLEAdvertisedDevice adv){
    if(!_cli->connect(&adv)) return false;
    auto* svc=_cli->getService(SVC_UUID);
    if(!svc){_cli->disconnect();return false;}
    _btn=svc->getCharacteristic(BTN_UUID);
    _joy=svc->getCharacteristic(JOY_UUID);
    _ori=svc->getCharacteristic(ORI_UUID);
    if(!(_btn&&_joy&&_ori)){_cli->disconnect();return false;}
    _btn->registerForNotify(_btnCB);
    _joy->registerForNotify(_joyCB);
    _ori->registerForNotify(_oriCB);
    _connected=true;
    Serial.println("   -> Conectado y suscrito");
    return true;
  }

  void _scanTry(){
    Serial.print("Buscando "); Serial.println(_ctrlName);
    _scan=BLEDevice::getScan();
    _scan->setActiveScan(true); _scan->setInterval(100); _scan->setWindow(99);
    auto* res=_scan->start(5,false);
    for(int i=0;i<res->getCount();++i){
      BLEAdvertisedDevice d=res->getDevice(i);
      if(d.haveName() && String(d.getName().c_str())==_ctrlName){
        Serial.print("Encontrado: "); Serial.println(_ctrlName);
        if(_connect(d)){_scan->clearResults();return;}
      }
    }
    _scan->clearResults();
    Serial.println("No conectado.");
  }

  class _CB : public BLEClientCallbacks{
   void onConnect   (BLEClient*) override{Serial.println("   -> Conectado (BLE)");}
   void onDisconnect(BLEClient*) override{Serial.println("   -> Desconectado");}
  };
}

/* -------- API visible -------- */
inline char readButton(){
  if (!_btnNew) return 0;
  _btnNew = false;
  char ret = _btnEvt;
  _btnLastReported = '-';          // desbloquea el mismo botón para próxima pulsación
  return ret;
}
inline bool joystickAvailable(){ bool f=_joyNew; _joyNew=false; return f; }
inline bool orientationAvailable(){ bool f=_oriNew; _oriNew=false; return f; }

void begin(const char* name){
  _ctrlName = name;
  Serial.println("\nESP32 Receptor – iniciando…");
  BLEDevice::init("");
  _cli = BLEDevice::createClient();
  _cli->setClientCallbacks(new _CB());
  _scanTry();
}
void task(){
  if(_connected && !_cli->isConnected()) _lost();
  if(!_connected){ _scanTry(); delay(1000);}
  delay(20);
}
} // namespace bleSys
#endif
