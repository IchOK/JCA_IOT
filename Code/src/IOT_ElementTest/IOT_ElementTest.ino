#define DEBUGLEVEL 2

#include <ArduinoJson.h>

#include "JCA_IOT_ELEMENT.h"

using namespace JCA::IOT;
using namespace JCA::IOT::ELEMENT;
uint32_t StoreMillis;
//std::vector<cRoot*> Elements;

//JsonObject as Pointer
JsonObject JConfig;

cHandler IotHandler;
void setup() {
  bool RetIO;
  #if DEBUGLEVEL > 0
    Serial.begin(115200);
    Serial.println("add Elements...");
  #endif
  //Add possible Elements
  beginDI(IotHandler);
  beginDO(IotHandler);
  
  //Konfig Handler
  RetIO = IotHandler.config(JConfig);

  StoreMillis = millis();
}

void loop() {
  uint32_t Timestamp;
  uint32_t ActMillis = millis();
  uint32_t DiffMillis = ActMillis - StoreMillis;
  #if DEBUGLEVEL >= 3
    delay(2000);
  #endif
  StoreMillis = ActMillis;
  IotHandler.update(DiffMillis, Timestamp);
}
