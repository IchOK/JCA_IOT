/**********************************************
 * Class:	JCA_IOT_MESH_Sync
 * Info: 	Die Klasse handelt alle Funktionen rund
 *        um das direkte Schreiben und Lesen von
 *        Datenpunkten innerhalb der Elemente.
 * Version:
 * 	V1.0.0	Erstellt	02.08.2020	JCA
 *		-recvDataPush
 *    -recvDataRequest
 *    -sendDataReply
 **********************************************/

#ifndef _JCA_IOT_MESH_SYNC_H
#define _JCA_IOT_MESH_SYNC_H

//Include extrenal
#include <map>
#include <vector>
#include <ArduinoJson.h>

//Elemente einbinden für Update
#include "JCA_IOT_Debug.h"

#include "Element/JCA_IOT_ELEMENT_Root.hpp"
#include "Element/JCA_IOT_ELEMENT_define.h"
#include "Mesh/JCA_IOT_MESH_define.h"
#include "Mesh/JCA_IOT_MESH_types.h"
#include "Mesh/JCA_IOT_MESH_Client.hpp"

namespace JCA{ namespace IOT{ namespace MESH{
  class cSync{
    public:
      /***************************************
       * Methode: cSync()
       * Info:  Der Konstruktor initialisiert Server-Vektoren
       *        und Status
       ***************************************/
      cSync(){
      }
    
      /***************************************
       * Methode: recvDataPush(JsonObject Data, cClient& Client, std::vector<ELEMENT::cRoot*> *Elements, JsonObject &MeshOut)
       * Info:  Funktion zum empfangen und verarbeiten eines
       *        Data-Push telegramm. Schreibt die Daten direkt 
       *        in den Datenpunkt des Element.
       * Parameter:
       *        Data [JsonObject]
       *            Enthält das empfangene Telegram
       *        Client [JCA::IOT::MESH::cClient]
       *            Stellt die Schnittstelle für Fehlerlogs zur Verfügung
       *        Elements [vector<ELEMENT::cRoot*>]
       *            Vector mit allen Elementen der Node
       *        MeshOut [JsonObject&]
       *            Puffer für ausgehende Mesh-Telegramme
       ***************************************/
      void recvDataPush(JsonObject Data, cClient& Client, std::vector<ELEMENT::cRoot*> *Elements, JsonObject &MeshOut){
        unsigned char error;
        int e;
        int d;
        
        strncpy(DstElement, Data["dstElement"], JCA_IOT_ELEMENT_NAME_LEN);
        strncpy(DstData, Data["dstElement"], JCA_IOT_ELEMENT_NAME_LEN);
        
        error = JCA_IOT_MESH_ERROR_PUSH_ELEMENT;
        // Elemente nach dem Namen durchsuchen
        for (e = 0; e < Elements.size(); e++){
          if(strcmp(Elements[e]->Name, Data["dstElement"].as<char*>) == 0){
            // Das Element wurde gefunden
            error = JCA_IOT_MESH_ERROR_PUSH_DATA;
            // .. im Element den Datenpunkt suchen
            for (d = 0; d < Elements[e]->Data.size(); d++){
              if(strcmp(Elements[e]->Data[d]->Name, Data["dstData"].as<char*>) == 0){
                // Es wurde ein passender Datenpunkt gefunden
                error = JCA_IOT_MESH_ERROR_NONE;
                // Daten anhand des Datentype schreiben
                switch(Elements[e]->Data[d]->Type){
                  case JCA_IOT_ELEMENT_DATA_BOOL:
                    Elements[e]->setDataBool(d, Data["value"].as<bool>);
                    break;
                  case JCA_IOT_ELEMENT_DATA_INT:
                    Elements[e]->setDataInt(d, Data["value"].as<int32_t>);
                    break;
                  case JCA_IOT_ELEMENT_DATA_FLOAT:
                    Elements[e]->setDataFloat(d, Data["value"].as<float>);
                    break;
                }
                break;
              }
            }
            break;
          }
        }
        
        // Falls kein passender Datenpunkt gefunden wurde wird ein Fehlerbericht gesendet
        if(error){
          iError ErrorReport;
          
          ErrorReport.type = JCA_IOT_MESH_ERROR_PUSH
          ErrorReport.data["srcNodeId"] = Data["from"];
          ErrorReport.data["dstNode"] = "";
          ErrorReport.data["dstElement"] = Data["dstElement"];
          ErrorReport.data["dstData"] = Data["dstData"];
          ErrorReport.data["value"] = Data["value"];
          ErrorReport.data["error"] = error;

          sendError(MeshOut, ErrorReport);
        }
      }
      
      /***************************************
       * Methode: recvAlarmAck(std::vector<ELEMENT::cRoot*> Elements, JsonObject Data)
       * Info:  Funktion zum empfangen und verarbeiten einer
       *        Alarm-Quittierung
       *        Alarm.State auf Ack/Idle setzen
       ***************************************/
      void recvAlarmAck(std::vector<ELEMENT::cRoot*> *Elements, JsonObject Data){
        unsigned char e = Data["eIdx"].as<unsigned char>();
        unsigned char i = Data["aIdx"].as<unsigned char>();
        // Alarm-Status prüfen ...
        switch ((*Elements)[e]->Alarm[i]->State) {
          case JCA_IOT_ELEMENT_ALARM_STATE_COMESEND:
            // ... der Status ist gekommen und wurde bestätigt
            if (Data["state"] == JCA_IOT_ELEMENT_ALARM_STATE_COME) {
              // ... dann wird er auf empfangen gesetzt
              (*Elements)[e]->Alarm[i]->State = JCA_IOT_ELEMENT_ALARM_STATE_COMEACK;
            }
          case JCA_IOT_ELEMENT_ALARM_STATE_GONESEND:
            // ... der Status ist gegangen und wurde bestätigt
            if (Data["state"] == JCA_IOT_ELEMENT_ALARM_STATE_GONE) {
              // ... dann wird er auf inaktiv gesetzt
              (*Elements)[e]->Alarm[i]->State = JCA_IOT_ELEMENT_ALARM_STATE_IDLE;
            }
        }
      }
      
      /***************************************
       * Methode: sendError(JsonObject MeshOut, error Data)
       * Info:  Funktion zur Erstellung eines Archiv Telegramms
       *        Fügt dem MeshOut-Buffer eine Archiv-Nachricht für
       *        jeden Server hinzu.
       ***************************************/
      bool sendError(JsonObject &MeshOut, iError Data){
        JsonArray Server;
        JsonObject Msg;
        char srvId[11];
        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_DIAG)
          Serial.print(F("SEND ERROR - type:"));
          Serial.println(Data.type);
        #endif
        // Alle erfallsten Server durchlaufen
        for (int srv = 0; srv < LogServers.size(); srv++){
          sprintf(srvId, "%d", LogServers[srv].id);
          // Falls noch kein Eintrag für die NodeId existiert, diese erstellen
          if (MeshOut.containsKey(srvId)){
            Server = MeshOut[srvId];
          }else{
            Server = MeshOut.createNestedArray(srvId);
          }
          Msg = Server.createNestedObject();
          Msg["msgId"] = JCA_IOT_MESH_SRV_FAILLOG;
          Msg["type"] = Data.type;
          Msg["data"] = Data.data;
        }
        return true;
      }

      /***************************************
       * Methode: sendArchivData(JsonObject MeshOut, archivData Data)
       * Info:  Funktion zur Erstellung eines Archiv Telegramms
       *        Fügt dem MeshOut-Buffer eine Archiv-Nachricht für
       *        jeden Server hinzu.
       ***************************************/
      bool sendArchivData(JsonObject &MeshOut, iArchivData Data){
        JsonArray Server;
        JsonObject Msg;
        char srvId[11];
        // Alle erfallsten Server durchlaufen
        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_LOOP)
          Serial.print(F("  CountArchivServer:"));
          Serial.println(ArchivServers.size());
        #endif
        for (int srv = 0; srv < ArchivServers.size(); srv++){
          sprintf(srvId, "%d", ArchivServers[srv].id);
          // Falls noch kein Eintrag für die NodeId existiert, diese erstellen
          if (MeshOut.containsKey(srvId)){
            Server = MeshOut[srvId];
          }else{
            Server = MeshOut.createNestedArray(srvId);
          }
          Msg = Server.createNestedObject();
          Msg["msgId"] = JCA_IOT_MESH_SRV_ARCHIVDATA;
          Msg["node"] = true;
          Msg["time"] = Data.timestamp;
          Msg["eIdx"] = Data.elementIndex;
          Msg["tIdx"] = Data.archivIndex;
          Msg["value"] = Data.value;
          Msg["type"] = Data.trigger;
        }
        return true;
      }
      
      /***************************************
       * Methode: sendAlarm(JsonObject MeshOut, alarm Data)
       * Info:  Funktion zur Erstellung eines Alarm Telegramms
       *        Fügt dem MeshOut-Buffer eine Alarm-Nachricht für
       *        jeden Server hinzu.
       ***************************************/
      bool sendAlarm(JsonObject &MeshOut, iAlarm Data){
        JsonArray Server;
        JsonObject Msg;
        char srvId[11];
        // Alle erfallsten Server durchlaufen
        for (int srv = 0; srv < AlarmServers.size(); srv++){
          sprintf(srvId, "%d", AlarmServers[srv].id);
          // Falls noch kein Eintrag für die NodeId existiert, diese erstellen
          if (MeshOut.containsKey(srvId)){
            Server = MeshOut[srvId];
          }else{
            Server = MeshOut.createNestedArray(srvId);
          }
          Msg = Server.createNestedObject();
          Msg["msgId"] = JCA_IOT_MESH_SRV_ALARM;
          Msg["time"] = Data.timestamp;
          Msg["node"] = true;
          Msg["prio"] = Data.prio;
          Msg["text"] = Data.text;
          Msg["eIdx"] = Data.elementIndex;
          Msg["aIdx"] = Data.alarmIndex;
          Msg["state"] = Data.state;
        }
        return true;
      }
      
    private:
      clientState State;
      int32_t reqTimer;
      uint32_t Timestamp;
      uint32_t TimeMillis;
      std::vector<serverState> LogServers;
      std::vector<serverState> AlarmServers;
      std::vector<serverState> ArchivServers;
      
      /***************************************
       * Methode: sendSrvRequest(JsonObject MeshOut)
       * Info:  Funktion zur Erstellung eines Server-Request
       *        Fügt dem MeshOut-Buffer eine Broadcast-Nachricht hinzu
       ***************************************/
      bool sendSrvRequest(JsonObject &MeshOut){
        JsonArray Broadcasts;
        JsonObject Msg;
        
        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_TELEGRAM)
          Serial.println(F("SEND SRV REQUEST"));
        #endif

        // Sync-Status aktuallisieren
        State = cltRequesting;
        
        // Falls noch kein Broadcast Eintrag existiert diesen erstellen
        if (MeshOut.containsKey("broadcast")){
          Broadcasts = MeshOut["broadcast"];
        }else{
          Broadcasts = MeshOut.createNestedArray("broadcast");
        }
        Msg = Broadcasts.createNestedObject();
        Msg["msgId"] = JCA_IOT_MESH_SRV_REQUEST;
        return true;
      }
      
  };
}}}
#endif