/**********************************************
 * Class:   JCA_IOT_ELEMENT_DO
 * Info:    Abgeleitete Klasse von JCA_IOT_ELEMENT
 *          Initialisiert einen GPIO und schreibt
 *          diesen.
 * Version:
 *    V1.0.0   Erstellt    01.11.2019  JCA
 *    -add Properties
 *       -- Pin
 *       -- InputValue
 *       -- AutoValue
 *    -add Methoden
 *       -- cDO
 *       -- update
 *    V1.0.1   Erweiterung  03.08.2020  JCA
 *    -add Properties
 *       -- ManMode
 *       -- ManValue
 **********************************************/

#ifndef _JCA_IOT_ELEMENT_DO_H
#define _JCA_IOT_ELEMENT_DO_H

#include "JCA_IOT_Debug.h"

#include "Element/JCA_IOT_ELEMENT_Root.hpp"
#include "Element/JCA_IOT_ELEMENT_Handler.hpp"

//Include extrenal

namespace JCA{ namespace IOT{ namespace ELEMENT{
  class cDO : public cRoot {
    public:
      unsigned char Pin;
      bool        Invert;
      cInputBool  InputValue;
      cDataBool   ManMode;
      cDataBool   AutoValue;
      cDataBool   ManValue;
      cDataBool   Value;
      
      cDO(const char* InName, const unsigned char InPin, bool InInvert) : cRoot(InName, JCA_IOT_ELEMENT_TYPE_DI) {
         
        // init Input
        InputValue.init("Input");
        // add Input to Vector
        Input.push_back((cInput*)(&InputValue));
        
        // init Data
        ManMode.init("Manual");
        AutoValue.init("AutoValue");
        ManValue.init("ManualValue");
        Value.init("Value");
        // add Data to Vector
        Data.push_back((cData*)(&ManMode));
        Data.push_back((cData*)(&AutoValue));
        Data.push_back((cData*)(&ManValue));
        Data.push_back((cData*)(&Value));
        
        // init DO
        Pin = InPin;
        pinMode(Pin, OUTPUT);
        Invert = InInvert;
      }
      
      virtual void update(uint32_t DiffMillis, uint32_t Timestamp) {
        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_LOOP)
          Serial.println(F(" START - cDO.update()"));
          Serial.printf("  Name:%s\r\n",Name);
        #endif
        
        //Inputs will be updated by the global Handler
        
        //Copy InputValue to AutoValue
        AutoValue.Value = InputValue.Value;
        
        if(ManMode.Value){
          //Manual-Mode: Move ManValue to Output
          Value.Value = ManValue.Value;
        }else{
          //Auto-Mode: Move AutoValue to Output
          Value.Value = AutoValue.Value;
          // .. updating ManValue 
          ManValue.Value = AutoValue.Value;
        }
        
        //Write Value to Output-Pin
        if (Invert){
          digitalWrite(Pin, !Value.Value);
        }else{
          digitalWrite(Pin, Value.Value);
        }
        
        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_LOOP)
          Serial.printf("  Value:%i\r\n",AutoValue.Value);
          Serial.println(F(" DONE - cDO.update()"));
        #endif
      }
  };

  void createDO(JsonObject JConf, std::vector<JCA::IOT::ELEMENT::cRoot*>& InElements){
    #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
       Serial.println(F("START - createDO()"));
    #endif
    if (JConf.containsKey("name") && JConf.containsKey("config")){
      if (JConf["config"].containsKey("pin")){
        char InName[JCA_IOT_ELEMENT_NAME_LEN];
        unsigned char InPin;
        bool Invert = false;
        
        InPin = JConf["config"]["pin"].as<unsigned char>();
        strncpy(InName, JConf["name"].as<char*>(), JCA_IOT_ELEMENT_NAME_LEN);
        
        if (JConf["config"].containsKey("invert")){
          Invert = JConf["config"]["invert"].as<bool>();
        }
        
        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
          Serial.printf("  Name:%s - Pin:%i\r\n", InName, InPin);
        #endif
        InElements.push_back(new cDO(InName, InPin, Invert));
        InElements.back()->config(JConf);
      }
    }
    #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
      Serial.println(F("DONE - createDO()"));
    #endif
  };
   
  void beginDO(cHandler& Handler){
    Handler.CreateElement.insert( std::pair<String, std::function<void(JsonObject, std::vector<ELEMENT::cRoot*>&)> >("DO", createDO));
  };
}}}

#endif