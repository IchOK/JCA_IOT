/*---------------------------------------------------------------
 * Basisprojekt für Mesharchitektur
 * Funktionen:
 *  MeshNode:
 *    - Eigenständige Node innerhalb des Netzwerks
 *    - Firmware laden über das Mesh-Netzwerk
 *    - 
 *  Gateway:
 *    - Verbindung zum allgemeinen WLAN
 *    - Kommunikation mit Node-RED
 *    - 
 * Änderungslog:
 *  V0.9.000  JCA   Erstellung anhand der Testprojekte 
 *---------------------------------------------------------------*/

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

//######################################################################
//-- HARDWARE --
//######################################################################
  #define HOSTNAME          "MeshGateway"
  #define DEBUGLEVEL        0
  #define DEBUGLEVEL_M      0
  #define STATION_SSID      "HomeSweetIOT"
  #define STATION_PASSWORD  "HomeSweetIOT2019"
  #define STATION_PORT      5555
  #define PIN_LED           LED_BUILTIN

  uint8_t station_ip[4] =   {192,168,223,1}; // Node-RED
  bool LED;

//######################################################################
//-- Mesh-Netzwerk --
//######################################################################
#include  "painlessMesh.h"

#define   MESH_PREFIX     "JCAmesh"
#define   MESH_PASSWORD   "T3f]xGX*J=QWDaZbv.v+M@3="
#define   MESH_PORT       5555
#define   MESH_CHANNEL    13 //Der Mesh-Channel muss dem WLAN-Channel entsprechen

//Prototypen
void sendMessage();  // Prototype so PlatformIO doesn't complain
void receivedCallback(uint32_t from, String &msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback();
void nodeTimeAdjustedCallback(int32_t offset);

Scheduler userScheduler;  // to control your personal task
painlessMesh mesh;

// User stub

Task taskSendMessage(TASK_SECOND * 1, TASK_FOREVER, &sendMessage);

//######################################################################
//-- SETUP --
//######################################################################
void setup() {
  #if (DEBUGLEVEL >= 1)
    uint16_t uiMeshDebug = 0;
    #if (DEBUGLEVEL_M >= 1)
      uiMeshDebug += ERROR;
    #endif
    #if (DEBUGLEVEL_M >= 3)
      uiMeshDebug += STARTUP;
    #endif
    #if (DEBUGLEVEL_M >= 7)
      uiMeshDebug += COMMUNICATION;
    #endif
    #if (DEBUGLEVEL_M >= 4)
      uiMeshDebug += CONNECTION;
    #endif
    
    Serial.begin(74880);
    mesh.setDebugMsgTypes( uiMeshDebug );
  #endif

  //Mesh ---------------------------------------------------------------
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA, MESH_CHANNEL );
  // Gateway to Node-Red
  mesh.stationManual(STATION_SSID, STATION_PASSWORD, STATION_PORT, station_ip); //Mit Node-RED direkt verbinden
  mesh.setRoot(true);
  mesh.setContainsRoot(true);

  // Set Hostname
  mesh.setHostname(HOSTNAME);

  // Init Calback Functions
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  // Init OTA Receiver
  mesh.initOTAReceive(HOSTNAME);

  // Start Task Scheduler
  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
}

//######################################################################
//-- LOOP --
//######################################################################
void loop() {
  mesh.update();
}

//######################################################################
//-- MESH -- Callbacks and Cycle Message
//######################################################################
void sendMessage() {
  String msg = "Hello OK from node ";
  msg += mesh.getNodeId();
  mesh.sendBroadcast(msg);
  taskSendMessage.setInterval(random(TASK_SECOND * 1, TASK_SECOND * 5));
}

// Needed for painless library
void receivedCallback(uint32_t from, String &msg) {
  #if (DEBUGLEVEL >= 5)
    Serial.printf("Mesh Recv: Received from %u msg=%s\n", from, msg.c_str());
  #endif
}

void newConnectionCallback(uint32_t nodeId) {
  #if (DEBUGLEVEL >= 5)
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
  #endif
}

void changedConnectionCallback() {
  #if (DEBUGLEVEL >= 5)
    Serial.printf("Changed connections\n");
  #endif
}

void nodeTimeAdjustedCallback(int32_t offset) {
  #if (DEBUGLEVEL >= 5)
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
  #endif
}
