/**********************************************
 * Class:   JCA_IOT_ELEMENT_CLOCKPULSE
 * Info:    Abgeleitete Klasse von JCA_IOT_ELEMENT
 *          Bildet eine logische Oder-Verknüpfung.
 *          Zur Zeit noch statisch mit 4 Eingängen
 * Version:
 *    V1.0.0   Erstellt    03.08.2020  JCA
 *    -add Properties
 *       -- TimeOn/-Off
 *       -- Value (Manual-Auto)
 *    -add Methoden
 *       -- cCLOCKPULSE
 *       -- update
 **********************************************/

#ifndef _JCA_IOT_ELEMENT_CLOCKPULSE_H
#define _JCA_IOT_ELEMENT_CLOCKPULSE_H

#include <time.h>

#include "JCA_IOT_Debug.h"

#include "Element/JCA_IOT_ELEMENT_Root.hpp"
#include "Element/JCA_IOT_ELEMENT_Handler.hpp"

//Include extrenal

namespace JCA{ namespace IOT{ namespace ELEMENT{
  class cCLOCKPULSE : public cRoot {
    public:
      uint32_t    LastInterval;
      int32_t     PulseCount;
      cDataInt    Time;
      cDataInt    Pulse;
      cDataBool   ManMode;
      cDataBool   AutoValue;
      cDataBool   ManValue;
      cDataBool   Value;
      
      cCLOCKPULSE(const char* InName) : cRoot(InName, JCA_IOT_ELEMENT_TYPE_CLOCKPULSE) {
        
        // init Data
        Time.init("Time");
        Pulse.init("Pulse");
        ManMode.init("Manual");
        AutoValue.init("AutoValue");
        ManValue.init("ManualValue");
        Value.init("Value");
        // add Data to Vector
        Data.push_back((cData*)(&ManMode));
        Data.push_back((cData*)(&AutoValue));
        Data.push_back((cData*)(&ManValue));
        Data.push_back((cData*)(&Value));
        Data.push_back((cData*)(&Time));
        Data.push_back((cData*)(&Pulse));
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
          Serial.println(F(" START - cCLOCKPULSE.update()"));
          Serial.printf("  Name:%s\r\n",Name);
          Serial.printf("  Uhrzeit  %02d:%02d:%02d\r\n", localTime.tm_hour, localTime.tm_min, localTime.tm_sec);
          Serial.printf("  Interval %08d\r\n", IntervalSeconds);
          Serial.printf("  Time     %08d\r\n", Time.Value);
          Serial.printf("  Pulse    %08d\r\n", Pulse.Value);
        #endif
        
        //Check if Timestamp ist valid, year bigger than 1
        if (localTime.tm_year > 1){
          if(IntervalSeconds >= Time.Value && LastInterval < Time.Value && IntervalSeconds < Time.Value + 60){
            PulseCount = Pulse.Value;
          }else if(PulseCount > 0){
            PulseCount -= DiffMillis;
          }
          AutoValue.Value = PulseCount > 0;
          LastInterval = IntervalSeconds;
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
          Serial.println(F(" DONE - cCLOCKPULSE.update()"));
        #endif
      }
  };

  void createCLOCKPULSE(JsonObject JConf, std::vector<JCA::IOT::ELEMENT::cRoot*>& InElements){
    #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
      Serial.println(F("START - createCLOCKPULSE()"));
    #endif
    if (JConf.containsKey("name")){
      char InName[JCA_IOT_ELEMENT_NAME_LEN];
      strncpy(InName, JConf["name"].as<char*>(), JCA_IOT_ELEMENT_NAME_LEN);
         
      #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
        Serial.printf("  Name:%s\r\n", InName);
      #endif
      InElements.push_back(new cCLOCKPULSE(InName));
      InElements.back()->config(JConf);
    }
    #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
       Serial.println(F("DONE - createCLOCKPULSE()"));
    #endif
  };
   
  void beginCLOCKPULSE(cHandler& Handler){
    Handler.CreateElement.insert( std::pair<String, std::function<void(JsonObject, std::vector<ELEMENT::cRoot*>&)> >("CLOCKPULSE", createCLOCKPULSE));
  };
}}}

#endif