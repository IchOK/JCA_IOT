/**********************************************
 * Class:   JCA_IOT_ELEMENT_ROOT
 * Info:    Die Klasse ist der Ursprung für alle 
 *          JCA_IOT_ELEMENT Klassen.
 *          Sie definiert den Aufbau der Eingänge
 *          und Datenpunkte sowie die Methode zur
 *          Konfiguration über ein Json-Objekt.
 * Version:
 *    V1.0.0   Erstellt    01.11.2019  JCA
 *    -add Properties
 *       -- Input Vector
 *       -- Data Vector
 *       -- Name, ErrorElement, Tvpe, QC
 *       -- Child Template 
 *    -add Methoden
 *       -- cRoot  (mit Child Template)
 *       -- update (mit Child Template)
 *       -- config
 **********************************************/

#ifndef _JCA_IOT_ELEMENT_ROOT_H
#define _JCA_IOT_ELEMENT_ROOT_H

#include "JCA_IOT_ELEMENT_define.h"
#include "JCA_IOT_ELEMENT_Input.hpp"
#include "JCA_IOT_ELEMENT_Data.hpp"
#include "JCA_IOT_ELEMENT_Archiv.hpp"

//Include extrenal
#include <vector>
#include <ArduinoJson.h>

namespace JCA{ namespace IOT{ namespace ELEMENT{
   class cRoot{
    public:
      std::vector<cInput*> Input;
      std::vector<cData*> Data;
      std::vector<cArchiv*> Archiv;
      std::vector<cAlarm*> Alarm;
      char Name[JCA_IOT_ELEMENT_NAME_LEN];
      char ErrorElement[JCA_IOT_ELEMENT_NAME_LEN];
      unsigned char Type;
      unsigned char QC;
      
      /*
      //------------------------------
      // Template Child-Class 
      //------------------------------
      // 
      // create Input
      cInputBool  In1;
      cInputInt   In2;
      cInputFloat In3;
      // 
      // create Datapoint
      cDataBool   Data1;
      cDataInt    Data2;
      cDataFloat  Data3;
      //
      //------------------------------
      */
      
      cRoot(const char* InName, const unsigned char InType){
         strncpy(Name, InName, JCA_IOT_ELEMENT_NAME_LEN);
         Type = InType;
         QC = JCA_IOT_ELEMENT_QC_CREAT;
         
         /*
         //------------------------------
         // Template Child-Class
         //------------------------------
         // 
         // init Inputs
         In1.init("Eingang1_B");
         In2.init("Eingang2_I");
         In3.init("Eingang3_F");
         // add Input to Vector
         Input.push_back((cInput*)(&In1));
         Input.push_back((cInput*)(&In2));
         Input.push_back((cInput*)(&In3));
         //
         // init Data
         Data1.init("DatenBool");
         Data2.init("DatenInt");
         Data3.init("DatenFloat");
         // add Data to Vector
         Data.push_back((cData*)(&Data1));
         Data.push_back((cData*)(&Data2));
         Data.push_back((cData*)(&Data3));
         //
         //------------------------------
         */ 
      }
      
      virtual void update(uint32_t DiffMillis, uint32_t Timestamp) {
         
         /*
         //------------------------------
         // Template Child-Class
         //------------------------------
         //
         //Inputs will be updated by the global Handler
         //
         if (In1.Value){
            Data2.Value = 21;
         } else {
            Data2.Value = -18;
         }
         
         Data1.Value = In2.Value > 298;
         
         Data3.Value = In3.Value;
         //
         //------------------------------
         */
      }
      
      void setInputBool(const unsigned char Index, const bool InValue){
         (static_cast<cInputBool*>(Input[Index]))->Value = InValue;
      }
      void setInputInt(const unsigned char Index, const int32_t InValue){
         (static_cast<cInputInt*>(Input[Index]))->Value = InValue;
      }
      void setInputFloat(const unsigned char Index, const float InValue){
         (static_cast<cInputFloat*>(Input[Index]))->Value = InValue;
      }
      bool getInputBool(const unsigned char Index){
         switch(Input[Index]->Type){
            case JCA_IOT_ELEMENT_DATA_BOOL:
               return static_cast<bool>((static_cast<cInputBool*>(Input[Index]))->Value);
               break;
            case JCA_IOT_ELEMENT_DATA_INT:
               return static_cast<bool>((static_cast<cInputInt*>(Input[Index]))->Value);
               break;
            case JCA_IOT_ELEMENT_DATA_FLOAT:
               return static_cast<bool>((static_cast<cInputFloat*>(Input[Index]))->Value);
               break;
         }
      }
      int32_t getInputInt(const unsigned char Index){
         switch(Input[Index]->Type){
            case JCA_IOT_ELEMENT_DATA_BOOL:
               return static_cast<int32_t>((static_cast<cInputBool*>(Input[Index]))->Value);
               break;
            case JCA_IOT_ELEMENT_DATA_INT:
               return static_cast<int32_t>((static_cast<cInputInt*>(Input[Index]))->Value);
               break;
            case JCA_IOT_ELEMENT_DATA_FLOAT:
               return static_cast<int32_t>((static_cast<cInputFloat*>(Input[Index]))->Value);
               break;
         }
      }
      float getInputFloat(const unsigned char Index){
         switch(Input[Index]->Type){
            case JCA_IOT_ELEMENT_DATA_BOOL:
               return static_cast<float>((static_cast<cInputBool*>(Input[Index]))->Value);
               break;
            case JCA_IOT_ELEMENT_DATA_INT:
               return static_cast<float>((static_cast<cInputInt*>(Input[Index]))->Value);
               break;
            case JCA_IOT_ELEMENT_DATA_FLOAT:
               return static_cast<float>((static_cast<cInputFloat*>(Input[Index]))->Value);
               break;
         }
      }
      
      void setDataBool(const unsigned char Index, const bool InValue){
         (static_cast<cDataBool*>(Data[Index]))->Value = InValue;
      }
      void setDataInt(const unsigned char Index, const int32_t InValue){
         (static_cast<cDataInt*>(Data[Index]))->Value = InValue;
      }
      void setDataFloat(const unsigned char Index, const float InValue){
         (static_cast<cDataFloat*>(Data[Index]))->Value = InValue;
      }
      bool getDataBool(const unsigned char Index){
         switch(Data[Index]->Type){
            case JCA_IOT_ELEMENT_DATA_BOOL:
               return static_cast<bool>((static_cast<cDataBool*>(Data[Index]))->Value);
               break;
            case JCA_IOT_ELEMENT_DATA_INT:
               return static_cast<bool>((static_cast<cDataInt*>(Data[Index]))->Value);
               break;
            case JCA_IOT_ELEMENT_DATA_FLOAT:
               return static_cast<bool>((static_cast<cDataFloat*>(Data[Index]))->Value);
               break;
         }
      }
      int32_t getDataInt(const unsigned char Index){
         switch(Data[Index]->Type){
            case JCA_IOT_ELEMENT_DATA_BOOL:
               return static_cast<int32_t>((static_cast<cDataBool*>(Data[Index]))->Value);
               break;
            case JCA_IOT_ELEMENT_DATA_INT:
               return static_cast<int32_t>((static_cast<cDataInt*>(Data[Index]))->Value);
               break;
            case JCA_IOT_ELEMENT_DATA_FLOAT:
               return static_cast<int32_t>((static_cast<cDataFloat*>(Data[Index]))->Value);
               break;
         }
      }
      float getDataFloat(const unsigned char Index){
         switch(Data[Index]->Type){
            case JCA_IOT_ELEMENT_DATA_BOOL:
               return static_cast<float>((static_cast<cDataBool*>(Data[Index]))->Value);
               break;
            case JCA_IOT_ELEMENT_DATA_INT:
               return static_cast<float>((static_cast<cDataInt*>(Data[Index]))->Value);
               break;
            case JCA_IOT_ELEMENT_DATA_FLOAT:
               return static_cast<float>((static_cast<cDataFloat*>(Data[Index]))->Value);
               break;
         }
      }
      
      float setArchiv(const unsigned char Index, float Value, uint32_t Timestamp){
         Archiv[Index].setValue(Value, Timestamp, false);
      }
      
      float pushArchiv(const unsigned char Index, float Value, uint32_t Timestamp){
         Archiv[Index].setValue(Value, Timestamp, true);
      }
      
      float setAlarm(const unsigned char Index, bool Value, uint32_t Timestamp){
         Alarm[Index].setAlarm(Value, Timestamp);
      }
      
      float ackAlarm(const unsigned char State){
         Alarm[Index].ackAlarm(State);
      }
      
      void config(JsonObject& JsonObj){
        JsonArray JsonConfig;
        char ConfigName[JCA_IOT_ELEMENT_NAME_LEN];
         
        QC = JCA_IOT_ELEMENT_QC_INIT;
        //Init Inputs
        if (JsonObj.containsKey("input")){
          JsonConfig = JsonObj["input"];
          for(JsonObject ConfigInput : JsonConfig){
            //check JsonObject for needed Keys
            if (ConfigInput.containsKey("name") && ConfigInput.containsKey("element") && ConfigInput.containsKey("data")){
              strncpy(ConfigName, ConfigInput["name"], JCA_IOT_ELEMENT_NAME_LEN);
              //search Input in Vector by Name
              for (int i = 0; i < Input.size(); i++){
                if (strcmp(Input[i]->Name, ConfigName) == 0){
                  Input[i]->config(ConfigInput["element"].as<unsigned char>(), ConfigInput["data"].as<unsigned char>());
                  break;
                }
              }
            }
          }
        }
         
        //Init Data
        if (JsonObj.containsKey("data")){
          JsonConfig = JsonObj["data"];
          for(JsonObject ConfigData : JsonConfig){
            //check JsonObject for needed Keys
            if (ConfigData.containsKey("name") && ConfigData.containsKey("value")){
              strncpy(ConfigName, ConfigData["name"], JCA_IOT_ELEMENT_NAME_LEN);
              //search Data in Vector by Name
              for (int i = 0; i < Data.size(); i++){
                if (strcmp(Data[i]->Name, ConfigName) == 0){
                  switch(Data[i]->Type){
                    case JCA_IOT_ELEMENT_DATA_BOOL:
                      (static_cast<cDataBool*>(Data[i]))->config(ConfigData["value"].as<bool>());
                      break;
                    case JCA_IOT_ELEMENT_DATA_INT:
                      (static_cast<cDataInt*>(Data[i]))->config(ConfigData["value"].as<int32_t>());
                      break;
                    case JCA_IOT_ELEMENT_DATA_FLOAT:
                      (static_cast<cDataFloat*>(Data[i]))->config(ConfigData["value"].as<float>());
                      break;
                  }
                  break;
                }
              }
            }
          }
        }
         
        //Init Archivs
        if (JsonObj.containsKey("archiv")){
          JsonConfig = JsonObj["archiv"];
          for(JsonObject ConfigArchiv : JsonConfig){
            //check JsonObject for needed Keys
            if (ConfigArchiv.containsKey("name")){
              bool OnChange = false;
              bool OnCylce = false;
              uint32_t Time = 0;
              float Hyst = 0.0;
              float Value = 0.0;
              strncpy(ConfigName, ConfigArchiv["name"], JCA_IOT_ELEMENT_NAME_LEN);
              //search Archiv in Vector by Name
              for (int i = 0; i < Archiv.size(); i++){
                if (strcmp(Archiv[i]->Name, ConfigName) == 0){
                  if (ConfigArchiv.containsKey("onChange") && ConfigArchiv.containsKey("hyst")){
                    OnChange = ConfigArchiv["onChange"].as<bool>;
                    Hyst = ConfigArchiv["hyst"].as<float>;
                  }
                  if (ConfigArchiv.containsKey("onCycle") && ConfigArchiv.containsKey("time")){
                    OnCycle = ConfigArchiv["onCycle"].as<bool>;
                    Time = ConfigArchiv["time"].as<uint32_t>;
                  }
                  if (ConfigArchiv.containsKey("value")){
                    Value = ConfigArchiv["value"].as<float>;
                  }
                  Archiv[i]->config(OnChange, OnCycle, Hyst, Time, Value);
                  break;
                }
              }
            }
          }
        }
         
        //Init Alarme
        if (JsonObj.containsKey("alarm")){
          JsonConfig = JsonObj["alarm"];
          for(JsonObject ConfigAlarm : JsonConfig){
            //check JsonObject for needed Keys
            if (ConfigAlarm.containsKey("name")){
              strncpy(ConfigName, ConfigAlarm["name"], JCA_IOT_ELEMENT_NAME_LEN);
              //search Alarm in Vector by Name
              for (int i = 0; i < Alarm.size(); i++){
                if (strcmp(Alarm[i]->Name, ConfigName) == 0){
                  if (ConfigAlarm.containsKey("text") && ConfigAlarm.containsKey("prio")){
                    unsigned char State = 0;
                    if (ConfigAlarm.containsKey("state")){
                      State = ConfigAlarm["state"].as<unsigned char>;
                    }
                    Alarm[i]->config(ConfigAlarm["state"].as<char*>, ConfigAlarm["state"].as<unsigned char>, State);
                    break;
                  }
                }
              }
            }
          }
        }
         
        //Check Initialisation
        for (int i = 0; i < Input.size(); i++){
          if (!(Input[i]->isGood())){
            QC = Input[i]->QC;
            strcpy(ErrorElement, Input[i]->Name);
            break;
          }
        }
        if (QC == JCA_IOT_ELEMENT_QC_INIT){
          for (int i = 0; i < Data.size(); i++){
            if (!(Data[i]->isGood())){
              QC = Data[i]->QC;
              strcpy(ErrorElement, Data[i]->Name);
              break;
            }
          }
        }
        if (QC == JCA_IOT_ELEMENT_QC_INIT){
          for (int i = 0; i < Archiv.size(); i++){
            if (!(Archiv[i]->isGood())){
              QC = Archiv[i]->QC;
              strcpy(ErrorElement, Archiv[i]->Name);
              break;
            }
          }
        }
        if (QC == JCA_IOT_ELEMENT_QC_INIT){
          for (int i = 0; i < Alarm.size(); i++){
            if (!(Alarm[i]->isGood())){
              QC = Alarm[i]->QC;
              strcpy(ErrorElement, Alarm[i]->Name);
              break;
            }
          }
        }
      }
   };
}}}

#endif