// Debuglevels:
// 0 : keine Debugmeldungen
// 1 : Diagnose Meldungen
// 2 : 
// 3 : Startup Information
// 4 :
// 5 : Mesh Data
// 6 :
// 7 : Scheduler Prints
// 8 : Loop Prints
// 9 : alle Debugmeldungen
#define DEBUGLEVEL 2

//#include <FS.h>
#include <painlessMesh.h>
#include <ArduinoJson.h>

#include "JCA_IOT_ELEMENT.h"
#include "JCA_IOT_MESH.h"

using namespace JCA::IOT;

//Zwischenspeicher für Differenz-Millisekunden
uint32_t StoreMillis;

//JsonObject as Pointer
JsonObject JConfig;
//Painless-Mesh Instanz
painlessMesh  Mesh;

//Element Handler
ELEMENT::cHandler IotHandler;

//Mesh Teile einzeln Testen
MESH::cConfig MeshConfig;
MESH::cClient MeshClient;
StaticJsonDocument<JCA_IOT_ELEMENT_HANDLER_JSON_DOCSIZE> JDocMesh;
JsonObject MeshOut = JDocMesh.to<JsonObject>();;

void setup() {
  bool RetIO;
  #if DEBUGLEVEL > 0
    Serial.begin(74880);
    Serial.println("add Elements...");
  #endif
  //Add possible Elements
  beginDI(IotHandler);
  beginDO(IotHandler);
  
  //TEST
  
  // Mesh-Config
  RetIO = MeshConfig.config("/Test.json", &(IotHandler.Elements), JConfig);
  
  //TEST END

  //Konfig Element-Handler
  RetIO = IotHandler.config(JConfig);

  StoreMillis = millis();
}

void loop() {
  uint32_t Timestamp;
  uint32_t ActMillis = millis();
  uint32_t DiffMillis = ActMillis - StoreMillis;
  #if DEBUGLEVEL >= 3
    delay(2000);
  #endif
  StoreMillis = ActMillis;
  IotHandler.update(DiffMillis, Timestamp);
  MeshConfig.update(DiffMillis, Mesh);
  MeshClient.update(&(IotHandler.Elements), MeshOut, DiffMillis, Timestamp);
}
