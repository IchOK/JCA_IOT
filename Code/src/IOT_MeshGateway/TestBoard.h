//######################################################################
//-- INCLUDE --
//######################################################################
//#include <ArduinoJson.h>

//######################################################################
//-- HARDWARE --
//######################################################################
  #define HOSTNAME    "TestBoard"
  #define DEBUGLEVEL  5
  #define DEBUGLEVEL_M  0
  #define PIN_LED     LED_BUILTIN

  bool LED;

//######################################################################
//-- SETUP --
//######################################################################
  void hwSetup(){
  pinMode(PIN_LED, OUTPUT);
  LED = true;
  digitalWrite(PIN_LED, LED);
  }

//######################################################################
//-- MESH --
//######################################################################
void meshSetDataJ(const JsonObject command, JsonArray &jaData){
  if (String("LED").equals(command["name"].as<String>())){
    LED = command["value"].as<int>() == 1;
    JsonObject joRet = jaData.createNestedObject();
    joRet["name"] = command["name"];
    joRet["value"] = LED;
  }
}

void meshGetDataJ(const JsonObject command, JsonArray &jaData){
  if (String("LED").equals(command["name"].as<String>())){
    #if (DEBUGLEVEL >= 5)
      Serial.println(" - Get LED -");
    #endif
    JsonObject joRet = jaData.createNestedObject();
    joRet["name"] = command["name"];
    joRet["value"] = LED;
    #if (DEBUGLEVEL >= 5)
      Serial.print(" -  Array.size() = ");
      Serial.println(jaData.size());
      Serial.print(" -  Array = ");
      serializeJson(jaData, Serial);
      Serial.println();
    #endif
  }else if (String("*").equals(command["name"].as<String>())){
    #if (DEBUGLEVEL >= 5)
      Serial.println(" - Get * -");
    #endif
    JsonObject joRet;
    joRet = jaData.createNestedObject();
    joRet["name"] = "LED";
    joRet["value"] = LED;
    joRet = jaData.createNestedObject();
    joRet["name"] = "Test_Stern";
    joRet["value"] = "Stern";
    #if (DEBUGLEVEL >= 5)
      Serial.print(" -  Array.size() = ");
      Serial.println(jaData.size());
      Serial.print(" -  Array = ");
      serializeJson(jaData, Serial);
      Serial.println();
    #endif
  }
}


//######################################################################
//-- LOOP --
//######################################################################
  void fcLoop(){
    digitalWrite(PIN_LED, LED);
  }
