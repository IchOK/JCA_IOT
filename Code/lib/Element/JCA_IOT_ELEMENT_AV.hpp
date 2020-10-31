/**********************************************
 * Class:  JCA_IOT_ELEMENT_AV
 * Info:   Abgeleitete Klasse von JCA_IOT_ELEMENT
 *         Virtueller analoger Datenpunkt
 * Version:
 *   V1.0.0  Erstellt   31.10.2020  JCA
 *   -add Properties
 *     -- Value
 *   -add Methoden
 *     -- cAV
 *     -- update
 **********************************************/

#ifndef _JCA_IOT_ELEMENT_AV_H
#define _JCA_IOT_ELEMENT_AV_H

#include "JCA_IOT_Debug.h"

#include "Element/JCA_IOT_ELEMENT_Root.hpp"
#include "Element/JCA_IOT_ELEMENT_Handler.hpp"

//Include extrenal
#include <string.h>

namespace JCA{ namespace IOT{ namespace ELEMENT{
  class cAV : public cRoot {
   public:
    cDataFloat  Value;
    
    cAV(const char* InName) : cRoot(InName, JCA_IOT_ELEMENT_TYPE_AV) {
      
      // init Data
      Value.init("Value");
      // add Data to Vector
      Data.push_back((cData*)(&Value));
      
    }
    
    virtual void update(uint32_t DiffMillis, uint32_t Timestamp) {
      //There is no function behinde that Datapoint
      
      #if (DEBUGLEVEL >= JCA_IOT_DEBUG_LOOP)
        Serial.println(F(" START - cAV.update()"));
        Serial.printf("  Name:%s\r\n",Name);
        Serial.println(F(" DONE - cAV.update()"));
      #endif
    }
    
  };
  
  void createAV(JsonObject JConf, std::vector<JCA::IOT::ELEMENT::cRoot*>& InElements){
    #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
      Serial.println(F("    START - createAV()"));
    #endif
    if (JConf.containsKey("name")){
      char InName[JCA_IOT_ELEMENT_NAME_LEN];
      strncpy(InName, JConf["name"].as<char*>(), JCA_IOT_ELEMENT_NAME_LEN);
        
      #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
        Serial.printf("      Name:%s\r\n", InName);
      #endif

      InElements.push_back(new cAV(InName));
      InElements.back()->config(JConf);
    }
    #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
      Serial.println(F("    DONE - createAV()"));
    #endif
  };
  
  void beginAV(cHandler& Handler){
    Handler.CreateElement.insert( std::pair<String, std::function<void(JsonObject, std::vector<ELEMENT::cRoot*>&)> >("AV", createAV));
  };
}}}

#endif