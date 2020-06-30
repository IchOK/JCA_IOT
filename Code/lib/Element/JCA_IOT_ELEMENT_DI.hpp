/**********************************************
 * Class:  JCA_IOT_ELEMENT_DI
 * Info:   Abgeleitete Klasse von JCA_IOT_ELEMENT
 *       Initialisiert einen GPIO und liest
 *       diesen ein.
 * Version:
 *   V1.0.0  Erstellt   01.11.2019  JCA
 *   -add Properties
 *     -- Pin
 *     -- Pullup
 *     -- DataInput
 *   -add Methoden
 *     -- cDI
 *     -- update
 **********************************************/

#ifndef _JCA_IOT_ELEMENT_DI_H
#define _JCA_IOT_ELEMENT_DI_H

#include "JCA_IOT_Debug.h"

#include "Element/JCA_IOT_ELEMENT_Root.hpp"
#include "Element/JCA_IOT_ELEMENT_Handler.hpp"

//Include extrenal
#include <string.h>

namespace JCA{ namespace IOT{ namespace ELEMENT{
  class cDI : public cRoot {
   public:
    unsigned char Pin;
    bool Pullup;
    cDataBool  DataInput;
    
    cDI(const char* InName, const unsigned char InPin, const bool InPullup) : cRoot(InName, JCA_IOT_ELEMENT_TYPE_DI) {
      
      // init Data
      DataInput.init("Value");
      // add Data to Vector
      Data.push_back((cData*)(&DataInput));
      
      // init DI
      Pin = InPin;
      Pullup = InPullup;
      if (Pullup){
        pinMode(Pin, INPUT_PULLUP);
      } else {
        pinMode(Pin, INPUT);
      }
    }
    
    virtual void update(uint32_t DiffMillis, uint32_t Timestamp) {
      #if (DEBUGLEVEL >= JCA_IOT_DEBUG_LOOP)
        Serial.println(F(" START - cDI.update()"));
        Serial.printf("  Name:%s\r\n",Name);
      #endif
      
      //Inputs will be updated by the global Handler
      
      //Read digital Input
      DataInput.Value = digitalRead(Pin);
      
      #if (DEBUGLEVEL >= JCA_IOT_DEBUG_LOOP)
        Serial.printf("  Value:%i\r\n",DataInput.Value);
        Serial.println(F(" DONE - cDI.update()"));
      #endif
    }
    
  };
  
  void createDI(JsonObject JConf, std::vector<JCA::IOT::ELEMENT::cRoot*>& InElements){
    #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
      Serial.println(F("START - createDI()"));
    #endif
    if (JConf.containsKey("name") && JConf.containsKey("config")){
      if (JConf["config"].containsKey("pin")){
        char InName[JCA_IOT_ELEMENT_NAME_LEN];
        unsigned char InPin;
        bool InPullup = false;
        
        InPin = JConf["config"]["pin"].as<unsigned char>();
        strncpy(InName, JConf["name"].as<char*>(), JCA_IOT_ELEMENT_NAME_LEN);
         
        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
          Serial.printf("  Name:%s - Pin:%i", InName, InPin);
        #endif
        if (JConf["config"].containsKey("pullup")){
          InPullup = JConf["config"]["pullup"].as<bool>();
          #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
            Serial.printf(" - Pullup:%i\r\n", InPullup);
          #endif
        }
        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
          else{
            Serial.println("");
          }
        #endif
        InElements.push_back(new cDI(InName, InPin, InPullup));
        InElements.back()->config(JConf);
      }
    }
    #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
      Serial.println(F("DONE - createDI()"));
    #endif
  };
  
  void beginDI(cHandler& Handler){
    Handler.CreateElement.insert( std::pair<String, std::function<void(JsonObject, std::vector<ELEMENT::cRoot*>&)> >("DI", createDI));
  };
}}}

#endif