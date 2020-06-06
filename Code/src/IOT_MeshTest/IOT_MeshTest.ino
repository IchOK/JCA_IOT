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
//-- JSON --
//######################################################################
//#define ARDUINOJSON_ENABLE_STD_STRING 1
//#include <ArduinoJson.h>

//######################################################################
//-- Mesh-Netzwerk --
//######################################################################
#include  "namedMesh.h"

#define   MESH_PREFIX     "JCAmesh"
#define   MESH_PASSWORD   "T3f]xGX*J=QWDaZbv.v+M@3="
#define   MESH_PORT       5555
#define   MESH_CHANNEL    13 //Der Mesh-Channel muss dem WLAN-Channel entsprechen

#include  "Mesh_Protocol.h"

//Prototypen
void receivedCallbackMeshID( uint32_t from, String &msg );
void receivedCallbackMeshName( String &from, String &msg );
void taskCallbackSendAlive();

//Variablen-Objekte
namedMesh mesh;
Task taskMeshSendAlive;

//######################################################################
//-- Scheduler --
//######################################################################
Scheduler     userScheduler;

//######################################################################
//-- HARDWARE --
//######################################################################

#define NODETYPE_GW 1
#define NODETYPE_RO 2
#define NODETYPE_TB 3
#define NDOETYPE_WL 4
#define NODETYPE NODETYPE_RO

#if (NODETYPE == NODETYPE_RO)
  #include "RoAnlage.h"
#elif (NODETYPE == NODETYPE_GW)
  #include "Gateway.h"
#elif (NODETYPE == NODETYPE_TB)
  #include "TestBoard.h"
#elif (NODETYPE == NODETYPE_WL)
  #include "RgbLampe.h"
#else
  #define HOSTNAME    "UnknownNode"
  #define DEBUGLEVEL  0
  #define DEBUGLEVEL_M  0
  void hwSetup(){;};
  void meshGetDataJ(const JsonObject command, JsonArray &jaData){;};
  void meshSetDataJ(const JsonObject command, JsonArray &jaData){;};
  void fcLoop(){;};
#endif

//######################################################################
//-- OTA Over The Air --
//######################################################################
#include <JCA_IOT_MESH_OTA.h>
#ifdef ESP32
  #include <SPIFFS.h>
  #include <Update.h>
#else
  #include <FS.h>
#endif
#include "base64.h"

//Prototypen
void createDataRequest(JsonObject &req, JCA::IOT::MESH::tOtaFW updateFW);
void firmwareFromJSON(JCA::IOT::MESH::tOtaFW &fw, JsonObject &req);

//Variablen-Objekte
JCA::IOT::MESH::tOtaFW currentFW;
JCA::IOT::MESH::tOtaFW updateFW;
Task taskOtaDataRequest;

//######################################################################
//-- SETUP --
//######################################################################
void setup() {
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
  
  #if (DEBUGLEVEL >= 1)
    Serial.begin(74880);
    mesh.setDebugMsgTypes( uiMeshDebug );
  #endif

  //Hardware -----------------------------------------------------------
  hwSetup();
  
  //Mesh ---------------------------------------------------------------
  mesh.init( MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT, WIFI_AP_STA, MESH_CHANNEL );

  // Verbindung zum allgemein WLAN herstellen
  #if (NODETYPE == NODETYPE_GW)
    // Verbindung zum allgemeinen WLAN und zum Node-RED
    mesh.stationManual(STATION_SSID, STATION_PASSWORD, STATION_PORT, station_ip); //Mit Node-RED direkt verbinden
    // Mesh als Root definieren
    mesh.setRoot(true);
  #endif

  mesh.setHostname(HOSTNAME);
  String nodeName = HOSTNAME;
  mesh.setName(nodeName);

  // Root im Netzwerk vorhanden
  mesh.setContainsRoot(true);

  // Callback für Daten aus dem Mesh-Netzwerk
  mesh.onReceive(&receivedCallbackMeshID);
  mesh.onReceive(&receivedCallbackMeshName);
  
  // Lebenszeichen aktivieren
  userScheduler.addTask(taskMeshSendAlive);
  taskMeshSendAlive.set(TASK_SECOND*10, TASK_FOREVER, &taskCallbackSendAlive);
  taskMeshSendAlive.enable();
  
  //OTA ----------------------------------------------------------------
  userScheduler.addTask(taskOtaDataRequest);
  
  #ifdef ESP32
    SPIFFS.begin(true);  // Start the SPI Flash Files System
  #else
    SPIFFS.begin();  // Start the SPI Flash Files System
  #endif
  if (SPIFFS.exists(OTA_FN)) {
    auto file = SPIFFS.open(OTA_FN, "r");
    String msg = "";
    while (file.available()) {
        msg += (char)file.read();
    }
    DynamicJsonDocument jsonBuffer(1024);
    DeserializationError error = deserializeJson(jsonBuffer, msg);
    if (error) {
        Serial.printf("JSON DeserializationError\n");
    }
    JsonObject root = jsonBuffer.as<JsonObject>();
    firmwareFromJSON(currentFW, root);
    #if (DEBUGLEVEL >= 3)
      Serial.printf("Current firmware MD5: %s, type = %s, hardware = %s\n",
                    currentFW.md5.c_str(), currentFW.nodeType.c_str(),
                    currentFW.hardware.c_str());
      file.close();
    #endif
  }
  else {
    currentFW.nodeType = HOSTNAME;
    #if (DEBUGLEVEL >= 3)
        Serial.printf("No OTA_FN found!\n");
    #endif
  }
  
  //DONE --------------------------------------------------------------
  #if (DEBUGLEVEL >= 1)
    Serial.print("My ST IP is ");
    Serial.println(mesh.getStationIP());
    Serial.print("My AP IP is ");
    Serial.println(mesh.getAPIP());
  #endif
}

//######################################################################
//-- LOOP --
//######################################################################
void loop() {
  userScheduler.execute();
  mesh.update();
  fcLoop();
}

//######################################################################
//-- Callback OTA --
//######################################################################
void createDataRequest(JsonObject &req, JCA::IOT::MESH::tOtaFW updateFW) {
    req["plugin"] = "ota";
    req["type"] = "request";
    req["hardware"] = updateFW.hardware;
    req["nodeType"] = updateFW.nodeType;
    req["md5"] = updateFW.md5;
    req["noPart"] = updateFW.noPart;
    req["partNo"] = updateFW.partNo;
}

void firmwareFromJSON(JCA::IOT::MESH::tOtaFW &fw, JsonObject &req) {
    fw.hardware = req["hardware"].as<String>();
    fw.nodeType = req["nodeType"].as<String>();
    fw.md5 = req["md5"].as<String>();
}

//######################################################################
//-- Callback MESH --
//######################################################################
bool handleRequest(String &msgIn, String &msgOut, uint32_t from){
    DynamicJsonDocument jdIn(1024 + msgIn.length());
    DynamicJsonDocument jdOut(1024);
    JsonObject joRoot;
    #if (DEBUGLEVEL >= 5)
      Serial.println(msgIn);
      Serial.println(" Json-Doc erzeugt");
    #endif
  
    //Json decodieren
    DeserializationError error = deserializeJson(jdIn, msgIn);
    if (error){
      #if (DEBUGLEVEL >= 5)
        Serial.println(" Json-Doc FAILED");
      #endif
      return false;
    }

    //Json Objekt erstellen
    #if (DEBUGLEVEL >= 5)
      Serial.println(" Json-Doc DONE");
    #endif
    joRoot = jdIn.as<JsonObject>();

    if (joRoot.containsKey("ident")){
      //Anforderung auswerten
      #if (DEBUGLEVEL >= 5)
        Serial.println("  Ident gefunden");
      #endif
      int i = joRoot["ident"];
      switch(i){
        case 0://Standard Echo
          #if (DEBUGLEVEL >= 5)
            Serial.println("    ECHO");
          #endif
          msgOut = "{\"ident\":100,\"name\":\"" + mesh.getName() + "\"}";
          return true;
          break;
        
        case 1://aktiviere OTA
          #if (DEBUGLEVEL >= 5)
            Serial.println("    OTA");
          #endif
          if (currentFW.nodeType.equals(joRoot["nodeType"].as<String>()) &&
            currentFW.hardware.equals(joRoot["hardware"].as<String>())) 
          {
            if (String("version").equals(joRoot["type"].as<String>())) {
              if ((currentFW.md5.equals(joRoot["md5"].as<String>()) ||
                updateFW.md5.equals(joRoot["md5"].as<String>())) &&
                !(joRoot["forced"].as<bool>()))
              {
                #if (DEBUGLEVEL >= 5)
                  Serial.println("     FW bereits bekannt");
                #endif
                return false;  // Announced version already known
              } else {
                #if (DEBUGLEVEL >= 5)
                  Serial.print("     neue Firmware von: ");
                  Serial.println(from);
                #endif
                // Setup new updatedFW
                updateFW = currentFW;
                updateFW.md5 = joRoot["md5"].as<String>();
                updateFW.partNo = 0;
                updateFW.noPart = joRoot["noPart"].as<size_t>();
      
                taskOtaDataRequest.set(30 * TASK_SECOND, 5, [from]() {
                  DynamicJsonDocument jsonBuffer(256);
                  auto req = jsonBuffer.to<JsonObject>();
                  createDataRequest(req, updateFW);
                  String msg;
                  serializeJson(req, msg);
                  uint32_t cpyFrom = from;
                  mesh.sendSingle(cpyFrom, msg);
                  #if (DEBUGLEVEL >= 5)
                    Serial.print("     request FW von: ");
                    Serial.println(cpyFrom);
                    Serial.println(msg);
                  #endif
                });
                taskOtaDataRequest.enableIfNot();
                taskOtaDataRequest.forceNextIteration();
              }
            } 
            else if (String("data").equals(joRoot["type"].as<String>()) &&
                updateFW.partNo == joRoot["partNo"].as<size_t>()) 
            {
              #if (DEBUGLEVEL >= 5)
                Serial.println("     neue FW-Data");
              #endif
              size_t partNo = joRoot["partNo"];
              size_t noPart = joRoot["noPart"];
              if (partNo == 0) {
                String otaMD5 = joRoot["md5"].as<String>();
                #ifdef ESP32
                  uint32_t maxSketchSpace = UPDATE_SIZE_UNKNOWN;
                #else
                  uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
                #endif
                #if (DEBUGLEVEL >= 5)
                  Serial.printf("Sketch size %d\n", maxSketchSpace);
                #endif
                if (Update.isRunning()) {
                  Update.end(false);
                }
                if (!Update.begin(maxSketchSpace)) {  // start with max available size
                  #if (DEBUGLEVEL >= 5)
                    Serial.println("handleOTA(): OTA start failed!");
                    Update.printError(Serial);
                  #endif
                  Update.end();
                } else {
                  Update.setMD5(otaMD5.c_str());
                }
              }
              //    write data
              auto b64data = joRoot["data"].as<std::string>();
              auto b64Data = base64_decode(b64data);

              if (Update.write((uint8_t *)b64Data.c_str(),
                  b64Data.length()) != b64Data.length()) 
              {
                #if (DEBUGLEVEL >= 5)
                  Serial.println("handleOTA(): OTA write failed!");
                  Update.printError(Serial);
                #endif
                Update.end();
                return false;
              }

              if (partNo == noPart - 1) {
                //       check md5, reboot
                if (Update.end(true)) {  // true to set the size to the
                                       // current progress
                  #if (DEBUGLEVEL >= 5)
                    Serial.printf("Update.MD5: %s\n", Update.md5String().c_str());
                  #endif
                  auto file = SPIFFS.open(OTA_FN, "w");
                  DynamicJsonDocument jsonBuffer(1024);
                  auto req = jsonBuffer.to<JsonObject>();
                  createDataRequest(req, updateFW);
                  String msg;
                  serializeJson(req, msg);
                  file.print(msg);
                  file.close();

                  #if (DEBUGLEVEL >= 5)
                    Serial.println("handleOTA(): OTA Success!");
                  #endif
                  ESP.restart();
                } else {
                  #if (DEBUGLEVEL >= 5)
                    Serial.println("handleOTA(): OTA failed!");
                    Update.printError(Serial);
                  #endif
                }
                taskOtaDataRequest.disable();
              } else {
                ++updateFW.partNo;
                taskOtaDataRequest.setIterations(5);
                taskOtaDataRequest.forceNextIteration();
              }
            }
          }
          return false;
          break;

        case 2://Set Data
          #if (DEBUGLEVEL >= 5)
            Serial.println("    Set Data");
          #endif
          jdOut["ident"] = 102;
          jdOut["name"] = mesh.getName();
//          msgOut = "";
          if (joRoot.containsKey("commands")){
//            String strArray;
//            strArray = "";
            //Anforderung auswerten
            #if (DEBUGLEVEL >= 5)
              Serial.println("     Befehle gefunden");
            #endif
            JsonArray jaCommands = joRoot["commands"].as<JsonArray>();
            JsonArray jaData = jdOut.createNestedArray("data");
            for (JsonObject joCommand : jaCommands){
              if (joCommand.containsKey("name") && joCommand.containsKey("value")){
                #if (DEBUGLEVEL >= 5)
                  Serial.println("      Befehl gefunden");
                #endif
/*                if (strArray.length() > 0){
                  strArray += ",";
                }
                strArray += meshSetData(joCommand["name"], joCommand["value"]);*/
                meshSetDataJ(joCommand, jaData);
              }
            }
/*            if (strArray.length() > 0){
              msgOut = "{\"ident\":102,\"name\":\"" + mesh.getName() + "\",\"data\":[" + strArray + "]}";
              return true;
            }else{
              msgOut = "{\"ident\":102,\"name\":\"" + mesh.getName() + "\",\"data\":[],\"error\":\"keine Befehle gefunden\"}";
              return true;
            }*/
            if (jaData.size() == 0){
              jdOut["error"] = "keine Befehle gefunden";
            }
          }else{
//            msgOut = "{\"ident\":102,\"name\":\"" + mesh.getName() + "\",\"data\":[],\"error\":\"Syntax: 'commands' fehlt\"}";
//            return true;
            jdOut["error"] = "Syntax: 'commands' fehlt";
          }
          serializeJson(jdOut, msgOut);
          break;
        
        case 3://Get Data
          #if (DEBUGLEVEL >= 5)
            Serial.println("    Get Data");
          #endif
          jdOut["ident"] = 103;
          jdOut["name"] = mesh.getName();
          if (joRoot.containsKey("requests")){
//            String strArray;
//            strArray = "";
            //Anforderung auswerten
            #if (DEBUGLEVEL >= 5)
              Serial.println("     Anfragen gefunden");
            #endif
            JsonArray jaRequests = joRoot["requests"].as<JsonArray>();
            JsonArray jaData = jdOut.createNestedArray("data");
            for (JsonObject joRequest : jaRequests){
              if (joRequest.containsKey("name")){
                #if (DEBUGLEVEL >= 5)
                  Serial.println("      Anfrage gefunden");
                #endif
/*                if (strArray.length() > 0){
                  strArray += ",";
                }
                strArray += meshGetData(joRequest["name"]);*/
                meshGetDataJ(joRequest, jaData);
              }
            }
/*            if (strArray.length() > 0){
              msgOut = "{\"ident\":103,\"name\":\"" + mesh.getName() + "\",\"data\":[" + strArray + "]}";
              return true;
            }else{
              msgOut = "{\"ident\":103,\"name\":\"" + mesh.getName() + "\",\"data\":[],\"error\":\"keine Daten gefunden\"}";
              return true;
            }*/
            if (jaData.size() == 0){
              jdOut["error"] = "keine Daten gefunden";
            }
          }else{
//            msgOut = "{\"ident\":103,\"name\":\"" + mesh.getName() + "\",\"data\":[],\"error\":\"Syntax: 'requests' fehlt\"}";
//            return true;
            jdOut["error"] = "Syntax: 'requests' fehlt";
          }
          serializeJson(jdOut, msgOut);
          break;
        
        default:
          #if (DEBUGLEVEL >= 5)
            Serial.println("    UNBEKANNT");
          #endif
          return false;
      }
    }else{
      #if (DEBUGLEVEL >= 5)
        Serial.println("  Ident NICHT gefunden");
      #endif
      return false;
    }
}
void receivedCallbackMeshID( uint32_t from, String &msg ) {
  String msgOut;
  #if (DEBUGLEVEL >= 5)
    Serial.print("Mesh Recv: Received from ");
    Serial.print(from);
    Serial.println(" :");
  #endif
  if(handleRequest(msg, msgOut, from)){
    mesh.sendSingle(from , msgOut);
  }
}

void receivedCallbackMeshName( String &from, String &msg ) {
  String msgOut;
  uint32_t fromId;
  mesh.getIdByName(fromId, from);
  #if (DEBUGLEVEL >= 5)
    Serial.print("Mesh Recv: Received from ");
    Serial.print(from);
    Serial.println(" :");
  #endif
  if(handleRequest(msg, msgOut, fromId)){
    mesh.sendSingle(from , msgOut);
  }
}

void taskCallbackSendAlive(){
  DynamicJsonDocument jsonBuffer(1024);
  JsonObject msg = jsonBuffer.to<JsonObject>();
  String strTemp;

  msg["ident"] = JCA_IOT_MPI_ALIVE;
  msg["time"] = "1970-01-01 00:00:00";
  serializeJson(msg, strTemp);
  String to = "NodeRED";
  mesh.sendSingle(to, strTemp);
  
  #if (DEBUGLEVEL >= 7)
    serializeJson(msg, Serial);
    Serial.printf("\n");
  #endif
}

static void JCA_MeshNodeAddName(JsonObject &joMeshNode){
  //Namen ermitteln
  String szNodeName;
  uint32_t uiNodeID = joMeshNode["nodeId"].as<uint32_t>();
  if(mesh.getNodeId() == uiNodeID){
    #if (DEBUGLEVEL >= 8)
      Serial.print("  NodeName: ");
      Serial.println(mesh.getName());
    #endif
    joMeshNode["name"] = mesh.getName();
  }else if(mesh.getNameById(szNodeName, uiNodeID)){
    #if (DEBUGLEVEL >= 8)
      Serial.print("  NodeName: ");
      Serial.println(szNodeName);
    #endif
    joMeshNode["name"] = szNodeName;
  }
  //Prüfen ob subs vorhanden sind
  if(joMeshNode.containsKey("subs")){
    JsonArray jaMeshSubs = joMeshNode["subs"].as<JsonArray>();
    for (JsonObject joMeshSub : jaMeshSubs){
      JCA_MeshNodeAddName(joMeshSub);
    }
  }
}
