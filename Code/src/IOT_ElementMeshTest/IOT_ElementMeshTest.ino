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

//######################################################################
//-- Mesh-Netzwerk --
//######################################################################
#include  "painlessMesh.h"

#define   MESH_PREFIX     "JCAmesh"
#define   MESH_PASSWORD   "T3f]xGX*J=QWDaZbv.v+M@3="
#define   MESH_PORT       5555
#define   MESH_CHANNEL    13 //Der Mesh-Channel muss dem WLAN-Channel entsprechen

Scheduler userScheduler;  // to control your personal task
painlessMesh  Mesh;

//######################################################################
//-- IOT --
//######################################################################
#include <ArduinoJson.h>

#include "JCA_IOT.h"

using namespace JCA::IOT;

//Zwischenspeicher für Differenz-Millisekunden
uint32_t StoreMillis;

//JsonObject as Pointer
JsonObject JConfig;

//Element Handler
ELEMENT::cHandler ElementHandler;

//Mesh Handler
MESH::cHandler MeshHandler;
void MeshHandlerRecv(uint32_t from, String msg){
  MeshHandler.recvMsg(from, msg);
}

void setup() {
  bool RetIO;
  #if DEBUGLEVEL > 0
    Serial.begin(74880);
    Serial.println("add Elements...");
  #endif
  //Add possible Elements
  beginDI(ElementHandler);
  beginDO(ElementHandler);

  // Init PainlessMesh
  Mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA, MESH_CHANNEL );
  Mesh.onReceive(&MeshHandlerRecv);
  
  // Mesh-Config
  RetIO = MeshHandler.config("/Test.json", &(ElementHandler.Elements), JConfig, Mesh);
  
  //Konfig Element-Handler
  RetIO = ElementHandler.config(JConfig);

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
  ElementHandler.update(DiffMillis, Timestamp);
//  MeshConfig.update(DiffMillis, Mesh);
//  MeshClient.update(&(IotHandler.Elements), MeshOut, DiffMillis, Timestamp);
}
