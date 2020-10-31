/**********************************************
 * Class:   JCA_IOT_ELEMENT_OR
 * Info:    Abgeleitete Klasse von JCA_IOT_ELEMENT
 *          Bildet eine logische Oder-Verkn�pfung.
 *          Zur Zeit noch statisch mit 4 Eing�ngen
 * Version:
 *    V1.0.0   Erstellt    03.08.2020  JCA
 *    -add Properties
 *       -- Input1-4
 *       -- Output
 *    -add Methoden
 *       -- cOR
 *       -- update
 **********************************************/

#ifndef _JCA_IOT_ELEMENT_OR_H
#define _JCA_IOT_ELEMENT_OR_H

#include "JCA_IOT_Debug.h"

#include "Element/JCA_IOT_ELEMENT_Root.hpp"
#include "Element/JCA_IOT_ELEMENT_Handler.hpp"

//Include extrenal

namespace JCA{ namespace IOT{ namespace ELEMENT{
  class cOR : public cRoot {
    public:
      cInputBool  Input1;
      cInputBool  Input2;
      cInputBool  Input3;
      cInputBool  Input4;
      cDataBool   Value;
      
      cOR(const char* InName) : cRoot(InName, JCA_IOT_ELEMENT_TYPE_OR) {
         
        // TODO: Create InputVector dynamic at Config
        // init Input
        Input1.init("Input1");
        Input2.init("Input2");
        Input3.init("Input3");
        Input4.init("Input4");
        // add Input to Vector
        Input.push_back((cInput*)(&Input1));
        Input.push_back((cInput*)(&Input2));
        Input.push_back((cInput*)(&Input3));
        Input.push_back((cInput*)(&Input4));
        
        // init Data
        Value.init("Value");
        // add Data to Vector
        Data.push_back((cData*)(&Value));
      }
      
      virtual void update(uint32_t DiffMillis, uint32_t Timestamp) {
        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_LOOP)
          Serial.println(F(" START - cOR.update()"));
          Serial.printf("  Name:%s\r\n",Name);
        #endif
        
        //Inputs will be updated by the global Handler
        
        //Link Inputs to Value
        // TODO: If Input-Voctor dynamic -> Loop over all Input
        Value.Value = Input1.Value || Input2.Value || Input3.Value || Input4.Value;
        
        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_LOOP)
          Serial.printf("  Value:%i\r\n",Value.Value);
          Serial.println(F(" DONE - cOR.update()"));
        #endif
      }
  };

  void createOR(JsonObject JConf, std::vector<JCA::IOT::ELEMENT::cRoot*>& InElements){
    #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
      Serial.println(F("    START - createOR()"));
    #endif
    if (JConf.containsKey("name")){
      char InName[JCA_IOT_ELEMENT_NAME_LEN];
      strncpy(InName, JConf["name"].as<char*>(), JCA_IOT_ELEMENT_NAME_LEN);
         
      #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
        Serial.printf("      Name:%s\r\n", InName);
      #endif
      InElements.push_back(new cOR(InName));
      InElements.back()->config(JConf);
    }
    #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
       Serial.println(F("    DONE - createOR()"));
    #endif
  };
   
  void beginOR(cHandler& Handler){
    Handler.CreateElement.insert( std::pair<String, std::function<void(JsonObject, std::vector<ELEMENT::cRoot*>&)> >("OR", createOR));
  };
}}}

#endif