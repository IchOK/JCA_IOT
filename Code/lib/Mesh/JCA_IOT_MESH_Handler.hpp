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
 **********************************************/

#ifndef _JCA_IOT_MESH_HANDLER_H
#define _JCA_IOT_MESH_HANDLER_H

//Include extrenal
#include <vector>
#include <ArduinoJson.h>

#include "Mesh/JCA_IOT_MESH_define.h"
#include "Mesh/JCA_IOT_MESH_types.h"
#include "Mesh/JCA_IOT_MESH_Client.hpp"
#include "Mesh/JCA_IOT_MESH_Config.hpp"

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
      bool config(const char *ConfigFileName, std::vector<ELEMENT::cRoot*> *Elements, JsonObject& JConfig, painlessMesh &Mesh){
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
        JsonObject Data;
        bool NodeExists = false;
        
        //create Data-Object
        deserializeJson(JDocMsg, msg);
        Data = JDocMsg.as<JsonObject>();
        Data["from"] = from;
        
        //switch betwen diffrent types and call subfunction
        switch (Data["msgId"].as<int>()){
          
          // Hello Msg
          case JCA_IOT_MESH_SRV_HELLO:
            for(MeshNameIt = MeshNames.begin(); MeshNameIt != MeshNames.end(); ++MeshNameIt) {
              if (MeshNameIt->id == from || strcmp(MeshNameIt->name, Data["name"]) == 0) {
                MeshNameIt->id = from;
                strncmp(MeshNameIt->name, Data["name"], JCA_IOT_ELEMENT_NAME_LEN);
                NodeExists = true;
              }
            }
            if (!NodeExists) {
              MeshNames.push_back(meshName());
              MeshNames.back().id = from;
              strncmp(MeshNames.back().name, Data["name"], JCA_IOT_ELEMENT_NAME_LEN);
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
          // Outnachricht erweitern bis der String max. 1024 Zeichen lang ist

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
      }
      
      /***************************************
       * Methode: sendHello()
       * Info:  Erstellt einen Hello-Broadcast und teilst somit
       *          den namen der Node den anderen mit.
       ***************************************/
      void sendHello(){
        JsonArray Broadcasts;
        JsonObject Msg;
        
        #if DEBUGLEVEL >= 2
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
        Msg["name"] = Config.Name;
      }
      
      
  };
}}}

#endif