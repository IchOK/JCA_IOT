/**********************************************
 * Class:   JCA_IOT_HANDLER
 * Info:    Die Klasse stellt den Handler für
 *          alle IOT-Elemente dar und bietet die
 *          die Schnittstelle zum Mesh-Netzwerk
 * Version:
 *    V1.0.0   Erstellt    02.11.2019  JCA
 *    -add Properties
 *       -- Elements Vector
 *       -- CreateElement Map
 *       -- JDoc StaticJsonDocument
 *       -- Name, ErrorTest, QC, ConfigFile
 *    -add Methoden
 *       -- cHandler
 *       -- addElement
 *       -- config
 *       -- update
 **********************************************/

#ifndef _JCA_IOT_HANDLER_H
#define _JCA_IOT_HANDLER_H

//Include extrenal
#include <map>
#include <vector>
#include <ArduinoJson.h>
#include <FS.h>
#include "JCA_IOT_ELEMENT.hpp"

namespace JCA{ namespace IOT{ namespace ELEMENT{
   
  class cHandler{
    public:
      /***************************************
       * Propertie: CreateElement
       * Info:  CreateElement ist eine Zuordnungsliste,
       *          String zu Element-Konstruktor-Funktion.
       *        Die Liste wird durch [begin..]-Funktionen der einzelnen
       *          Elemente in der Setup-Prozedur zu Begin befüllt.
       ***************************************/
      std::map<String, std::function<void(JsonObject, std::vector<ELEMENT::cRoot*>&)> > CreateElement;
      /***************************************
       * Propertie: Elements
       * Info:  Elements ist Vektor von Elementen,
       *          die alle auf der Root-Class basieren.
       *        Die Elemente werden anhand der Konfigurationsdatei 
       *          zur Runtime erzeugt.
       *        Um ein Element erzeugen zu können,
       *          muss zuvor die Konstrutor-Funktion in die 
       *          CreateElement Liste eingetragen werden.
       ***************************************/
      std::vector<ELEMENT::cRoot*> Elements;
      char Name[JCA_IOT_ELEMENT_NAME_LEN];
      String ErrorText;
      String ConfigFileName;
      StaticJsonDocument<JCA_IOT_HANDLER_JSON_DOCSIZE> JDoc;
      unsigned char QC;
      
      /***************************************
       * Methode: cHandler()
       * Info:  Der Konstruktor initialisiert den Quality-Code
       ***************************************/
      cHandler(){
        QC = JCA_IOT_QC_CREAT;
      }
      
      /***************************************
       * Methode: update(unit32_t DiffMillis)
       * Info:  Die Methore aktuallisiert alle Elemente im Elements-Vector.
       *        - Eingänge aktuallisieren
       *        - Element-Code ausführen der die Daten aktuallisiert.
       * Parameter:
       *        DiffMillis [uint32_t] vergangene Zeit seit letztem Aufruf.
       ***************************************/
      void update(uint32_t DiffMillis) {
        #if DEBUGLEVEL >= 3
          Serial.println(F("START - cHandler.update()"));
          Serial.printf("  DiffMillis:%i\r\n",DiffMillis);
        #endif
        for (int e = 0; e < Elements.size(); e++){
          //-----------------------------------------------------------------------------------------------------------
          // UPDATE INPUTS
          // Jedes Element [e] verfügt über einen Vector von Eingängen {Input}.
          for (int i = 0; i < Elements[e]->Input.size(); i++){
            // Über den Type des Eingangs [i] {Input.Type} wird der erforderliche Datentype definiert.
            switch(Elements[e]->Input[i]->Type){
              case JCA_IOT_ELEMENT_DATA_BOOL:
                // Für jeden Datenpunkt ist in der Basisklasse {cRoot} eine Schreibfunktion definiert {setInput...}
                // Als Parameter wird der Eingang [i] und der Wert übergeben.
                //    Der Wert wird direkt dem referenzierten Element entnommen {getData...}
                //      Element    = Element-Index des Eingangs {Input.ElementIndex}
                //      Datenpunkt = Daten-Index des Eingangs {Input.DataIndex}
                //-----------------------------------------------------------------------------------------------------
                Elements[e]->setInputBool(i, Elements[Elements[e]->Input[i]->ElementIndex]->getDataBool(Elements[e]->Input[i]->DataIndex));
                break;
              case JCA_IOT_ELEMENT_DATA_INT:
                Elements[e]->setInputInt(i, Elements[Elements[e]->Input[i]->ElementIndex]->getDataInt(Elements[e]->Input[i]->DataIndex));
                break;
              case JCA_IOT_ELEMENT_DATA_FLOAT:
                Elements[e]->setInputFloat(i, Elements[Elements[e]->Input[i]->ElementIndex]->getDataFloat(Elements[e]->Input[i]->DataIndex));
                break;
            }
          }
          
          //------------------------
          // UPDATE ELEMENT / DATA
          // Die Update Funktion ist spezifisch in jedem Element definiert {update(Millis)}
          //------------------------
          Elements[e]->update(DiffMillis);
        }
        #if DEBUGLEVEL >= 3
          Serial.println(F("DONE - cHandler.update()"));
        #endif
      }
      
      /***************************************
       * Methode: config(char* InConfigFile)
       * Info:  Die Konfigurations-Methode prüft die Konfigurations-Datei und
       *        konfiguriert das IOT.
       *        - INIT      + Filesystem initialisieren
       *                    + Datei öffnen und Länge prüfen
       *                    + Json-Konfig einlesen
       *        - CONFIG    + Global : Node-Name
       *                    + Kommunikations-Protokolle
       *                    + Elemente
       * Parameter:
       *        InConfigFile [char*] Name/Pfad der Konfigurations-Datei
       ***************************************/
      bool config(char* InConfigFile){
        #if DEBUGLEVEL >= 2
          Serial.println(F("START - cHandler.config()"));
        #endif
        ConfigFileName = InConfigFile;
        //-------------------------------------------------------------------------------------------------------------
        // INIT
        // Initialisierung des Filesystems
        if (!SPIFFS.begin()) {
          // .. Bei Fehler wird der Quality-Code angepasst und 
          QC = JCA_IOT_QC_CONFCREAT;
          // .. der Fehlertext geschrieben.
          ErrorText = F("Failed to config file");
          #if DEBUGLEVEL >= 1
            Serial.println(ErrorText);
          #endif
          return false;
        }
        // Öffnen der Konfigurations-Datei
        File ConfigFile = SPIFFS.open(ConfigFileName, "r");
        if(!ConfigFile){
          // .. Bei Fehler wird der Quality-Code angepasst und 
          QC = JCA_IOT_QC_CONFCREAT;
          // .. der Fehlertext geschrieben.
          ErrorText = F("Failed to config file");
          #if DEBUGLEVEL >= 1
            Serial.println(ErrorText);
          #endif
          return false;
        }
        // Die Datei-Länge wird geprüft, um sicher zu stellen 
        //  dass der JSON-Speicherbereich ausreicht.
        size_t Size = ConfigFile.size();
        if(Size > JCA_IOT_HANDLER_FILE_MAXSIZE){
          // .. Ist die Datei zu gross wird der Quality-Code angepasst und 
          QC = JCA_IOT_QC_CONFCREAT;
          // .. der Fehlertext geschrieben.
          ErrorText = F("Config file size is too large");
          #if DEBUGLEVEL >= 1
            Serial.println(ErrorText);
          #endif
          return false;
        }
        // Die Konfigurations-Datei wird in ein JSON-Dokument konvertiert.
        DeserializationError JError = deserializeJson(JDoc, ConfigFile);
        if(JError){
          // .. Bei Fehler wird der Quality-Code angepasst und 
          QC = JCA_IOT_QC_CONFCREAT;
          // .. der Fehlertext geschrieben.
          ErrorText = F("deserialize FAILD: ");
          ErrorText += JError.c_str();
          #if DEBUGLEVEL >= 1
            Serial.println(ErrorText);
          #endif
          return false;
        }
        //-------------------------------------------------------------------------------------------------------------
        
        //-------------------------------------------------------------------------------------------------------------
        // CONFIG - Node Einstellunegn
        #if DEBUGLEVEL >= 2
          Serial.println(F("  START - global Settings"));
        #endif
        // Das JSON-Dokument -> Objekt
        JsonObject JConfig = JDoc.as<JsonObject>();
        // Ist ein Name vorhanden ..
        if (JConfig.containsKey("name")){
          // .. wird dieser als Node-Name verwendet.
          strncpy(Name, JConfig["name"], JCA_IOT_ELEMENT_NAME_LEN);
        }else{
          // .. sonst wird die Chip-ID als eindeutige Identifikations verwendet.
          itoa(ESP.getChipId(),Name, JCA_IOT_ELEMENT_NAME_LEN);
        }
        //-------------------------------------------------------------------------------------------------------------

        //-------------------------------------------------------------------------------------------------------------
        // CONFIG - Netzwerk Einstellungen
        // TODO
        // Ist eine Konfiguration vorhanden ..
        // .. wird das Node mit dem Mesh-Netzwerk verbunden
        //    und der Node-Handler initialisiert
        // .. sonst wird ein lokaler AP gestartet
        //    und ein Webserver
        //    mit einer Konfigurations-Webseite
        //    - File Upload
        //    - OnBoard-Blink Funktion
        //-------------------------------------------------------------------------------------------------------------

        //-------------------------------------------------------------------------------------------------------------
        // CONFIG - Bussysteme
        // TODO
        // Es wird für jedes definierte Bussystem die Konfiguration geprüft ..
        // .. ist die Konfiguration vorhanden wird das Bussystem initialisiert.
        //    - i2c
        //    - spi
        //    - oneWire
        //
        //  TTTTT  OOO  DDD    OOO 
        //    T   O   O D  D  O   O
        //    T   O   O D   D O   O
        //    T   O   O D   D O   O
        //    T   O   O D  D  O   O
        //    T    OOO  DDD    OOO 
        //
        //-------------------------------------------------------------------------------------------------------------

        #if DEBUGLEVEL >= 2
          Serial.print(F("    Node Name:"));
          Serial.println(Name);
          Serial.println(F("  DONE - global Settings"));
        #endif

        //-------------------------------------------------------------------------------------------------------------
        // CONFIG - Elemente erzeugen
        #if DEBUGLEVEL >= 2
          Serial.println(F("  START - create Elements"));
        #endif
        // Für jedes Element im "elements"-Array erfolgt ein Eintrag in den Elemens-Vector
        for(JsonObject JElement : JConfig["elements"].as<JsonArray>()){
          #if DEBUGLEVEL >= 2
            if (JElement.containsKey("name")){
              Serial.print(F("    Element Name:"));
              Serial.println(JElement["name"].as<char*>());
            }
          #endif
          if (JElement.containsKey("type")){
            // .. das Element muss über eine "type"-Eigenschaft verfügen
            #if DEBUGLEVEL >= 2
              Serial.print(F("    Element Type:"));
              Serial.println(JElement["type"].as<char*>());
            #endif
            if(CreateElement.find(JElement["type"].as<String>()) != CreateElement.end() ){
              // .. und der Type muss in der Konstruktor-Zuordnungsliste {CreateElement} vorhanden sein
              CreateElement[JElement["type"].as<String>()](JElement, Elements);
            }else{
              // .. sonst wird ein Fehler ausgegeben.
              QC = JCA_IOT_QC_CONFCREAT;
              ErrorText = F("Unable to create Type ");
              ErrorText += JElement["type"].as<String>();
              #if DEBUGLEVEL >= 1
                Serial.println(ErrorText);
              #endif
            }
          }
        }
        //-------------------------------------------------------------------------------------------------------------
        #if DEBUGLEVEL >= 2
          Serial.println(F("DONE - cHandler.config()"));
        #endif
        return true;
      }
  };
}}}

#endif