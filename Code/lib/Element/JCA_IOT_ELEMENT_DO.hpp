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
 *       -- DataOutput
 *    -add Methoden
 *       -- cDO
 *       -- update
 **********************************************/

#ifndef _JCA_IOT_ELEMENT_DO_H
#define _JCA_IOT_ELEMENT_DO_H

#include "JCA_IOT_ELEMENT.hpp"
#include "JCA_IOT_ELEMENT_HANDLER.hpp"
#define JCA_IOT_ELEMENT_TYPE_DO 2

//Include extrenal

namespace JCA{ namespace IOT{ namespace ELEMENT{
   class cDO : public cRoot {
    public:
      unsigned char Pin;
      cInputBool  InputValue;
      cDataBool   DataOutput;
      
      cDO(const char* InName, const unsigned char InPin) : cRoot(InName, JCA_IOT_ELEMENT_TYPE_DI) {
         
         // init Input
         InputValue.init("Input");
         // add Input to Vector
         Input.push_back((cInput*)(&InputValue));
         
         // init Data
         DataOutput.init("Value");
         // add Data to Vector
         Data.push_back((cData*)(&DataOutput));
         
         // init DO
         Pin = InPin;
         pinMode(Pin, OUTPUT);
      }
      
      virtual void update(uint32_t DiffMillis) {
         #if DEBUGLEVEL >= 3
            Serial.println(F(" START - cDO.update()"));
            Serial.printf("  Name:%s\r\n",Name);
         #endif
         
         //Inputs will be updated by the global Handler
         DataOutput.Value = InputValue.Value;
         digitalWrite(Pin, DataOutput.Value);
         #if DEBUGLEVEL >= 3
            Serial.printf("  Value:%i\r\n",DataOutput.Value);
            Serial.println(F(" DONE - cDO.update()"));
         #endif
      }
      
   };

   void createDO(JsonObject JConf, std::vector<JCA::IOT::ELEMENT::cRoot*>& InElements){
      #if DEBUGLEVEL >= 2
         Serial.println(F("START - createDO()"));
      #endif
      if (JConf.containsKey("name") && JConf.containsKey("pin")){
         char InName[JCA_IOT_ELEMENT_NAME_LEN];
         unsigned char InPin;
         
         InPin = JConf["pin"].as<unsigned char>();
         strcpy(InName, JConf["name"].as<char*>());
          
         #if DEBUGLEVEL >= 2
            Serial.printf("  Name:%s - Pin:%i\r\n", InName, InPin);
         #endif
         InElements.push_back(new cDO(InName, InPin));
         InElements.back()->config(JConf);
      }
      #if DEBUGLEVEL >= 2
         Serial.println(F("DONE - createDI()"));
      #endif
   };
   
   void beginDO(cHandler& Handler){
      Handler.CreateElement.insert( std::pair<String, std::function<void(JsonObject, std::vector<ELEMENT::cRoot*>&)> >("DO", createDO));
   };
}}}

#endif