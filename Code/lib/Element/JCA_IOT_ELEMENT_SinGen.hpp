/**********************************************
 * Class:  JCA_IOT_ELEMENT_SinGen
 * Info:   Abgeleitete Klasse von JCA_IOT_ELEMENT
 *       Erzeugt einen Sinus mit definierbarer Amplitude
 *       und Frequent, sendet einen Alarm bei erreichen
 *       der maximalen Amplitude.
 * Version:
 *   V1.0.0  Erstellt   30.06.2020  JCA
 *   -add Properties
 *     -- Amp
 *     -- Freq
 *   -add Methoden
 *     -- cSinGen
 *     -- update
 **********************************************/

#ifndef _JCA_IOT_ELEMENT_SINGEN_H
#define _JCA_IOT_ELEMENT_SINGEN_H

#include "JCA_IOT_Debug.h"

#include "Element/JCA_IOT_ELEMENT_Root.hpp"
#include "Element/JCA_IOT_ELEMENT_Handler.hpp"

//Include extrenal
#include <string.h>

namespace JCA{ namespace IOT{ namespace ELEMENT{
  class cSinGen : public cRoot {
   public:
    uint32_t    Millis;
    float       Amp;
    float       Period;
    cDataFloat  DataSinus;
    cArchiv     ArchivSinus;
    cAlarm      AlarmSinusMax;
    
    cSinGen(const char* InName, const float InAmp, const float InPeriod) : cRoot(InName, JCA_IOT_ELEMENT_TYPE_SINGEN) {
      
      // init Data
      DataSinus.init("Value");
      // add Data to Vector
      Data.push_back((cData*)(&DataSinus));
      
      // Archiv Tag
      ArchivSinus.init("Sinus");
      // add Archiv to Vector
      Archiv.push_back(&ArchivSinus);
      
      // Archiv Tag
      AlarmSinusMax.init("SinusMax");
      // add Archiv to Vector
      Alarm.push_back(&AlarmSinusMax);
      
      // init SinGen
      Amp = InAmp;
      Period = InPeriod;
    }
    
    virtual void update(uint32_t DiffMillis, uint32_t Timestamp) {
      bool AlarmAct;
      #if (DEBUGLEVEL >= JCA_IOT_DEBUG_LOOP)
        Serial.println(F(" START - cSinGen.update()"));
        Serial.printf("  Name:%s\r\n",Name);
      #endif
      
      //calc Sinusvalue
      Millis += DiffMillis;
      DataSinus.Value = Amp * sin(6.2831853 * (float)Millis / Period);
      #if (DEBUGLEVEL >= JCA_IOT_DEBUG_LOOP)
        Serial.printf("    Millis=%i Value=", Millis);
        Serial.println(DataSinus.Value);
      #endif
      
      //Write Archiv-Value
      ArchivSinus.setValue(DataSinus.Value, Timestamp, false);
      
      //Set Alarm if Value higher then 0.95Amp
      AlarmAct = DataSinus.Value >= 0.95 * Amp;
      AlarmSinusMax.setAlarm(AlarmAct, Timestamp);
      
      #if (DEBUGLEVEL >= JCA_IOT_DEBUG_LOOP)
        Serial.printf("    Alarm=%i\r\n", AlarmAct);
        Serial.println(F(" DONE - cSinGen.update()"));
      #endif
    }
    
  };
  
  void createSinGen(JsonObject JConf, std::vector<JCA::IOT::ELEMENT::cRoot*>& InElements){
    #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
      Serial.println(F("START - createSinGen()"));
    #endif
    if (JConf.containsKey("name") && JConf.containsKey("config")){
      if (JConf["config"].containsKey("amp") && JConf["config"].containsKey("period")){
        char InName[JCA_IOT_ELEMENT_NAME_LEN];
        float InAmp;
        float InPeriod;
        
        InAmp = JConf["config"]["amp"].as<float>();
        InPeriod = JConf["config"]["period"].as<float>();
        strncpy(InName, JConf["name"].as<char*>(), JCA_IOT_ELEMENT_NAME_LEN);
         
        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
          Serial.print(F("  Amp:"));
          Serial.print(InAmp);
          Serial.print(F(" - Freq:"));
          Serial.println(InPeriod);
        #endif

        InElements.push_back(new cSinGen(InName, InAmp, InPeriod));
        InElements.back()->config(JConf);
      }
    }
    #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
      Serial.println(F("DONE - createSinGen()"));
    #endif
  };
  
  void beginSinGen(cHandler& Handler){
    Handler.CreateElement.insert( std::pair<String, std::function<void(JsonObject, std::vector<ELEMENT::cRoot*>&)> >("SinGen", createSinGen));
  };
}}}

#endif