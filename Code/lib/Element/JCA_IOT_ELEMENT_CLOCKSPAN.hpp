/**********************************************
 * Class:   JCA_IOT_ELEMENT_CLOCKSPAN
 * Info:    Abgeleitete Klasse von JCA_IOT_ELEMENT
 *          Bildet eine logische Oder-Verknüpfung.
 *          Zur Zeit noch statisch mit 4 Eingängen
 * Version:
 *    V1.0.0   Erstellt    03.08.2020  JCA
 *    -add Properties
 *       -- TimeOn/-Off
 *       -- Value (Manual-Auto)
 *    -add Methoden
 *       -- cCLOCKSPAN
 *       -- update
 **********************************************/

#ifndef _JCA_IOT_ELEMENT_CLOCKSPAN_H
#define _JCA_IOT_ELEMENT_CLOCKSPAN_H

#include <time.h>

#include "JCA_IOT_Debug.h"

#include "Element/JCA_IOT_ELEMENT_Root.hpp"
#include "Element/JCA_IOT_ELEMENT_Handler.hpp"

//Include extrenal

namespace JCA{ namespace IOT{ namespace ELEMENT{
  class cCLOCKSPAN : public cRoot {
    public:
      cDataInt    TimeOn;
      cDataInt    TimeOff;
      cDataBool   ManMode;
      cDataBool   AutoValue;
      cDataBool   ManValue;
      cDataBool   Value;
      
      cCLOCKSPAN(const char* InName) : cRoot(InName, JCA_IOT_ELEMENT_TYPE_CLOCKSPAN) {
        
        // init Data
        TimeOn.init("TimeOn");
        TimeOff.init("TimeOff");
        ManMode.init("Manual");
        AutoValue.init("AutoValue");
        ManValue.init("ManualValue");
        Value.init("Value");
        // add Data to Vector
        Data.push_back((cData*)(&ManMode));
        Data.push_back((cData*)(&AutoValue));
        Data.push_back((cData*)(&ManValue));
        Data.push_back((cData*)(&Value));
        Data.push_back((cData*)(&TimeOn));
        Data.push_back((cData*)(&TimeOff));
      }
      
      virtual void update(uint32_t DiffMillis, uint32_t Timestamp) {
        // TODO: make it dynamic Timestamp dynamic (with an dwithout Daylight Saving)
        tm localTime;
        time_t tTimestamp = Timestamp;
        gmtime_r(&tTimestamp, &localTime);
        
        // calc IntervalSeconds based on TimerInterval
        // TODO: make TimerInterval dynamic (Day, Week, Year)
        uint32_t IntervalSeconds;
        // Seconds per Day
        IntervalSeconds = localTime.tm_hour*3600 + localTime.tm_min*60 + localTime.tm_sec;
        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_LOOP)
          Serial.println(F(" START - cCLOCKSPAN.update()"));
          Serial.printf("  Name:%s\r\n",Name);
          Serial.printf("  Uhrzeit  %02d:%02d:%02d\r\n", localTime.tm_hour, localTime.tm_min, localTime.tm_sec);
          Serial.printf("  Interval %08d\r\n", IntervalSeconds);
          Serial.printf("  TimeOn   %08d\r\n", TimeOn.Value);
          Serial.printf("  TimeOff  %08d\r\n", TimeOff.Value);
        #endif
        
        //Check if Timestamp ist valid, year bigger than 1
        if (localTime.tm_year > 1){
          //If current Time between On and Off set Value
          if (TimeOn.Value > TimeOff.Value){
            AutoValue.Value = IntervalSeconds >= TimeOn.Value || IntervalSeconds < TimeOff.Value;
          }else{
            AutoValue.Value = IntervalSeconds >= TimeOn.Value && IntervalSeconds < TimeOff.Value;
          }
        }
        
        if(ManMode.Value){
          //Manual-Mode: Move ManValue to Output
          Value.Value = ManValue.Value;
        }else{
          //Auto-Mode: Move AutoValue to Output
          Value.Value = AutoValue.Value;
          // .. updating ManValue 
          ManValue.Value = AutoValue.Value;
        }

        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_LOOP)
          Serial.printf("  AutoValue:%i\r\n",AutoValue.Value);
          Serial.printf("  ManValue:%i\r\n",ManValue.Value);
          Serial.printf("  Value:%i\r\n",Value.Value);
          Serial.println(F(" DONE - cCLOCKSPAN.update()"));
        #endif
      }
  };

  void createCLOCKSPAN(JsonObject JConf, std::vector<JCA::IOT::ELEMENT::cRoot*>& InElements){
    #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
      Serial.println(F("START - createCLOCKSPAN()"));
    #endif
    if (JConf.containsKey("name")){
      char InName[JCA_IOT_ELEMENT_NAME_LEN];
      strncpy(InName, JConf["name"].as<char*>(), JCA_IOT_ELEMENT_NAME_LEN);
         
      #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
        Serial.printf("  Name:%s\r\n", InName);
      #endif
      InElements.push_back(new cCLOCKSPAN(InName));
      InElements.back()->config(JConf);
    }
    #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
       Serial.println(F("DONE - createCLOCKSPAN()"));
    #endif
  };
   
  void beginCLOCKSPAN(cHandler& Handler){
    Handler.CreateElement.insert( std::pair<String, std::function<void(JsonObject, std::vector<ELEMENT::cRoot*>&)> >("CLOCKSPAN", createCLOCKSPAN));
  };
}}}

#endif