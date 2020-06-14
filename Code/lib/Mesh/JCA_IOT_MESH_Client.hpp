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
#include "JCA_IOT_ELEMENT_Root.hpp"

#include "JCA_IOT_MESH_define.h"
#include "JCA_IOT_MESH_types.h"

namespace JCA{ namespace IOT{ namespace MESH{
  class cClient{
    public:
      /***************************************
       * Methode: cClient()
       * Info:  Der Konstruktor initialisiert Server-Vektoren
       *        und Status
       ***************************************/
      cClient(){
        State = init;
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
      void update(std::vector<ELEMENT::cRoot*> Elements, JsonObject &MeshOut, uint32_t DiffMillis, uint32_t Timestamp){
        // TODO
        // Elemente durchsuchen
        //    Archiv-Liste durchsuchen
        //    Alarm-Liste durchsuchen
        //    Nachrichten in MeshOut eintragen
        // Server-Watchdogs hochzählen
        //    Server status updaten / entfernen
        //    -> sendSrvRequest
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
        
        // Uhrzeit stellen
        
        // Zu Logging-Server-Liste hinzufügen oder watchdog zurück setzen
        if (Data.msg["logging"]){
          // Client-Status aktuallisieren
          State = online;
          
          // Server-Liste durchsuchen
          notFound = true;
          id = Data.from;
          for (int srv = 0; srv < LogServers.size(); srv++){
            if (LogServers[srv]->id == id){
              // Watchdog zurücksetzen
              LogServers[srv]->wd = 0;
              notFound = false;
              break;
            }
          }
          if (notFound){
            LogServers.push_back(serverState());
            LogServers.back()->id = id;
            LogServers.back()->wd = 0;
          }
        }
        
        // Zu Alarming-Server-Liste hinzufügen oder watchdog zurück setzen
        if (Data.msg["alarming"]){
          notFound = true;
          id = Data.from;
          for (int srv = 0; srv < AlarmServers.size(); srv++){
            if (AlarmServers[srv]->id == id){
              // Watchdog zurücksetzen
              AlarmServers[srv]->wd = 0;
              notFound = false;
              break;
            }
          }
          if (notFound){
            AlarmServers.push_back(serverState());
            AlarmServers.back()->id = id;
            AlarmServers.back()->wd = 0;
          }
        }
        
        // Zu Archiv-Server-Liste hinzufügen oder watchdog zurück setzen
        if (Data.msg["archiv"]){
          notFound = true;
          id = Data.from;
          for (int srv = 0; srv < ArchivServers.size(); srv++){
            if (ArchivServers[srv]->id == id){
              // Watchdog zurücksetzen
              ArchivServers[srv]->wd = 0;
              notFound = false;
              break;
            }
          }
          if (notFound){
            ArchivServers.push_back(serverState());
            ArchivServers.back()->id = id;
            ArchivServers.back()->wd = 0;
          }
        }
      }
      
      /***************************************
       * Methode: recvAlarmAck(std::vector<ELEMENT::cRoot*> Elements, JsonObject Data)
       * Info:  Funktion zum empfangen und verarbeiten einer
       *        Alarm-Quittierung
       *        Alarm.State auf Ack/Idle setzen
       ***************************************/
      void recvAlarmAck(std::vector<ELEMENT::cRoot*> Elements, JsonObject Data){
        // TODO
        // Alarm Quittierung empfangen
        // Element[elementIdx].Alarm[alarmIdx].State schreiben
      }
      
      /***************************************
       * Methode: sendArchivData(JsonObject MeshOut, archivData Data)
       * Info:  Funktion zur Erstellung eines Archiv Telegramms
       *        Fügt dem MeshOut-Buffer eine Archiv-Nachricht für
       *        jeden Server hinzu.
       ***************************************/
      void sendArchivData(JsonObject &MeshOut, archivData Data){
        JsonArray Server;
        JsonObject Msg;
        // Alle erfallsten Server durchlaufen
        for (int srv = 0; srv < ArchivServers.size(); srv++){
          // Falls noch kein Eintrag für die NodeId existiert, diese erstellen
          if (MeshOut->containsKey(ArchivServers[srv]->id)){
            Server = MeshOut[ArchivServers[srv]->id];
          }else{
            Server = MeshOut->createNestedArray(ArchivServers[srv]->id);
          }
          Msg = Server.createNestedObject();
          Msg["msgId"] = JCA_IOT_MESH_SRV_ARCHIVDATA;
          Msg["time"] = Data.timestamp;
          Msg["element"] = Data.elementIndex;
          Msg["data"] = Data.datapointIndex;
          Msg["value"] = Data.value;
          Msg["type"] = Data.typeMask;
        }
      }
      
      /***************************************
       * Methode: sendAlarm(JsonObject MeshOut, alarm Data)
       * Info:  Funktion zur Erstellung eines Alarm Telegramms
       *        Fügt dem MeshOut-Buffer eine Alarm-Nachricht für
       *        jeden Server hinzu.
       ***************************************/
      void sendAlarm(JsonObject &MeshOut, alarm Data){
        JsonArray Server;
        JsonObject Msg;
        // Alle erfallsten Server durchlaufen
        for (int srv = 0; srv < AlarmServers.size(); srv++){
          // Falls noch kein Eintrag für die NodeId existiert, diese erstellen
          if (MeshOut->containsKey(AlarmServers[srv]->id)){
            Server = MeshOut[AlarmServers[srv]->id];
          }else{
            Server = MeshOut->createNestedArray(AlarmServers[srv]->id);
          }
          Msg = Server.createNestedObject();
          Msg["msgId"] = JCA_IOT_MESH_SRV_ALARM;
          Msg["time"] = Data.timestamp;
          Msg["text"] = Data.text;
          Msg["prio"] = Data.prio;
          Msg["eIdx"] = Data.elementIndex;
          Msg["aIdx"] = Data.alarmIndex;
        }
      }
      
      /***************************************
       * Methode: sendError(JsonObject MeshOut, error Data)
       * Info:  Funktion zur Erstellung eines Archiv Telegramms
       *        Fügt dem MeshOut-Buffer eine Archiv-Nachricht für
       *        jeden Server hinzu.
       ***************************************/
      void sendError(JsonObject &MeshOut, error Data){
        JsonArray Server;
        JsonObject Msg;
        // Alle erfallsten Server durchlaufen
        for (int srv = 0; srv < LogServers.size(); srv++){
          // Falls noch kein Eintrag für die NodeId existiert, diese erstellen
          if (MeshOut->containsKey(LogServers[srv]->id)){
            Server = MeshOut[LogServers[srv]->id];
          }else{
            Server = MeshOut->createNestedArray(LogServers[srv]->id);
          }
          Msg = Server.createNestedObject();
          Msg["msgId"] = JCA_IOT_MESH_SRV_FAILLOG;
          Msg["type"] = Data.type;
          Msg["data"] = Data.data;
        }
      }

    private:
      clientState State;
      std::vector<serverState> LogServers;
      std::vector<serverState> AlarmServers;
      std::vector<serverState> ArchivServers;
      
      /***************************************
       * Methode: sendSrvRequest(JsonObject MeshOut)
       * Info:  Funktion zur Erstellung eines Server-Request
       *        Fügt dem MeshOut-Buffer eine Broadcast-Nachricht hinzu
       ***************************************/
      void sendSrvRequest(JsonObject &MeshOut){
        JsonArray Broadcasts;
        JsonObject Msg;
        
        // Client-Status aktuallisieren
        State = requesting;
        
        // Falls noch kein Broadcast Eintrag existiert diesen erstellen
        if (MeshOut->containsKey("broadcast")){
          Broascasts = MeshOut["broadcast"];
        }else{
          Broascasts = MeshOut->createNestedArray("broadcast");
        }
        Msg = Broadcasts.createNestedObject();
        Msg["msgId"] = JCA_IOT_MESH_SRV_REQUEST;
      }
      
  };
}}}
#endif