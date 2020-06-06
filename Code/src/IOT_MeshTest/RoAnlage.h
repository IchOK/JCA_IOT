//######################################################################
//-- INCLUDE --
//######################################################################
//#include <ArduinoJson.h>

//######################################################################
//-- HARDWARE --
//######################################################################
//#define PIN_RELAIS  LED_BUILTIN
//#define PIN_BUTTON  5 //D1
//#define PIN_LSH     4 //D2
//#define PIN_LSL     0 //D3
#define PIN_RELAIS  4
#define PIN_BUTTON  1
#define PIN_LSH     5
#define PIN_LSL     3
#define HOSTNAME    "RoAnlage"
#define DEBUGLEVEL  0
#define DEBUGLEVEL_M  0
#define IN_DELAY 1000
bool PumpeAktiv;
bool LSL;
bool LSH;
bool Button;
bool oldButton;
unsigned long delayLSL;
unsigned long delayLSH;
unsigned long delayButton;
#if (DEBUGLEVEL >= 8)
  unsigned long oldMillis;
#endif

//######################################################################
//-- SETUP --
//######################################################################
void hwSetup(){
  pinMode(PIN_RELAIS, OUTPUT);
  digitalWrite(PIN_RELAIS, false);
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  pinMode(PIN_LSH, INPUT_PULLUP);
  pinMode(PIN_LSL, INPUT_PULLUP);
  PumpeAktiv = false;
  LSL = false;
  LSH = false;
  Button = false;
  delayLSL = millis();
  delayLSH = delayLSL;
  delayButton = delayLSL;
  #if (DEBUGLEVEL >= 8)
    oldMillis = millis();
  #endif
}

//######################################################################
//-- MESH --
//######################################################################
/*String meshSetData(const String strName, const String strValue){
  if (strName == "PumpeAktiv"){
    bool xValue = strValue.toInt() == 1;
    PumpeAktiv = xValue;
    return "{\"name\":\"" + strName + "\",\"value\":\"" + xValue + "\"}";
  }else{
    return "";
  }
}*/
void meshSetDataJ(const JsonObject command, JsonArray &jaData){
  if (String("PumpeAktiv").equals(command["name"].as<String>())){
    bool xValue = command["value"].as<int>() == 1;
    PumpeAktiv = xValue;
    JsonObject joRet = jaData.createNestedObject();
    joRet["name"] = command["name"];
    joRet["value"] = xValue;
  }
}

/*String meshGetData(const String strName){
  if (strName == "PumpeAktiv"){
      return "{\"name\":\"" + strName + "\",\"value\":\"" + PumpeAktiv + "\"}";
  }else if (strName == "LSL"){
      return "{\"name\":\"" + strName + "\",\"value\":\"" + LSL + "\"}";
  }else if (strName == "LSH"){
      return "{\"name\":\"" + strName + "\",\"value\":\"" + LSH + "\"}";
  }else if (strName == "Button"){
      return "{\"name\":\"" + strName + "\",\"value\":\"" + Button + "\"}";
  }else{
    return "";
  }
}*/
void meshGetDataJ(const JsonObject command, JsonArray &jaData){
  if (String("PumpeAktiv").equals(command["name"].as<String>())){
    JsonObject joRet = jaData.createNestedObject();
    joRet["name"] = command["name"];
    joRet["value"] = PumpeAktiv;
  }else if (String("LSL").equals(command["name"].as<String>())){
    JsonObject joRet = jaData.createNestedObject();
    joRet["name"] = command["name"];
    joRet["value"] = LSL;
  }else if (String("LSH").equals(command["name"].as<String>())){
    JsonObject joRet = jaData.createNestedObject();
    joRet["name"] = command["name"];
    joRet["value"] = LSH;
  }else if (String("Button").equals(command["name"].as<String>())){
    JsonObject joRet = jaData.createNestedObject();
    joRet["name"] = command["name"];
    joRet["value"] = Button;
  }else if (String("*").equals(command["name"].as<String>())){
    JsonObject joRet;
    joRet = jaData.createNestedObject();
    joRet["name"] = "PumpeAktiv";
    joRet["value"] = PumpeAktiv;
    joRet = jaData.createNestedObject();
    joRet["name"] = "LSL";
    joRet["value"] = LSL;
    joRet = jaData.createNestedObject();
    joRet["name"] = "LSH";
    joRet["value"] = LSH;
    joRet = jaData.createNestedObject();
    joRet["name"] = "Button";
    joRet["value"] = Button;
  }
}


//######################################################################
//-- LOOP --
//######################################################################
void fcLoop(){
  bool SetPumpe;
  if(Button == digitalRead(PIN_BUTTON)){
    if(millis() - delayButton >= IN_DELAY){
      Button = !digitalRead(PIN_BUTTON);
      delayButton = millis();
    }
  }else{
    delayButton = millis();
  }
  if(LSL == !digitalRead(PIN_LSL)){
    if(millis() - delayLSL >= IN_DELAY){
      LSL = digitalRead(PIN_LSL);
      delayLSL = millis();
    }
  }else{
    delayLSL = millis();
  }
  if(LSH == !digitalRead(PIN_LSH)){
    if(millis() - delayLSH >= IN_DELAY){
      LSH = digitalRead(PIN_LSH);
      delayLSH = millis();
    }
  }else{
    delayLSH = millis();
  }
  if(Button && !oldButton){
    PumpeAktiv = !PumpeAktiv;
  }
  oldButton = Button;
  SetPumpe = PumpeAktiv;
  if(LSL || LSH){
    SetPumpe = false;
  }
  digitalWrite(PIN_RELAIS, SetPumpe);
  PumpeAktiv = SetPumpe;
  #if (DEBUGLEVEL >= 8)
    if(oldMillis - millis() >= 1000){
      Serial.print("LSL : ");
      Serial.println(LSL);
      Serial.print("LSH : ");
      Serial.println(LSH);
      Serial.print("Btn : ");
      Serial.println(Button);
      Serial.print("Pump: ");
      Serial.println(PumpeAktiv);
      oldMillis = oldMillis + 1000;
    }
  #endif
}
