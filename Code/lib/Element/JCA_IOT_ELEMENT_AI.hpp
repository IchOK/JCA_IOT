/**********************************************
 * Class:  JCA_IOT_ELEMENT_AI
 * Info:   Abgeleitete Klasse von JCA_IOT_ELEMENT
 *       Initialisiert einen Analog-GPIO und liest
 *       diesen ein.
 * Version:
 *   V1.0.0  Erstellt   28.06.2020  JCA
 *   -add Properties
 *     -- Pin
 *     -- DataInput
 *     -- readVCC
 *     -- scaleMin/Max
 *   -add Methoden
 *     -- cAI
 *     -- update
 **********************************************/

#ifndef _JCA_IOT_ELEMENT_AI_H
#define _JCA_IOT_ELEMENT_AI_H

#include "JCA_IOT_Debug.h"

#include "Element/JCA_IOT_ELEMENT_Root.hpp"
#include "Element/JCA_IOT_ELEMENT_Handler.hpp"

//Include extrenal
#include <string.h>

namespace JCA{ namespace IOT{ namespace ELEMENT{
  class cAI : public cRoot {
   public:
    unsigned char Pin;
    bool readVCC;
    float scaleMin;
    float scaleMax;
    cDataFloat  DataInput;
    cArchiv     ArchivTag;
    
    cAI(const char* InName, const unsigned char InPin, const bool VCC, const float InMin, const float InMax) : cRoot(InName, JCA_IOT_ELEMENT_TYPE_AI) {
      
      // init Data
      DataInput.init("Value");
      // add Data to Vector
      Data.push_back((cData*)(&DataInput));
      
      // Archiv Tag
      ArchivTag.init("AnalogIn");
      // add Archiv to Vector
      Archiv.push_back(&ArchivTag);
      
      // init AI
      Pin = InPin;
      readVCC = VCC;
      scaleMin = InMin;
      scaleMax = InMax;
    }
    
    virtual void update(uint32_t DiffMillis, uint32_t Timestamp) {
      int InRaw;
      float InScaled;
      #if (DEBUGLEVEL >= JCA_IOT_DEBUG_LOOP)
        Serial.println(F(" START - cAI.update()"));
        Serial.printf("  Name:%s\r\n",Name);
      #endif
      
      //Inputs will be updated by the global Handler
      
      //Read Value from AnalogIn
      if (readVCC){
        InRaw = ESP.getVcc();
        InScaled = (float)InRaw / scaleMax / 1000;
        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_LOOP)
          Serial.print(F("  VCC-Value:"));
        #endif
      }else{
        InRaw = analogRead(Pin);
        InScaled = (float)InRaw / 1023.0;
        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_LOOP)
          Serial.print(F("  ADC-Value:"));
        #endif
      }
      #if (DEBUGLEVEL >= JCA_IOT_DEBUG_LOOP)
        Serial.print(InScaled * (scaleMax - scaleMin) + scaleMin);
        Serial.printf("[%i]\r\n", InRaw);
      #endif
      DataInput.Value = InScaled * (scaleMax - scaleMin) + scaleMin;
      //Write Archiv-Value
      ArchivTag.setValue(InRaw, Timestamp, false);
      
      #if (DEBUGLEVEL >= JCA_IOT_DEBUG_LOOP)
        Serial.println(F(" DONE - cAI.update()"));
      #endif
    }
    
  };
  
  void createAI(JsonObject JConf, std::vector<JCA::IOT::ELEMENT::cRoot*>& InElements){
    #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
      Serial.println(F("START - createAI()"));
    #endif
    if (JConf.containsKey("name") && JConf.containsKey("config")){
      if (JConf["config"].containsKey("pin")){
        char InName[JCA_IOT_ELEMENT_NAME_LEN];
        unsigned char InPin;
        float InMin;
        float InMax;
        bool readVCC;
        
        InPin = JConf["config"]["pin"].as<unsigned char>();
        readVCC = JConf["config"]["vcc"].as<bool>();
        InMin = JConf["config"]["min"].as<float>();
        InMax = JConf["config"]["max"].as<float>();
        strncpy(InName, JConf["name"].as<char*>(), JCA_IOT_ELEMENT_NAME_LEN);
         
        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
          Serial.printf("  Name:%s - Pin:%i - VCC:%i - Min:", InName, InPin, readVCC);
          Serial.print(InMin);
          Serial.print(F(" - Max:"));
          Serial.println(InMax);
        #endif

        InElements.push_back(new cAI(InName, InPin, readVCC, InMin, InMax));
        InElements.back()->config(JConf);
      }
    }
    #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
      Serial.println(F("DONE - createAI()"));
    #endif
  };
  
  void beginAI(cHandler& Handler){
    Handler.CreateElement.insert( std::pair<String, std::function<void(JsonObject, std::vector<ELEMENT::cRoot*>&)> >("AI", createAI));
  };
}}}

#endif