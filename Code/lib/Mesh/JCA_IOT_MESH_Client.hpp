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

//Elemente einbinden f�r Update
#include "JCA_IOT_Debug.h"

#include "Element/JCA_IOT_ELEMENT_Root.hpp"
#include "Element/JCA_IOT_ELEMENT_define.h"
#include "Mesh/JCA_IOT_MESH_define.h"
#include "Mesh/JCA_IOT_MESH_types.h"

namespace JCA{ namespace IOT{ namespace MESH{
  class cClient{
    public:
      uint32_t Timestamp;

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
       *            Puffer f�r ausgehende Mesh-Telegramme
       *        DiffMillis [uint32_t]
       *            Vergangene Zeit seit letztem Aufruf
       * ReturnValue:
       *        Timestamp [uint32_t]
       *            Aktueller Zeitstempel
       ***************************************/
      uint32_t update(std::vector<ELEMENT::cRoot*> *Elements, JsonObject &MeshOut, uint32_t DiffMillis){
        int i;
        std::vector<serverState>::iterator srv;
        iArchivData archData;
        iAlarm almData;
        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_LOOP)
          Serial.println(F("CLIENT update"));
        #endif
        
        // Uhrzeit
        TimeMillis += DiffMillis;
        if (TimeMillis > 1000) {
          Timestamp += 1;
          TimeMillis -= 1000;
        }
        
        // Elemente durchlaufen
        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_LOOP)
          Serial.print(F("  Element:"));
          Serial.println(Elements->size());
        #endif
        for (int e = 0; e < Elements->size(); e++){
          #if (DEBUGLEVEL >= JCA_IOT_DEBUG_LOOP)
            Serial.printf("    %i:", e+1);
          #endif
          //-----------------------------------------------------------------------------------------------------------
          // CHECK ARCHIVS
          #if (DEBUGLEVEL >= JCA_IOT_DEBUG_LOOP)
            Serial.printf(" Archives:%i", (*Elements)[e]->Archiv.size());
          #endif
          for (i = 0; i < (*Elements)[e]->Archiv.size(); i++){
            // Wenn der Archiv-Trigger gesetzt wurde ...
            #if (DEBUGLEVEL >= JCA_IOT_DEBUG_LOOP)
              Serial.printf("[%i]", (*Elements)[e]->Archiv[i]->Trigger);
            #endif
            if ((*Elements)[e]->Archiv[i]->Trigger != 0) {
              // ... wird das Telegram versendet und der Trigger gel�scht
              #if (DEBUGLEVEL >= JCA_IOT_DEBUG_TELEGRAM)
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
          #if (DEBUGLEVEL >= JCA_IOT_DEBUG_LOOP)
            Serial.printf(" Alarms:%i", (*Elements)[e]->Alarm.size());
          #endif
          for (i = 0; i < (*Elements)[e]->Alarm.size(); i++){
            // Alarm-Status pr�fen ...
            switch ((*Elements)[e]->Alarm[i]->State) {
              case JCA_IOT_ELEMENT_ALARM_STATE_COME:
                // ... ist der Alarm gekommen wird, versenden und Status setzen
                #if (DEBUGLEVEL >= JCA_IOT_DEBUG_TELEGRAM)
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
                #if (DEBUGLEVEL >= JCA_IOT_DEBUG_TELEGRAM)
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
          #if (DEBUGLEVEL >= JCA_IOT_DEBUG_LOOP)
            Serial.println("");
          #endif
          }
        }
        //-----------------------------------------------------------------------------------------------------------
        // SERVER WATCHDOGS
        
        // Logging-Server
        for(srv = LogServers.begin(); srv != LogServers.end(); /*++srv*/) {
          srv->wd -= DiffMillis;
          // Wenn der Timeout �berschritten ist wird der Server aus der Liste entfernt
          if (srv->wd <= 0){
            #if (DEBUGLEVEL >= JCA_IOT_DEBUG_WATCHDOG)
              Serial.print(F("REMOVE LOGSRV - "));
              Serial.println(srv->id);
            #endif
            srv = LogServers.erase(srv);
          }else{
            ++srv;
          }
        }
        // Server-Request versenden , falls kein Logging-Server verf�rbar ist
        if (LogServers.size() == 0){
          reqTimer -= DiffMillis;
          #if (DEBUGLEVEL >= JCA_IOT_DEBUG_LOOP)
            Serial.print(F("LOGSRV REQTIMER - "));
            Serial.println(reqTimer);
          #endif
          if (reqTimer <= 0){
            sendSrvRequest(MeshOut);
            reqTimer = random(JCA_IOT_MESH_SRV_TIMEREQ, JCA_IOT_MESH_SRV_TIMEREQ + JCA_IOT_MESH_SRV_TIMERANGE);
            #if (DEBUGLEVEL >= JCA_IOT_DEBUG_TELEGRAM)
              Serial.print(F("  Next "));
              Serial.println(reqTimer);
            #endif
          }
        }
        State = cltOnline;
        // Alarming-Server
        for(srv = AlarmServers.begin(); srv != AlarmServers.end(); /*++srv*/) {
          srv->wd -= DiffMillis;
          // Wenn der Timeout �berschritten ist wird der Server aus der Liste entfernt
          if (srv->wd <= 0){
            #if (DEBUGLEVEL >= JCA_IOT_DEBUG_WATCHDOG)
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
          // Wenn der Timeout �berschritten ist wird der Server aus der Liste entfernt
          if (srv->wd <= 0){
            #if (DEBUGLEVEL >= JCA_IOT_DEBUG_WATCHDOG)
              Serial.print(F("REMOVE ARCSRV - "));
              Serial.println(srv->id);
            #endif
            srv = ArchivServers.erase(srv);
          }else{
            ++srv;
          }
        }
        return Timestamp;
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
        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_TELEGRAM)
          Serial.print(F("RECV SRVPUBLISH - "));
          Serial.print(id);
        #endif
        // Uhrzeit stellen
        if (Data.containsKey("time")){
          Timestamp = Data["time"];
          TimeMillis = 0;
        }
        
        // Zu Logging-Server-Liste hinzuf�gen oder watchdog zur�ck setzen
        if (Data["logging"]){
          #if (DEBUGLEVEL >= JCA_IOT_DEBUG_TELEGRAM)
            Serial.print(F(" -logging"));
          #endif
          // Client-Status aktuallisieren
          State = cltOnline;
          
          // Server-Liste durchsuchen
          notFound = true;
          for(srv = LogServers.begin(); srv != LogServers.end(); ++srv) {
            if (srv->id == id){
              // Watchdog zur�cksetzen
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
        
        // Zu Alarming-Server-Liste hinzuf�gen oder watchdog zur�ck setzen
        if (Data["alarming"]){
          #if (DEBUGLEVEL >= JCA_IOT_DEBUG_TELEGRAM)
            Serial.print(F(" -alarming"));
          #endif
          notFound = true;
          for(srv = AlarmServers.begin(); srv != AlarmServers.end(); ++srv) {
            if (srv->id == id){
              // Watchdog zur�cksetzen
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
        
        // Zu Archiv-Server-Liste hinzuf�gen oder watchdog zur�ck setzen
        if (Data["archiv"]){
          #if (DEBUGLEVEL >= JCA_IOT_DEBUG_TELEGRAM)
            Serial.print(F(" -archiv"));
          #endif
          notFound = true;
          for(srv = ArchivServers.begin(); srv != ArchivServers.end(); ++srv) {
            if (srv->id == id){
              // Watchdog zur�cksetzen
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
        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_TELEGRAM)
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
        unsigned char e = Data["eIdx"].as<unsigned char>();
        unsigned char i = Data["aIdx"].as<unsigned char>();
        // Alarm-Status pr�fen ...
        switch ((*Elements)[e]->Alarm[i]->State) {
          case JCA_IOT_ELEMENT_ALARM_STATE_COMESEND:
            // ... der Status ist gekommen und wurde best�tigt
            if (Data["state"] == JCA_IOT_ELEMENT_ALARM_STATE_COME) {
              // ... dann wird er auf empfangen gesetzt
              (*Elements)[e]->Alarm[i]->State = JCA_IOT_ELEMENT_ALARM_STATE_COMEACK;
            }
          case JCA_IOT_ELEMENT_ALARM_STATE_GONESEND:
            // ... der Status ist gegangen und wurde best�tigt
            if (Data["state"] == JCA_IOT_ELEMENT_ALARM_STATE_GONE) {
              // ... dann wird er auf inaktiv gesetzt
              (*Elements)[e]->Alarm[i]->State = JCA_IOT_ELEMENT_ALARM_STATE_IDLE;
            }
        }
      }
      
      /***************************************
       * Methode: sendError(JsonObject MeshOut, error Data)
       * Info:  Funktion zur Erstellung eines Archiv Telegramms
       *        F�gt dem MeshOut-Buffer eine Archiv-Nachricht f�r
       *        jeden Server hinzu.
       ***************************************/
      bool sendError(JsonObject &MeshOut, iError Data){
        JsonArray Server;
        JsonObject Msg;
        JsonObject ErrData;
        char srvId[11];
        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_DIAG)
          Serial.print(F("SEND ERROR - type:"));
          Serial.println(Data.type);
        #endif
        // Alle erfallsten Server durchlaufen
        for (int srv = 0; srv < LogServers.size(); srv++){
          sprintf(srvId, "%d", LogServers[srv].id);
          // Falls noch kein Eintrag f�r die NodeId existiert, diese erstellen
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
       *        F�gt dem MeshOut-Buffer eine Archiv-Nachricht f�r
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
          // Falls noch kein Eintrag f�r die NodeId existiert, diese erstellen
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
       *        F�gt dem MeshOut-Buffer eine Alarm-Nachricht f�r
       *        jeden Server hinzu.
       ***************************************/
      bool sendAlarm(JsonObject &MeshOut, iAlarm Data){
        JsonArray Server;
        JsonObject Msg;
        char srvId[11];
        // Alle erfallsten Server durchlaufen
        for (int srv = 0; srv < AlarmServers.size(); srv++){
          sprintf(srvId, "%d", AlarmServers[srv].id);
          // Falls noch kein Eintrag f�r die NodeId existiert, diese erstellen
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
      uint32_t TimeMillis;
      std::vector<serverState> LogServers;
      std::vector<serverState> AlarmServers;
      std::vector<serverState> ArchivServers;
      
      /***************************************
       * Methode: sendSrvRequest(JsonObject MeshOut)
       * Info:  Funktion zur Erstellung eines Server-Request
       *        F�gt dem MeshOut-Buffer eine Broadcast-Nachricht hinzu
       ***************************************/
      bool sendSrvRequest(JsonObject &MeshOut){
        JsonArray Broadcasts;
        JsonObject Msg;
        
        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_TELEGRAM)
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