/**********************************************
 * Class:	JCA_IOT_MESH_Client
 * Info: 	Die Klasse handelt alle Funktionen rund
 *        um die Server-Telegramme (Fehler-Logs,
 *        Archiv-Daten, Alarme, ...)
 * Version:
 * 	V1.0.0	Erstellt	12.06.2020	JCA
 *		-recvSrvPublish
 *    -sendSrvRequest
 *    -sendArchiData
 *    -sendAlarm
 *    -sendError
 *    -update
 **********************************************/

#ifndef _JCA_IOT_MESH_CLIENT_H
#define _JCA_IOT_MESH_CLIENT_H

//Include extrenal
#include <map>
#include <vector>
#include <ArduinoJson.h>
//Elemente einbinden für Update
#include "Element/JCA_IOT_ELEMENT_Root.hpp"

#include "Element/JCA_IOT_ELEMENT_define.h"
#include "Mesh/JCA_IOT_MESH_define.h"
#include "Mesh/JCA_IOT_MESH_types.h"

namespace JCA{ namespace IOT{ namespace MESH{
  class cClient{
    public:
      /***************************************
       * Methode: cClient()
       * Info:  Der Konstruktor initialisiert Server-Vektoren
       *        und Status
       ***************************************/
      cClient(){
        State = cltInit;
        reqTimer = random(JCA_IOT_MESH_SRV_TIMEREQ, JCA_IOT_MESH_SRV_TIMEREQ + JCA_IOT_MESH_SRV_TIMERANGE);
      }
    
      /***************************************
       * Methode: update(Elements, &MeshOut, DiffMillis, Timestamp))
       * Info:  Funktion durchsucht die Elemente nach
       *        Client-Nachrichten und handelt die
       *        Server-Listen.
       * Parameter:
       *        Elements [vector<ELEMENT::cRoot*>]
       *            Vector mit allen Elementen der Node
       *        MeshOut [JsonObject&]
       *            Puffer für ausgehende Mesh-Telegramme
       *        DiffMillis [uint32_t]
       *            Vergangene Zeit seit letztem Aufruf
       *        Timestamp [uint32_t]
       *            Aktueller Zeitstempel
       ***************************************/
      uint32_t update(std::vector<ELEMENT::cRoot*> *Elements, JsonObject &MeshOut, uint32_t DiffMillis){
        int i;
        std::vector<serverState>::iterator srv;
        iArchivData archData;
        iAlarm almData;
        #if DEBUGLEVEL >= 9
          Serial.println(F("CLIENT update"));
        #endif
        
        // Uhrzeit
        TimeMillis += DiffMillis;
        if (TimeMillis > 1000) {
          Timestamp += 1;
          TimeMillis -= 1000;
        }
        
        // Elemente durchlaufen
        for (int e = 0; e < Elements->size(); e++){
          //-----------------------------------------------------------------------------------------------------------
          // CHECK ARCHIVS
          for (i = 0; i < (*Elements)[e]->Archiv.size(); i++){
            // Wenn der Archiv-Trigger gesetzt wurde ...
            if ((*Elements)[e]->Archiv[i]->Trigger != 0) {
              // ... wird das Telegram versendet und der Trigger gelöscht
              #if DEBUGLEVEL >= 2
                Serial.print(F("SEND ARCHIV - "));
                Serial.println((*Elements)[e]->Archiv[i]->Name);
              #endif
              archData.timestamp = (*Elements)[e]->Archiv[i]->Timestamp;
              archData.elementIndex = e;
              archData.archivIndex = i;
              archData.trigger = (*Elements)[e]->Archiv[i]->Trigger;
              archData.value = (*Elements)[e]->Archiv[i]->Value;
              if (sendArchivData(MeshOut, archData)){
                (*Elements)[e]->Archiv[i]->Trigger = 0;
              }
            }
          }
          //-----------------------------------------------------------------------------------------------------------
          // CHECK ALARMS
          for (i = 0; i < (*Elements)[e]->Alarm.size(); i++){
            // Alarm-Status prüfen ...
            switch ((*Elements)[e]->Alarm[i]->State) {
              case JCA_IOT_ELEMENT_ALARM_STATE_COME:
                // ... ist der Alarm gekommen wird, versenden und Status setzen
                #if DEBUGLEVEL >= 2
                  Serial.print(F("SEND ALARM-COME - "));
                  Serial.println((*Elements)[e]->Alarm[i]->Name);
                #endif
                almData.timestamp = (*Elements)[e]->Alarm[i]->TimestampCome;
                almData.text = (*Elements)[e]->Alarm[i]->Text;
                almData.prio = (*Elements)[e]->Alarm[i]->Prio;
                almData.elementIndex = e;
                almData.alarmIndex = i;
                almData.state = JCA_IOT_ELEMENT_ALARM_STATE_COME;
                if (sendAlarm(MeshOut, almData)){
                  (*Elements)[e]->Alarm[i]->State = JCA_IOT_ELEMENT_ALARM_STATE_COMESEND;
                }
                break;
                
              case JCA_IOT_ELEMENT_ALARM_STATE_GONE:
                // ... ist der Alarm gekommen wird, versenden und Status setzen
                #if DEBUGLEVEL >= 2
                  Serial.print(F("SEND ALARM-GONE - "));
                  Serial.println((*Elements)[e]->Alarm[i]->Name);
                #endif
                almData.timestamp = (*Elements)[e]->Alarm[i]->TimestampGone;
                almData.text = (*Elements)[e]->Alarm[i]->Text;
                almData.prio = (*Elements)[e]->Alarm[i]->Prio;
                almData.elementIndex = e;
                almData.alarmIndex = i;
                almData.state = JCA_IOT_ELEMENT_ALARM_STATE_GONE;
                if (sendAlarm(MeshOut, almData)){
                  (*Elements)[e]->Alarm[i]->State = JCA_IOT_ELEMENT_ALARM_STATE_GONESEND;
                }
                break;
            }
          }
        }
        //-----------------------------------------------------------------------------------------------------------
        // SERVER WATCHDOGS
        
        // Logging-Server
        for(srv = LogServers.begin(); srv != LogServers.end(); /*++srv*/) {
          srv->wd -= DiffMillis;
          // Wenn der Timeout überschritten ist wird der Server aus der Liste entfernt
          if (srv->wd <= 0){
            #if DEBUGLEVEL >= 2
              Serial.print(F("REMOVE LOGSRV - "));
              Serial.println(srv->id);
            #endif
            srv = LogServers.erase(srv);
          }else{
            ++srv;
          }
        }
        // Server-Request versenden , falls kein Logging-Server verfürbar ist
        if (LogServers.size() == 0){
          reqTimer -= DiffMillis;
          #if DEBUGLEVEL >= 9
            Serial.print(F("  LOGSRV REQTIMER - "));
            Serial.println(reqTimer);
          #endif
          if (reqTimer <= 0){
            sendSrvRequest(MeshOut);
            reqTimer = random(JCA_IOT_MESH_SRV_TIMEREQ, JCA_IOT_MESH_SRV_TIMEREQ + JCA_IOT_MESH_SRV_TIMERANGE);
            #if DEBUGLEVEL >= 2
              Serial.print(F("  Next "));
              Serial.print(reqTimer);
              serializeJsonPretty(MeshOut, Serial);
            #endif
          }
        }
        State = cltOnline;
        // Alarming-Server
        for(srv = AlarmServers.begin(); srv != AlarmServers.end(); /*++srv*/) {
          srv->wd -= DiffMillis;
          // Wenn der Timeout überschritten ist wird der Server aus der Liste entfernt
          if (srv->wd <= 0){
            #if DEBUGLEVEL >= 2
              Serial.print(F("REMOVE ALMSRV - "));
              Serial.println(srv->id);
            #endif
            srv = AlarmServers.erase(srv);
          }else{
            ++srv;
          }
        }
        // Archiv-Server
        for(srv = ArchivServers.begin(); srv != ArchivServers.end(); /*++srv*/) {
          srv->wd -= DiffMillis;
          // Wenn der Timeout überschritten ist wird der Server aus der Liste entfernt
          if (srv->wd <= 0){
            #if DEBUGLEVEL >= 2
              Serial.print(F("REMOVE ARCSRV - "));
              Serial.println(srv->id);
            #endif
            srv = ArchivServers.erase(srv);
          }else{
            ++srv;
          }
        }
      }
      
      /***************************************
       * Methode: recvSrvPublish(JsonObject Data)
       * Info:  Funktion zum empfangen und verarbeiten eines
       *        Server-Publish telegramm.
       *        Status Updaten
       *        Server-Listen aktuallisieren
       ***************************************/
      void recvSrvPublish(JsonObject Data){
        bool notFound;
        uint32_t id;
        std::vector<serverState>::iterator srv;
        id = Data["from"];
        #if DEBUGLEVEL >= 2
          Serial.print(F("RECV SRVPUBLISH - "));
          Serial.print(id);
        #endif
        // Uhrzeit stellen
        if (Data["msg"].containsKey("time")){
          Timestamp = Data["msg"]["time"];
          TimeMillis = 0;
        }
        
        // Zu Logging-Server-Liste hinzufügen oder watchdog zurück setzen
        if (Data["msg"]["logging"]){
          #if DEBUGLEVEL >= 2
            Serial.print(F(" -logging"));
          #endif
          // Client-Status aktuallisieren
          State = cltOnline;
          
          // Server-Liste durchsuchen
          notFound = true;
          for(srv = LogServers.begin(); srv != LogServers.end(); ++srv) {
            if (srv->id == id){
              // Watchdog zurücksetzen
              srv->wd = random(JCA_IOT_MESH_SRV_TIMEWD, JCA_IOT_MESH_SRV_TIMEWD + JCA_IOT_MESH_SRV_TIMERANGE);
              notFound = false;
              break;
            }
          }
          if (notFound){
            LogServers.push_back(serverState());
            LogServers.back().id = id;
            LogServers.back().wd = random(JCA_IOT_MESH_SRV_TIMEWD, JCA_IOT_MESH_SRV_TIMEWD + JCA_IOT_MESH_SRV_TIMERANGE);
          }
        }
        
        // Zu Alarming-Server-Liste hinzufügen oder watchdog zurück setzen
        if (Data["msg"]["alarming"]){
          #if DEBUGLEVEL >= 2
            Serial.print(F(" -alarming"));
          #endif
          notFound = true;
          for(srv = AlarmServers.begin(); srv != AlarmServers.end(); ++srv) {
            if (srv->id == id){
              // Watchdog zurücksetzen
              srv->wd = random(JCA_IOT_MESH_SRV_TIMEWD, JCA_IOT_MESH_SRV_TIMEWD + JCA_IOT_MESH_SRV_TIMERANGE);
              notFound = false;
              break;
            }
          }
          if (notFound){
            AlarmServers.push_back(serverState());
            AlarmServers.back().id = id;
            AlarmServers.back().wd = random(JCA_IOT_MESH_SRV_TIMEWD, JCA_IOT_MESH_SRV_TIMEWD + JCA_IOT_MESH_SRV_TIMERANGE);
          }
        }
        
        // Zu Archiv-Server-Liste hinzufügen oder watchdog zurück setzen
        if (Data["msg"]["archiv"]){
          #if DEBUGLEVEL >= 2
            Serial.print(F(" -archiv"));
          #endif
          notFound = true;
          for(srv = ArchivServers.begin(); srv != ArchivServers.end(); ++srv) {
            if (srv->id == id){
              // Watchdog zurücksetzen
              srv->wd = random(JCA_IOT_MESH_SRV_TIMEWD, JCA_IOT_MESH_SRV_TIMEWD + JCA_IOT_MESH_SRV_TIMERANGE);
              notFound = false;
              break;
            }
          }
          if (notFound){
            ArchivServers.push_back(serverState());
            ArchivServers.back().id = id;
            ArchivServers.back().wd = random(JCA_IOT_MESH_SRV_TIMEWD, JCA_IOT_MESH_SRV_TIMEWD + JCA_IOT_MESH_SRV_TIMERANGE);
          }
        }
        #if DEBUGLEVEL >= 2
          Serial.println("");
        #endif
      }
      
      /***************************************
       * Methode: recvAlarmAck(std::vector<ELEMENT::cRoot*> Elements, JsonObject Data)
       * Info:  Funktion zum empfangen und verarbeiten einer
       *        Alarm-Quittierung
       *        Alarm.State auf Ack/Idle setzen
       ***************************************/
      void recvAlarmAck(std::vector<ELEMENT::cRoot*> *Elements, JsonObject Data){
        unsigned char e = Data["msg"]["eIdx"].as<unsigned char>();
        unsigned char i = Data["msg"]["aIdx"].as<unsigned char>();
        // Alarm-Status prüfen ...
        switch ((*Elements)[e]->Alarm[i]->State) {
          case JCA_IOT_ELEMENT_ALARM_STATE_COMESEND:
            // ... der Status ist gekommen und wurde bestätigt
            if (Data["msg"]["state"] == JCA_IOT_ELEMENT_ALARM_STATE_COME) {
              // ... dann wird er auf empfangen gesetzt
              (*Elements)[e]->Alarm[i]->State = JCA_IOT_ELEMENT_ALARM_STATE_COMEACK;
            }
          case JCA_IOT_ELEMENT_ALARM_STATE_GONESEND:
            // ... der Status ist gegangen und wurde bestätigt
            if (Data["msg"]["state"] == JCA_IOT_ELEMENT_ALARM_STATE_GONE) {
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
        #if DEBUGLEVEL >= 2
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
          Msg["time"] = Data.timestamp;
          Msg["element"] = Data.elementIndex;
          Msg["archiv"] = Data.archivIndex;
          Msg["value"] = Data.value;
          Msg["trigger"] = Data.trigger;
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
          Msg["text"] = Data.text;
          Msg["prio"] = Data.prio;
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
        
        #if DEBUGLEVEL >= 2
          Serial.println(F("SEND SRV REQUEST"));
        #endif

        // Client-Status aktuallisieren
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