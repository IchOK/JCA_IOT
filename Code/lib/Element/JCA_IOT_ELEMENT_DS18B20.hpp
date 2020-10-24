/**********************************************
 * Class:  JCA_IOT_ELEMENT_DS18B20
 * Info:   Abgeleitete Klasse von JCA_IOT_ELEMENT
 *       Liest die Temperatur von einem DS18B20 Sensor ein.
 * Version:
 *   V1.0.0  Erstellt   24.10.2020  JCA
 *   -add Properties
 *     -- Raw, Resend, LastUpdate
 *     -- Type, Addr, Watchdog, UpdateTime
 *     -- Value
 *   -add Methoden
 *     -- cDS18B20
 *     -- update
 **********************************************/

#ifndef _JCA_IOT_ELEMENT_DS18B20_H
#define _JCA_IOT_ELEMENT_DS18B20_H

#include "JCA_IOT_Debug.h"

#include "Element/JCA_IOT_ELEMENT_HW_OneWire.hpp"

#include "Element/JCA_IOT_ELEMENT_Root.hpp"
#include "Element/JCA_IOT_ELEMENT_Handler.hpp"

#define JCA_IOT_ELEMENT_DS18B20_TYPE_S 0x10
#define JCA_IOT_ELEMENT_DS18B20_TYPE_B 0x28
#define JCA_IOT_ELEMENT_DS18B20_TYPE_22 0x22

#define JCA_IOT_ELEMENT_DS18B20_CMD_CONV 0x44
#define JCA_IOT_ELEMENT_DS18B20_CMD_READ 0xBE
#define JCA_IOT_ELEMENT_DS18B20_CMD_WRITE 0x4E
#define JCA_IOT_ELEMENT_DS18B20_CMD_COPY 0x48
#define JCA_IOT_ELEMENT_DS18B20_CMD_RECALL 0xB8
#define JCA_IOT_ELEMENT_DS18B20_CMD_POWER 0xB4

//Include extrenal
#include <string.h>

namespace JCA{ namespace IOT{ namespace ELEMENT{
  class cDS18B20 : public cRoot {
    private:
      uint8_t   Raw[12];
      int32_t   Resend;
      uint32_t  LastUpdate;
      //Config
      uint8_t   Type;
      uint8_t   Addr[8];
      uint8_t   Watchdog;
      uint8_t   UpdateTime;
      
    public:
      cDataFloat  Value;
      
      cDS18B20(const char* InName, const uint8_t* InAddr, const uint8_t InWatchdog, const uint8_t InUpdateTime) : cRoot(InName, JCA_IOT_ELEMENT_TYPE_DS18B20) {
        int i;
        
        // init Data
        Value.init("Value");
        // add Data to Vector
        Data.push_back((cData*)(&Value));
        
        // init DS18B20
        for(i=0;i<8;i++){
          this->Addr[i] = InAddr[i];
        }
        switch (this->Addr[0]) {
          case JCA_IOT_ELEMENT_DS18B20_TYPE_S:
            this->Type = 1;
            break;
          case JCA_IOT_ELEMENT_DS18B20_TYPE_B:
            this->Type = 0;
            break;
          case JCA_IOT_ELEMENT_DS18B20_TYPE_22:
            this->Type = 0;
            break;
        }
      }
      
      virtual void update(uint32_t DiffMillis, uint32_t Timestamp) {
        int i;
        int16_t raw;
        uint8_t cfg;
        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_LOOP)
          Serial.println(F(" START - cDS18B20.update()"));
          Serial.printf("  Name:%s\r\n",Name);
        #endif
        
        // Check Watchdog
        if(this->LastUpdate + (uint32_t)this->Watchdog * 1000 < Timestamp ) {
          if(this->LastUpdate > 0) {
            this->QC = JCA_IOT_ELEMENT_QC_COMMLAST;
          } else {
            this->QC = JCA_IOT_ELEMENT_QC_COMMFAULT;
          }
        }
        
        // If Resend counts to 0 resend convertion Request
        if(this->Resend <= 0){
          // OneWire Bus is free to write Data
          if(JCA_OneWire.reset()){
            JCA_OneWire.select(this->Addr);
            JCA_OneWire.write(JCA_IOT_ELEMENT_DS18B20_CMD_CONV);
            this->Resend = this->UpdateTime;
          }
        }
        // checking if Convetion is Done
        else{
          // OneWire Bus is free to write Data
          if(JCA_OneWire.reset()){
            // send Data Request
            JCA_OneWire.select(this->Addr);
            JCA_OneWire.write(JCA_IOT_ELEMENT_DS18B20_CMD_READ);
            JCA_OneWire.read_bytes(this->Raw,9);
            // check data Consistens
            if(OneWire::crc8(this->Raw,8) == this->Raw[8]){
              raw = (this->Raw[1] << 8) | Raw[0];
              if (this->Type) {
                // Type DS18S20 has special Data-Setup, allways use 9 bit resolition
                raw = raw << 3;
                if (this->Raw[7] == 0x10) {
                  raw = (raw & 0xFFF0) + 12 - this->Raw[6];
                }
              } else {
                byte cfg = (this->Raw[4] & 0x60);
                switch (cfg) {
                  case 0x00:
                    // 9 bit resolution, 93.75 ms
                    raw = raw & ~7;
                    break;
                  case 0x20:
                    // 10 bit res, 187.5 ms
                    raw = raw & ~3;
                    break;
                  case 0x40:
                    // 11 bit res, 375 ms
                    raw = raw & ~1;
                }
              }
              Value.Value = (float)raw / 16.0;
              this->Resend = 0;
              this->LastUpdate = Timestamp;
              this->QC = JCA_IOT_ELEMENT_QC_GOOD;
            }
          }
          this->Resend -= DiffMillis;
        }
        
        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_LOOP)
          Serial.printf("  Value:%i\r\n",Value.Value);
          Serial.println(F(" DONE - cDS18B20.update()"));
        #endif
      }
  };
  
  void createDS18B20(JsonObject JConf, std::vector<JCA::IOT::ELEMENT::cRoot*>& InElements){
    #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
      Serial.println(F("START - createDS18B20()"));
    #endif
    if (JConf.containsKey("name") && JConf.containsKey("config")){
      if (JConf["config"].containsKey("pin")){
        int i;
        char InName[JCA_IOT_ELEMENT_NAME_LEN];
        uint8_t   InAddr[8];
        uint8_t   InWatchdog = 60;
        uint8_t   InUpdateTime = 10;
        
        strncpy(InName, JConf["name"].as<char*>(), JCA_IOT_ELEMENT_NAME_LEN);
        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
          Serial.printf("  Name:%s", InName);
        #endif
        for (i=0; i<8; i++) {
          InAddr[i] = JConf["config"]["addr"].as<JsonArray>()[i].as<uint8_t>();
          #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
            if (i==0){
              Serial.printf("- Adr:%02X", InName, InAddr[0]);
            }else{
              Serial.printf("-%02X", InName, InAddr[i]);
            }
          #endif
        }
        
        if (JConf["config"].containsKey("watchdog")){
          InWatchdog = JConf["config"]["watchdog"].as<uint8_t>();
          #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
            Serial.printf(" - Watchdog:%i\r\n", InWatchdog);
          #endif
        }
        
        if (JConf["config"].containsKey("updateTime")){
          InUpdateTime = JConf["config"]["updateTime"].as<uint8_t>();
          #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
            Serial.printf(" - UpdateTime:%i\r\n", InUpdateTime);
          #endif
        }
        
        #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
          Serial.println("");
        #endif
        InElements.push_back(new cDS18B20(InName, InAddr, InWatchdog, InUpdateTime));
        InElements.back()->config(JConf);
      }
    }
    #if (DEBUGLEVEL >= JCA_IOT_DEBUG_STARTUP)
      Serial.println(F("DONE - createDS18B20()"));
    #endif
  };
  
  void beginDS18B20(cHandler& Handler){
    Handler.CreateElement.insert( std::pair<String, std::function<void(JsonObject, std::vector<ELEMENT::cRoot*>&)> >("DS18B20", createDS18B20));
  };
}}}

#endif