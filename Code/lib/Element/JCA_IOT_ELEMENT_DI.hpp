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
    
    virtual void update(uint32_t DiffMillis) {
      #if DEBUGLEVEL >= 3
        Serial.println(F(" START - cDI.update()"));
        Serial.printf("  Name:%s\r\n",Name);
      #endif
      
      //Inputs will be updated by the global Handler
      DataInput.Value = digitalRead(Pin);
      #if DEBUGLEVEL >= 3
        Serial.printf("  Value:%i\r\n",DataInput.Value);
        Serial.println(F(" DONE - cDI.update()"));
      #endif
    }
    
  };
  
  void createDI(JsonObject JConf, std::vector<JCA::IOT::ELEMENT::cRoot*>& InElements){
    #if DEBUGLEVEL >= 2
      Serial.println(F("START - createDI()"));
    #endif
    if (JConf.containsKey("name") && JConf.containsKey("config")){
      if (JConf["config"].containsKey("pin")){
        char InName[JCA_IOT_ELEMENT_NAME_LEN];
        unsigned char InPin;
        bool InPullup = false;
        
        InPin = JConf["config"]["pin"].as<unsigned char>();
        strncpy(InName, JConf["name"].as<char*>(), JCA_IOT_ELEMENT_NAME_LEN);
         
        #if DEBUGLEVEL >= 2
          Serial.printf("  Name:%s - Pin:%i\r\n", InName, InPin);
        #endif
        if (JConf["config"].containsKey("pullup")){
          InPullup = JConf["config"]["pullup"].as<bool>();
        }
        InElements.push_back(new cDI(InName, InPin, InPullup));
        InElements.back()->config(JConf);
      }
    }
    #if DEBUGLEVEL >= 2
      Serial.println(F("DONE - createDI()"));
    #endif
  };
  
  void beginDI(cHandler& Handler){
    Handler.CreateElement.insert( std::pair<String, std::function<void(JsonObject, std::vector<ELEMENT::cRoot*>&)> >("DI", createDI));
  };
}}}

#endif