/**********************************************
 * Class:  JCA_IOT_ELEMENT_PWM
 * Info:   Abgeleitete Klasse von JCA_IOT_ELEMENT
 *       Initialisiert einen PWM-GPIO und schreibt diesen
 * Version:
 *   V1.0.0  Erstellt   31.10.2020  JCA
 *   -add Properties
 *     -- Pin
 *     -- Value
 *     -- Frequency
 *     -- scaleMin/Max
 *   -add Methoden
 *     -- cPWM
 *     -- update
 **********************************************/

#ifndef _JCA_IOT_ELEMENT_PWM_H
#define _JCA_IOT_ELEMENT_PWM_H

#include "JCA_IOT_Debug.h"

#include "Element/JCA_IOT_ELEMENT_Root.hpp"
#include "Element/JCA_IOT_ELEMENT_Handler.hpp"

//Include extrenal
#include <string.h>

namespace JCA{ namespace IOT{ namespace ELEMENT{
  class cPWM : public cRoot {
    public:
      unsigned char Pin;
      float scaleMin;
      float scaleMax;
      cInputFloat ProcessValue;
      
      cPWM(const char* InName, const unsigned char InPin, const uint16_t InFrequency, const float InMin, const float InMax) : cRoot(InName, JCA_IOT_ELEMENT_TYPE_PWM) {
        
        // init Input
        ProcessValue.init("ProcessValue");
        // add Input to Vector
        Input.push_back((cInput*)(&ProcessValue));

        // init PWM
        Pin = InPin;
        scaleMin = InMin;
        scaleMax = InMax;
        //Set Frequency [Hz]
        analogWriteFreq(InFrequency);
      }
      
      virtual void update(uint32_t DiffMillis, uint32_t Timestamp) {
        uint16_t OutRaw;
        float Scaled;
        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_LOOP)
          Serial.println(F(" START - cPWM.update()"));
          Serial.printf("  Name:%s\r\n",Name);
        #endif
        
        //Inputs will be updated by the global Handler
        
        //Scale ProcessValue
        if (ProcessValue.Value >= scaleMax){
          OutRaw = 1023;
        } else if (ProcessValue.Value <= scaleMin) {
          OutRaw = 0;
        } else {
          Scaled = (ProcessValue.Value - scaleMin) / (scaleMax - scaleMin) * 1023.0;
          OutRaw = round(Scaled);
        }
        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_LOOPDEBUG)
          Serial.print("  ProcessValue:");
          Serial.println(ProcessValue.Value);
          Serial.print("  Scaled:");
          Serial.print(Scaled);
          Serial.printf(" [%i]\r\n", OutRaw);
        #endif

        //Write Value to OutputPin
        analogWrite(this->Pin, OutRaw);
        
        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_LOOP)
          Serial.println(F(" DONE - cPWM.update()"));
        #endif
      }
  };
  
  void createPWM(JsonObject JConf, std::vector<JCA::IOT::ELEMENT::cRoot*>& InElements){
    #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
      Serial.println(F("    START - createPWM()"));
    #endif
    if (JConf.containsKey("name") && JConf.containsKey("config")){
      if (JConf["config"].containsKey("pin")){
        char InName[JCA_IOT_ELEMENT_NAME_LEN];
        unsigned char InPin;
        float InMin;
        float InMax;
        uint16_t InFrequency;
        
        InPin = JConf["config"]["pin"].as<unsigned char>();
        InFrequency = JConf["config"]["frequency"].as<uint16_t>();
        InMin = JConf["config"]["min"].as<float>();
        InMax = JConf["config"]["max"].as<float>();
        strncpy(InName, JConf["name"].as<char*>(), JCA_IOT_ELEMENT_NAME_LEN);
         
        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
          Serial.printf("      Name:%s - Pin:%i - Freq:%i - Min:", InName, InPin, InFrequency);
          Serial.print(InMin);
          Serial.print(F(" - Max:"));
          Serial.println(InMax);
        #endif

        InElements.push_back(new cPWM(InName, InPin, InFrequency, InMin, InMax));
        InElements.back()->config(JConf);
      }
    }
    #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
      Serial.println(F("    DONE - createPWM()"));
    #endif
  };
  
  void beginPWM(cHandler& Handler){
    Handler.CreateElement.insert( std::pair<String, std::function<void(JsonObject, std::vector<ELEMENT::cRoot*>&)> >("PWM", createPWM));
  };
}}}

#endif