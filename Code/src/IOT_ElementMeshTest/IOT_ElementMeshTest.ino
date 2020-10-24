//   || 
//   ||  Serial Debug Level
//  \||/
//   \/

//######################################################################
//-- Mesh-Netzwerk --
//######################################################################
#include  "painlessMesh.h"

#define   MESH_PREFIX     "JCAmesh"
#define   MESH_PASSWORD   "T3f]xGX*J=QWDaZbv.v+M@3="
#define   MESH_PORT       5555
#define   MESH_CHANNEL    13 //Der Mesh-Channel muss dem WLAN-Channel entsprechen
#define   CONFIG_FILE     "/Config.json"

Scheduler UserScheduler;
painlessMesh  Mesh;

//######################################################################
//-- IOT --
//######################################################################
#include <ArduinoJson.h>

// Debuglevels:
// 0 : keine Debugmeldungen
// 1 : Diagnose Meldungen
// 2 : Watchdogs
// 3 : Startup Information
// 4 : Telegramm Data
// 5 : Mesh Data
// 6 : Conversions and internal Data
// 7 : Scheduler Prints
// 8 : Loop Prints
// 9 : alle Debugmeldungen
#define DEBUGLEVEL JCA_IOT_DEBUG_ALL //JCA_IOT_DEBUG_STARTUP //JCA_IOT_DEBUG_SCHEDULER //JCA_IOT_DEBUG_TELEGRAM
#define DEBUGLEVEL_M JCA_IOT_DEBUG_NONE //JCA_IOT_DEBUG_ALL //JCA_IOT_DEBUG_MESHDATA

// Hardware Erweiterungen und Bussysteme
#define JCA_IOT_ELEMENT_ONEWIRE

ADC_MODE(ADC_VCC);
#include "JCA_IOT.h"
using namespace JCA::IOT;

//Zwischenspeicher für Differenz-Millisekunden
#define JCA_IOT_TASK_MILLIS 50
uint32_t StoreMillis;
uint32_t Timestamp;
#if DEBUGLEVEL >= JCA_IOT_DEBUG_SCHEDULER
  uint32_t SumTaskMillis = 0;
  uint32_t SumMeshMillis = 0;
  uint32_t SumElementMillis = 0;
  uint32_t TaskCount = 0;
  uint32_t TaskOutput = 10000 / JCA_IOT_TASK_MILLIS;
#endif

//JsonObject as Pointer
JsonObject JConfig;

//Element Handler
ELEMENT::cHandler ElementHandler;

//Mesh Handler
MESH::cHandler MeshHandler;
void MeshHandlerRecv(uint32_t from, String msg){
  MeshHandler.recvMsg(from, msg);
}

//Update Task
void updateIOT(){
  uint32_t ActMillis = millis();
  uint32_t DiffMillis = ActMillis - StoreMillis;
  StoreMillis = ActMillis;
  #if DEBUGLEVEL >= JCA_IOT_DEBUG_SCHEDULER
    uint32_t StartMillis = millis();
  #endif
  Timestamp = MeshHandler.update(DiffMillis, Mesh);
  #if DEBUGLEVEL >= JCA_IOT_DEBUG_SCHEDULER
    uint32_t MeshMillis = millis();
  #endif
  ElementHandler.update(DiffMillis, Timestamp);
  #if DEBUGLEVEL >= JCA_IOT_DEBUG_SCHEDULER
    uint32_t ElementMillis = millis();
    SumTaskMillis += DiffMillis;
    SumMeshMillis += MeshMillis - StartMillis;
    SumElementMillis += ElementMillis - MeshMillis;
    TaskCount += 1;
    if (TaskCount >= TaskOutput) {
      Serial.printf("TASK : Task=%i\r\n  Mesh=%i\r\n  Element=%i\r\n  Timestamp=%i\r\n", SumTaskMillis / TaskCount, SumMeshMillis, SumElementMillis, Timestamp);
      SumTaskMillis = 0;
      SumMeshMillis = 0;
      SumElementMillis = 0;
      TaskCount = 0;
    }
  #endif
}
Task UpdateTask(TASK_MILLISECOND * JCA_IOT_TASK_MILLIS, TASK_FOREVER, &updateIOT);

void setup() {
  bool RetIO;
  //Add possible Elements
  #if DEBUGLEVEL > JCA_IOT_DEBUG_NONE
    uint16_t MeshDebug = 0;
    #if DEBUGLEVEL_M >= JCA_IOT_DEBUG_DIAG
      MeshDebug += ERROR;
    #endif
    #if DEBUGLEVEL_M >= JCA_IOT_DEBUG_STARTUP
      MeshDebug += STARTUP;
    #endif
    #if DEBUGLEVEL_M >= JCA_IOT_DEBUG_MESHDATA
      MeshDebug += COMMUNICATION;
    #endif
    #if DEBUGLEVEL_M >= JCA_IOT_DEBUG_SCHEDULER
      MeshDebug += CONNECTION;
    #endif
    Mesh.setDebugMsgTypes( MeshDebug );
    Serial.begin(74880);
    Serial.print(F("\r\n##############################\r\n"));
    Serial.print(F("SET IOT Debug to "));
    Serial.println(DEBUGLEVEL);
    Serial.print(F("SET Mesh Debug to "));
    Serial.print(MeshDebug, BIN);
    Serial.println("");
    Serial.print(F("BuildIn LED "));
    Serial.print(LED_BUILTIN);
    Serial.println("");
  #endif

  #if DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP
    Serial.println(F("SETUP START"));
    Serial.println(F("add Elements..."));
  #endif
  beginDI(ElementHandler);
  beginDO(ElementHandler);
  beginAI(ElementHandler);
  beginSinGen(ElementHandler);
  beginOR(ElementHandler);
  beginCLOCKSPAN(ElementHandler);
  beginCLOCKPULSE(ElementHandler);
  beginHeatCool(ElementHandler);
  beginDS18B20(ElementHandler);

  // Init PainlessMesh
  #if DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP
    Serial.println("init Mesh...");
  #endif
  Mesh.init( MESH_PREFIX, MESH_PASSWORD, &UserScheduler, MESH_PORT, WIFI_AP_STA, MESH_CHANNEL );
  Mesh.onReceive(&MeshHandlerRecv);
  
  // Mesh-Config
  RetIO = MeshHandler.config(CONFIG_FILE, &(ElementHandler.Elements), JConfig, Mesh);
  
  //Konfig Element-Handler
  RetIO = ElementHandler.config(JConfig);

  UserScheduler.addTask(UpdateTask);
  UpdateTask.enable();
  #if DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP
    Serial.println("SETUP DONE");
  #endif
  StoreMillis = millis();
}

void loop() {
  Mesh.update();
}
