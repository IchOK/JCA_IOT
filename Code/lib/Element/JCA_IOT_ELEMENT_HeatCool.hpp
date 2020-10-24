/**********************************************
 * Class:   JCA_IOT_ELEMENT_HeatCool
 * Info:    Abgeleitete Klasse von JCA_IOT_ELEMENT
 *          Stellt einen 3-Punkt-Regler dar mit separat
 *          einstellbaren Schwellwerten.
 *          - Hysterese
 *          - Umschaltverzögerung
 * Version:
 *    V1.0.0   Erstellt    24.10.2020  JCA
 *    -add Properties
 *      -- ProcessValue, Setpoint
 *      -- DelatHeat, DeltaCool
 *      -- Hyst, Delay
 *      -- Heat, Cool
 *    -add Methoden
 *      -- cHeatCool
 *      -- update
 **********************************************/

#ifndef _JCA_IOT_ELEMENT_HEATCOOL_H
#define _JCA_IOT_ELEMENT_HEATCOOL_H

#include "JCA_IOT_Debug.h"

#include "Element/JCA_IOT_ELEMENT_Root.hpp"
#include "Element/JCA_IOT_ELEMENT_Handler.hpp"

//Include extrenal

namespace JCA{ namespace IOT{ namespace ELEMENT{
  class cHeatCool : public cRoot {
    public:
      cInputFloat ProcessValue;
      cInputFloat Setpoint;
      
      //Parameter
      cDataFloat  DeltaHeat;
      cDataFloat  DeltaCool;
      cDataFloat  Hyst;
      cDataInt    Delay;
      
      //Output
      cDataBool   Heat;
      cDataBool   Cool;
      
      //Internal
      cDataInt    State;
      uint32_t    DelayCounter;
      
      cHeatCool(const char* InName) : cRoot(InName, JCA_IOT_ELEMENT_TYPE_HEATCOOL) {
        
        // init Input
        ProcessValue.init("ProcessValue");
        Setpoint.init("Setpoint");
        // add Input to Vector
        Input.push_back((cInput*)(&ProcessValue));
        Input.push_back((cInput*)(&Setpoint));
        
        // init Parameter
        DeltaHeat.init("DeltaHeat");
        DeltaCool.init("DeltaCool");
        Hyst.init("Hyst");
        Delay.init("Delay");
        Heat.init("Heat");
        Cool.init("Cool");
        State.init("State");
        // add Data to Vector
        Data.push_back((cData*)(&DeltaHeat));
        Data.push_back((cData*)(&DeltaCool));
        Data.push_back((cData*)(&Hyst));
        Data.push_back((cData*)(&Delay));
        Data.push_back((cData*)(&Heat));
        Data.push_back((cData*)(&Cool));
        Data.push_back((cData*)(&State));
        
        // init Internal
        State.Value = JCA_IOT_ELEMENT_STATE_IDLE;
        DelayCounter = 0;
      }
      
      virtual void update(uint32_t DiffMillis, uint32_t Timestamp) {
        float Delta;
        
        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_LOOP)
          Serial.println(F(" START - cHeatCool.update()"));
          Serial.printf("  Name:%s\r\n",Name);
        #endif
        
        //Inputs will be updated by the global Handler
        
        //Get Delta from Setpoint
        Delta = ProcessValue.Value - Setpoint.Value;
        
        //Check State to know how to Handle the Delta
        switch(State.Value){
          case JCA_IOT_ELEMENT_STATE_IDLE:
            //It's possible to change to Heat or Cool
            if (Delta < DeltaHeat.Value) {
              #if (DEBUGLEVEL >= JCA_IOT_DEBUG_INTERNAL)
                Serial.printf("INTERNAL [%010i] %s : Start Heating\r\n",Timestamp, Name);
              #endif
              Heat.Value = true;
              State.Value = JCA_IOT_ELEMENT_STATE_HEAT;
            }else if (Delta > DeltaCool.Value) {
              #if (DEBUGLEVEL >= JCA_IOT_DEBUG_INTERNAL)
                Serial.printf("INTERNAL [%010i] %s : Start Cooling\r\n",Timestamp, Name);
              #endif
              Cool.Value = true;
              State.Value = JCA_IOT_ELEMENT_STATE_COOL;
            }
            break;
            
          case JCA_IOT_ELEMENT_STATE_HEAT:
            //Check if still Heating
            if (Heat.Value) {
              //check if Heating is done
              if (Delta > DeltaHeat.Value + Hyst.Value) {
                Heat.Value = false;
                DelayCounter = 0;
              }
            } else {
              //Restart Heating or count Delay-Timer
              if (Delta < DeltaHeat.Value) {
                #if (DEBUGLEVEL >= JCA_IOT_DEBUG_INTERNAL)
                  Serial.printf("INTERNAL [%010i] %s : Restart Heating\r\n",Timestamp, Name);
                #endif
                Heat.Value = true;
              } else {
                DelayCounter += DiffMillis;
                if (DelayCounter >= (uint32_t)Delay.Value * 1000) {
                  State.Value = JCA_IOT_ELEMENT_STATE_IDLE;
                  DelayCounter = 0;
                }
              }
            }
            break;
            
          case JCA_IOT_ELEMENT_STATE_COOL:
            //Check if still Cooling
            if (Cool.Value) {
              //check if Cooling is done
              if (Delta < DeltaCool.Value - Hyst.Value) {
                Cool.Value = false;
                DelayCounter = 0;
              }
            } else {
              //Restart Cooling or count Delay-Timer
              if (Delta > DeltaCool.Value) {
                #if (DEBUGLEVEL >= JCA_IOT_DEBUG_INTERNAL)
                  Serial.printf("INTERNAL [%010i] %s : Restart Cooling\r\n",Timestamp, Name);
                #endif
                Cool.Value = true;
              } else {
                DelayCounter += DiffMillis;
                if (DelayCounter >= (uint32_t)Delay.Value * 1000) {
                  State.Value = JCA_IOT_ELEMENT_STATE_IDLE;
                  DelayCounter = 0;
                }
              }
            }
            break;
        }
        
        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_LOOP)
          Serial.print("  Delta: ");
          Serial.println(Delta);
          Serial.printf("  Heat:  %i", Heat.Value);
          Serial.printf("  Cool:  %i", Cool.Value);
          Serial.println(F(" DONE - cHeatCool.update()"));
        #endif
      }
  };

  void createHeatCool(JsonObject JConf, std::vector<JCA::IOT::ELEMENT::cRoot*>& InElements){
    #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
      Serial.println(F("START - createHeatCool()"));
    #endif
    if (JConf.containsKey("name")){
      char InName[JCA_IOT_ELEMENT_NAME_LEN];
      strncpy(InName, JConf["name"].as<char*>(), JCA_IOT_ELEMENT_NAME_LEN);
         
      #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
        Serial.printf("  Name:%s\r\n", InName);
      #endif
      InElements.push_back(new cHeatCool(InName));
      InElements.back()->config(JConf);
    }
    #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
       Serial.println(F("DONE - createHeatCool()"));
    #endif
  };
   
  void beginHeatCool(cHandler& Handler){
    Handler.CreateElement.insert( std::pair<String, std::function<void(JsonObject, std::vector<ELEMENT::cRoot*>&)> >("HeatCool", createHeatCool));
  };
}}}

#endif