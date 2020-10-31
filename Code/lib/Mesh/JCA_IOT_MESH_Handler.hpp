/**********************************************
 * Class:   JCA_IOT_MESH_Handler
 * Info:    Die Klasse stellt den Handler für
 *          alle MESH-Funktionen dar und bietet die
 *          die Schnittstelle zum Mesh-Netzwerk
 * Version:
 *    V1.0.0  Erstellt    19.06.2020  JCA
 *    -add Properties
 *       -- Name
 *       -- Config cConfig
 *       -- Client cClient
 *       -- MeshNames vector<id to Name>
 *       -- Elements *vector
 *       -- MeshOut JsonObject as Buffer
 *    -add Methoden
 *       -- cHandler
 *       -- config
 *       -- update
 *    V1.0.1  Erweiterung 02.08.2020  JCA
 *    -sendMsg
 *       -- srcNode Id auflösen für Fehlerberichte
 *       -- dstNode mit lokalem Node-Name ersetzen
 *    -recvMsg
 *       -- Sync-Push/-Request Aufrufe
 **********************************************/

#ifndef _JCA_IOT_MESH_HANDLER_H
#define _JCA_IOT_MESH_HANDLER_H

//Include extrenal
#include <vector>
#include <ArduinoJson.h>

#include "JCA_IOT_Debug.h"
#include "Mesh/JCA_IOT_MESH_define.h"
#include "Mesh/JCA_IOT_MESH_types.h"
#include "Mesh/JCA_IOT_MESH_Client.hpp"
#include "Mesh/JCA_IOT_MESH_Config.hpp"
#include "Mesh/JCA_IOT_MESH_Sync.hpp"

#include "Element/JCA_IOT_ELEMENT_Root.hpp"

namespace JCA{ namespace IOT{ namespace MESH{
   
  class cHandler{
    public:
      /***************************************
       * Propertie: MeshNames
       * Info:  Der vector MeshNames verknüpft die MeshID's
       *          mit dem Klartext-Namen der Nodes
       ***************************************/
      std::vector<meshName> MeshNames;
      std::vector<meshName>::iterator MeshNameIt;

      /***************************************
       * Propertie: Elements
       * Info:  Pointer auf den Elements-Vector der
       *          ELEMENTS::Handler-Klasse
       ***************************************/
      std::vector<ELEMENT::cRoot*> *Elements;

      /***************************************
       * Propertie: MeshOut
       * Info:  Das JsonObject MeshOut dient als Zwischenspeicher
       *          für ausgehende Mesh-Telegramme.
       ***************************************/
      JsonObject MeshOut;
      StaticJsonDocument<JCA_IOT_MESH_HANDLER_JSON_BUFSIZE> JDocBuffer;
      StaticJsonDocument<JCA_IOT_MESH_HANDLER_JSON_MSGSIZE> JDocMsg;
      
      char Name[JCA_IOT_ELEMENT_NAME_LEN];
      int32_t HelloCycle;
      cConfig Config;
      cClient Client;
      cSync Sync;
      
      /***************************************
       * Methode: cHandler()
       * Info:  Der Konstruktor initialisiert den Quality-Code
       ***************************************/
      cHandler(){
        MeshOut = JDocBuffer.to<JsonObject>();
        HelloCycle = random(JCA_IOT_MESH_HANDLER_TIMEHELLO, JCA_IOT_MESH_HANDLER_TIMEHELLO + JCA_IOT_MESH_SRV_TIMERANGE);
      }
      
      /***************************************
       * Methode: update(unit32_t DiffMillis)
       * Info:  Die Methore ruf die einzelnen Update-Funktionen
       *          Unterfunktionen auf und versendet die Mesh-Telegramme
       * Parameter:
       *        DiffMillis [uint32_t] vergangene Zeit seit letztem Aufruf.
       *        Mesh [&PainlessMesh] Referenz auf das Mesh-Netzwerk
       * Return Value:
       *        Timestamp [uint32_t]
       ***************************************/
      uint32_t update(uint32_t DiffMillis, painlessMesh &Mesh) {
        uint32_t Timestamp;
        // Update Client, erhält die aktuelle Uhrzeit zurück
        Timestamp = Client.update(Elements, MeshOut, DiffMillis);
        
        // Update Config
        Config.update(DiffMillis, Mesh);
        
        // Update Subcriber
        
        // Hello-Telegramm erzeugen
        HelloCycle -= DiffMillis;
        if (HelloCycle <= 0) {
          // Wenn der Timer abgelaufn ist wird dar Broadcast versendet ...
          sendHello();
          // ... und der Timer initialisiert;
          HelloCycle = random(JCA_IOT_MESH_HANDLER_TIMEHELLO, JCA_IOT_MESH_HANDLER_TIMEHELLO + JCA_IOT_MESH_SRV_TIMERANGE);
        }
        
        // Telegramm-Buffer prüfen und versenden
        sendMsg(Mesh);
        
        return Timestamp;
      }
      
      /***************************************
       * Methode: config(char* ConfigFileName, vector<cRoot*> *Elements, JsonObeject &JConfig, painlessMesh &Mesh)
       * Info:  Konfiguriert die unter Unktionen und
       *          speichert die Mesh-Kennung
       * Parameter:
       *        ConfigFileName [char*] Name/Pfad der Konfigurations-Datei
       *        Elements [vector<cRoot*>] Zeiger auf den Elemet-Vector
       *        JConfig [JsonObect&] Referenz zum Kongfigurations-Objekt
       *        Mesh [painlessMesh&] Referenz zum Mesh-Netzwerk
       ***************************************/
      bool config(const char *ConfigFileName, std::vector<ELEMENT::cRoot*> *InElements, JsonObject& JConfig, painlessMesh &Mesh){
        // Elements Pointer Speichern
        Elements = InElements;
        
        // Konfiguration einlesen
        Config.config(ConfigFileName, Elements, JConfig, Mesh);
        
        // Konfig Subscriber
        // read Subscribe-Elements from Config
        
        // read Push-Elements from Config
        
        return true;
      }
      
      /***************************************
       * Methode: recvMsg(uint32_t from, String &msg)
       * Info:  Verteilt die empfangenen Nachrichten auf die Unterfuktionen
       *        sichert die Node-Namen
       * Parameter:
       *        from [uint32_t] MeshID der Ursprungs-Node
       *        msg [String] empfangene Nachricht
       ***************************************/
      void recvMsg(uint32_t from, String &msg){
        JsonArray MsgArray;
        JsonObject Data;
        bool NodeExists = false;
        
        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_TELEGRAM)
          Serial.print(F("RECV-MESH -> From:"));
          Serial.print(from);
          Serial.print(F(" msg:"));
          Serial.println(msg);
        #endif
        
        //create Msg-Array
        deserializeJson(JDocMsg, msg);
        MsgArray = JDocMsg.as<JsonArray>();
        
        //create Data-Object for each Msg in Array
        for(JsonVariant MsgElement : MsgArray) {
          Data = MsgElement.as<JsonObject>();
          Data["from"] = from;
          #if (DEBUGLEVEL >= JCA_IOT_DEBUG_INTERNAL)
            String DataObjStr;
            serializeJson(Data, DataObjStr);
            Serial.println(F("RECV-MESH -> DataObject"));
            Serial.println(DataObjStr);
          #endif

          
          //switch betwen diffrent types and call subfunction
          switch (Data["msgId"].as<int>()){
            //#####################################
            // Subscriber
            // TODO
            //#####################################
            
            //#####################################
            // Sync
            //#####################################
            //-------------------------
            // Data-Push Msg
            //-------------------------
            case JCA_IOT_MESH_PUSH:
              Sync.recvDataPush(Data, Client, Elements, MeshOut);
              break;
                        
            //-------------------------
            // Data-Request Msg
            //-------------------------
            case JCA_IOT_MESH_DATA_REQUEST:
              Sync.recvDataRequest(Data, Client, Elements, MeshOut);
              // not implemented at the Moment, maybe later if there will be a ServerNode for Error-Output (signal Lamp or Display)
              break;
                        
            //-------------------------
            // Data-Reply Msg
            //-------------------------
            case JCA_IOT_MESH_DATA_REPLY:
              // not implemented at the Moment, sync read only used from HMI
              break;
                        
            //#####################################
            // Config
            // TODO
            //#####################################

            //#####################################
            // Server-Client
            //#####################################
            //-------------------------
            // Server Publish
            //-------------------------
            case JCA_IOT_MESH_SRV_PUBLISH:
              Client.recvSrvPublish(Data);
              break;

            //-------------------------
            // Server Request Msg
            //-------------------------
            case JCA_IOT_MESH_SRV_REQUEST:
              // not implemented at the Moment, maybe later if there will be a ServerNode for Logging or something
              break;

            //-------------------------
            // Archiv Msg
            //-------------------------
            case JCA_IOT_MESH_SRV_ARCHIVDATA:
              // not implemented at the Moment, maybe later if there will be a ServerNode for Archivdata
              break;

            //-------------------------
            // Alarm Msg
            //-------------------------
            case JCA_IOT_MESH_SRV_ALARM:
              // not implemented at the Moment, maybe later if there will be a ServerNode for Alarming (signal Lamp or Display)
              break;

            //-------------------------
            // AlarmAck Msg
            //-------------------------
            case JCA_IOT_MESH_SRV_ALARMACK:
              Client.recvAlarmAck(Elements, Data);
              break;

            //-------------------------
            // Hello Msg
            //-------------------------
            case JCA_IOT_MESH_SRV_HELLO:
              #if (DEBUGLEVEL >= JCA_IOT_DEBUG_TELEGRAM)
                Serial.print(F("  Hello"));
              #endif
              for(MeshNameIt = MeshNames.begin(); MeshNameIt != MeshNames.end(); ++MeshNameIt) {
                if (MeshNameIt->id == from || strcmp(MeshNameIt->name, Data["name"]) == 0) {
                  #if (DEBUGLEVEL >= JCA_IOT_DEBUG_TELEGRAM)
                    Serial.print(F("  Exist:"));
                    Serial.println(Data["name"].as<String>());
                  #endif
                  MeshNameIt->id = from;
                  strncmp(MeshNameIt->name, Data["name"], JCA_IOT_ELEMENT_NAME_LEN);
                  NodeExists = true;
                  break;
                }
              }
              if (!NodeExists) {
                #if (DEBUGLEVEL >= JCA_IOT_DEBUG_TELEGRAM)
                  Serial.print(F("  Add:"));
                  Serial.println(Data["name"].as<String>());
                #endif
                MeshNames.push_back(meshName());
                MeshNames.back().id = from;
                strncmp(MeshNames.back().name, Data["name"], JCA_IOT_ELEMENT_NAME_LEN);
              }
              break;
              
            //-------------------------
            // Data-Push Msg
            //-------------------------
            case JCA_IOT_MESH_SRV_FAILLOG:
              // not implemented at the Moment, maybe later if there will be a ServerNode for Error-Output (signal Lamp or Display)
              break;
                        
          }
        }
      }
      
      /***************************************
       * Methode: sendMsg(painlessMesh &Mesh)
       * Info:  Liest den Nachrichten Puffer aus und sendet
       *          1024 Byte blöcke an die einzelnen Nodes
       * Parameter:
       *        Mesh [painlessMesh&] Referenz zum Mesh-Netzwerk
       ***************************************/
      void sendMsg(painlessMesh &Mesh){
        String OutMsg;
        String OutMsgTmp;
        JsonArray InBuffer;
        // loop alt Destinations in Out-Buffer
        for (JsonPair BufElement : MeshOut) {
          InBuffer = BufElement.value();
          // Out-Nachricht erweitern bis der String max. 1024 Zeichen lang ist
          for (int i = 0; i < InBuffer.size();){
            if (OutMsgTmp.length() == 0){
              OutMsgTmp = "[";
            }else{
              OutMsgTmp += ",";
            }
            // Index in Klartext wandeln
            if (InBuffer[i]["eIdx"]){
              InBuffer[i]["element"] = (*Elements)[InBuffer[i]["eIdx"].as<unsigned char>()]->Name;
            }
            if (InBuffer[i]["tIdx"]){
              InBuffer[i]["tag"] = (*Elements)[InBuffer[i]["eIdx"].as<unsigned char>()]->Archiv[InBuffer[i]["tIdx"].as<unsigned char>()]->Name;
            }
            if (InBuffer[i]["aIdx"]){
              InBuffer[i]["alarm"] = (*Elements)[InBuffer[i]["eIdx"].as<unsigned char>()]->Alarm[InBuffer[i]["aIdx"].as<unsigned char>()]->Name;
            }
            if (InBuffer[i]["dIdx"]){
              InBuffer[i]["data"] = (*Elements)[InBuffer[i]["eIdx"].as<unsigned char>()]->Data[InBuffer[i]["dIdx"].as<unsigned char>()]->Name;
            }
            
            // Node-Name ersetzen
            if (InBuffer[i]["node"]){
              InBuffer[i]["node"] = Config.Name;
            }

            // Node-Name ersetzen
            if (InBuffer[i]["time"]){
              InBuffer[i]["time"] = Client.Timestamp;
            }

            // Source-Node Name als Quelle des fehlerhaften Telegramms im Names-Vector suchen.
            if (InBuffer[i]["srcNodeId"]){
              for(MeshNameIt = MeshNames.begin(); MeshNameIt != MeshNames.end(); ++MeshNameIt) {
                if (MeshNameIt->id == InBuffer[i]["srcNodeId"].as<uint32_t>()){
                  InBuffer[i]["srcNode"] = MeshNameIt->name;
                  break;
                }
              }
            }
            // Localer Node Name des aufgetretenen Fehlers
            if (InBuffer[i]["failNode"]){
              InBuffer[i]["failNode"] = Config.Name;
            }
            
            serializeJson(InBuffer[i], OutMsgTmp);
            if (OutMsgTmp.length() < 300) {
              OutMsg = OutMsgTmp;
              InBuffer.remove(i);
            }else{
              break;
            }
          }
          
          // Output Message created
          if(OutMsg.length() > 0){
              OutMsg += "]";
            // Destination is a Broadcast
            if (BufElement.key() == "broadcast"){
              Mesh.sendBroadcast(OutMsg);
            }
            // Destination is a single Node
            else{
              uint32_t Dest;
              Dest = atoi(BufElement.key().c_str());
              Mesh.sendSingle(Dest, OutMsg);
            }
          }
          
          // Remove JsonPair if no Messages left
          if (InBuffer.size() == 0){
            MeshOut.remove(BufElement.key());
          }
        }
        // Cleanup JsonDocument Storage
        JDocBuffer.garbageCollect();
      }
      
      /***************************************
       * Methode: sendHello()
       * Info:  Erstellt einen Hello-Broadcast und teilst somit
       *          den namen der Node den anderen mit.
       ***************************************/
      void sendHello(){
        JsonArray Broadcasts;
        JsonObject Msg;
        
        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_TELEGRAM)
          Serial.println(F("SEND Hello"));
        #endif

        // Falls noch kein Broadcast Eintrag existiert diesen erstellen
        if (MeshOut.containsKey("broadcast")){
          Broadcasts = MeshOut["broadcast"];
        }else{
          Broadcasts = MeshOut.createNestedArray("broadcast");
        }
        Msg = Broadcasts.createNestedObject();
        Msg["msgId"] = JCA_IOT_MESH_SRV_HELLO;
        Msg["node"] = true;
      }
      
      
  };
}}}

#endif