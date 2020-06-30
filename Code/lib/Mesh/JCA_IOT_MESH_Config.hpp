/**********************************************
 * Class:	JCA_IOT_MESH_Config
 * Info: 	Die Klasse handelt alle Funktionen rund
 *        um die Konfig-Telegramme (get-/SteConf)
 * Version:
 * 	V1.0.0	Erstellt	16.06.2020	JCA
 *		-recvSrvPublish
 *    -sendSrvRequest
 *    -sendArchiData
 *    -sendAlarm
 *    -sendError
 *    -update
 **********************************************/

#ifndef _JCA_IOT_MESH_CONFIG_H
#define _JCA_IOT_MESH_CONFIG_H

//Include extrenal
#include <FS.h>
#include <vector>
#include <ArduinoJson.h>
#include <painlessMesh.h>

//Elemente einbinden für Update
#include "JCA_IOT_Debug.h"

#include "Element/JCA_IOT_ELEMENT_Root.hpp"

#include "Mesh/JCA_IOT_MESH_define.h"
#include "Mesh/JCA_IOT_MESH_types.h"

namespace JCA{ namespace IOT{ namespace MESH{
  class cConfig{
    public:
      char Name[JCA_IOT_ELEMENT_NAME_LEN];
      char Role[JCA_IOT_ELEMENT_NAME_LEN];
      configState State;
      unsigned char QC;
      String ErrorText;
      /***************************************
       * Methode: cConfig()
       * Info:  Der Konstruktor
       ***************************************/
      cConfig(){
         State = confInit;
      }
    
      /***************************************
       * Methode: update(DiffMillis, &painlessMesh)
       * Info:  Funktion durchsucht die Elemente nach
       *        Client-Nachrichten und handelt die
       *        Server-Listen.
       * Parameter:
       *        DiffMillis [uint32_t]
       *            Vergangene Zeit seit letztem Aufruf
       *        Mesh [&painlessMesh]
       *            Zeiger auf die Mesh-Instanz
       ***************************************/
      void update(uint32_t DiffMillis, painlessMesh &Mesh){
        // TODO
        // Statusprüfen
        //    nach Timeout Daten erneut anfordern
      }
      
      
      /***************************************
       * Methode: config(ConfigFileName, *Elements)
       * Info:  Funktion durchsucht die Elemente nach
       *        Client-Nachrichten und handelt die
       *        Server-Listen.
       * Parameter:
       *        ConfigFileName [const char*]
       *            Pfad der Konfigurationsdatei
       *        Element [std::vector<ELEMENT::cRoot*>]
       *            Zeiger auf die Elementsammlung
       ***************************************/
      bool config(const char *ConfigFileName, std::vector<ELEMENT::cRoot*> *Elements, JsonObject& JConfig, painlessMesh &Mesh){
        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
          Serial.println(F("START - cConfig.config()"));
        #endif
        FileName = ConfigFileName;
        ptrElements = Elements;
        
        //-------------------------------------------------------------------------------------------------------------
        // INIT
        // Initialisierung des Filesystems
        if (!SPIFFS.begin()) {
          // .. Bei Fehler wird der Quality-Code angepasst und 
          QC = JCA_IOT_ELEMENT_QC_CONFCREAT;
          // .. der Fehlertext geschrieben.
          ErrorText = F("Failed to config file");
          #if (DEBUGLEVEL >= JCA_IOT_DEBUG_DIAG)
            Serial.println(ErrorText);
          #endif
          return false;
        }
        // Öffnen der Konfigurations-Datei
        File ConfigFile = SPIFFS.open(ConfigFileName, "r");
        if(!ConfigFile){
          // .. Bei Fehler wird der Quality-Code angepasst und 
          QC = JCA_IOT_ELEMENT_QC_CONFCREAT;
          // .. der Fehlertext geschrieben.
          ErrorText = F("Failed to config file");
          #if (DEBUGLEVEL >= JCA_IOT_DEBUG_DIAG)
            Serial.println(ErrorText);
          #endif
          return false;
        }
        // Die Datei-Länge wird geprüft, um sicher zu stellen 
        //  dass der JSON-Speicherbereich ausreicht.
        size_t Size = ConfigFile.size();
        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
          Serial.print(F("Filesize:"));
          Serial.println(Size);
        #endif
        if(Size > JCA_IOT_MESH_CONFIG_FILE_MAXSIZE){
          // .. Ist die Datei zu gross wird der Quality-Code angepasst und 
          QC = JCA_IOT_ELEMENT_QC_CONFCREAT;
          // .. der Fehlertext geschrieben.
          ErrorText = F("Config file size is too large");
          #if (DEBUGLEVEL >= JCA_IOT_DEBUG_DIAG)
            Serial.println(ErrorText);
          #endif
          return false;
        }
        // Die Konfigurations-Datei wird in ein JSON-Dokument konvertiert.
        DeserializationError JError = deserializeJson(JDoc, ConfigFile);
        if(JError){
          // .. Bei Fehler wird der Quality-Code angepasst und 
          QC = JCA_IOT_ELEMENT_QC_CONFCREAT;
          // .. der Fehlertext geschrieben.
          ErrorText = F("deserialize FAILD: ");
          ErrorText += JError.c_str();
          #if (DEBUGLEVEL >= JCA_IOT_DEBUG_DIAG)
            Serial.println(ErrorText);
          #endif
          return false;
        }
        // Konfig JSON-Dokument in JsonOject konvertiert.
        JConfig = JDoc.as<JsonObject>();
        //-------------------------------------------------------------------------------------------------------------
        
        //-------------------------------------------------------------------------------------------------------------
        // CONFIG - Node Einstellungen
        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
          Serial.println(F("  START - global Settings"));
        #endif
        // Ist ein Name vorhanden ..
        if (JConfig.containsKey("name")){
          // .. wird dieser als Node-Name verwendet.
          strncpy(Name, JConfig["name"].as<char*>(), JCA_IOT_ELEMENT_NAME_LEN);
        }else{
          // .. sonst wird die Chip-ID als eindeutige Identifikations verwendet.
          itoa(ESP.getChipId(),Name, JCA_IOT_ELEMENT_NAME_LEN);
        }
        // Ist ein Rolle vorhanden ..
        if (JConfig.containsKey("role")){
          // .. wird dieser als Node-Name verwendet.
          strncpy(Role, JConfig["role"].as<char*>(), JCA_IOT_ELEMENT_NAME_LEN);
        }else{
          // .. sonst wird die Chip-ID als eindeutige Identifikations verwendet.
          itoa(ESP.getChipId(),Role, JCA_IOT_ELEMENT_NAME_LEN);
        }

        // Prüfen ob es sich um die Gateway-Node handelt
        if (JConfig.containsKey("gateway")){
          uint8_t IP[4];
          uint8_t IP2[4] = {192,168,223,1};
          IP[0] = JConfig["gateway"]["station"][0];
          IP[1] = JConfig["gateway"]["station"][1];
          IP[2] = JConfig["gateway"]["station"][2];
          IP[3] = JConfig["gateway"]["station"][3];
          //Mit Node-RED direkt verbinden
          Mesh.stationManual(JConfig["gateway"]["ssid"].as<String>(), JConfig["gateway"]["pass"].as<String>(), JConfig["gateway"]["port"].as<uint16_t>(), IP);
          Mesh.setRoot(true);
          Mesh.setContainsRoot(true);
          // Set Hostname
          Mesh.setHostname(Name);
          #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
            Serial.println(F("    GATEWAY"));
          Serial.print(F("      SSID:"));
          Serial.println(JConfig["gateway"]["ssid"].as<String>());
          Serial.print(F("      Pass:"));
          Serial.println(JConfig["gateway"]["pass"].as<String>());
          Serial.print(F("      Port:"));
          Serial.println(JConfig["gateway"]["port"].as<uint16_t>());
          Serial.print(F("      Station:"));
          Serial.printf("%d.%d.%d.%d\r\n", IP[0], IP[1], IP[2], IP[3]);
          #endif
        }
        
        // Mesh-Node als OTA-Receiver konfiguriren
        Mesh.initOTAReceive(Role);
        
        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
          Serial.print(F("    Node Name:"));
          Serial.println(Name);
          Serial.print(F("    Node Role:"));
          Serial.println(Role);
          Serial.println(F("  DONE - global Settings"));
        #endif
        //-------------------------------------------------------------------------------------------------------------
        
        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
          Serial.println(F("DONE - cConfig.config()"));
        #endif
        return true;
      }
      
    private:
      uint16_t noPart;
      uint16_t partNo;
      uint32_t wd;
      String FileName;
      std::vector<ELEMENT::cRoot*> *ptrElements;
      StaticJsonDocument<JCA_IOT_MESH_CONFIG_JSON_DOCSIZE> JDoc;
  };
  
}}}

#endif