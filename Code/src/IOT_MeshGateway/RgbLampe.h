//######################################################################
//-- INCLUDE --
//######################################################################
//#include <ArduinoJson.h>

//######################################################################
//-- HARDWARE --
//######################################################################
  #define HOSTNAME    "Wohnzimmer"
  #define DEBUGLEVEL  5
  #define DEBUGLEVEL_M  0
  #define PIN_DEBUG     LED_BUILTIN
  #define PIN_R         D5
  #define PIN_G         D6
  #define PIN_B         D7
  #define PIN_BUTTON    D8
  #define DELAY_BUTTON  100
  #define DELAY_COUNT   500
  #define DELAY_DIMM    500
  #define RANGE_DIMM    2000

  bool firstRun;
  bool LED;
  bool outButton;
  bool inButton;
  bool oldButton;
  bool dimmActiv;
  bool dimmAdd;
  int countButton;
  int colorMode;
  int dimmIntens;
  unsigned long timeNew;
  unsigned long timeOld;
  unsigned long timeDelta;
  unsigned long timeButtonIn;
  unsigned long timeButtonDimm;
  unsigned long timeButtonCount;
  unsigned int intensR;
  unsigned int intensG;
  unsigned int intensB;

//######################################################################
//-- SETUP --
//######################################################################
void hwSetup(){
  firstRun = true;
  pinMode(PIN_BUTTON, INPUT);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  LED = true;
  dimmAdd = false;
  dimmIntens = 100;
  intensR = 1023;
  intensG = 1023;
  intensB = 1023;
  digitalWrite(PIN_LED, LED);
  analogWrite(PIN_R, intensR);
  analogWrite(PIN_G, intensG);
  analogWrite(PIN_B, intensB);
  outButton = digitalRead(PIN_BUTTON);
  timeOld = millis();
}

//######################################################################
//-- MESH --
//######################################################################
void meshSetDataJ(const JsonObject command, JsonArray &jaData){
  if (String("ColorR").equals(command["name"].as<String>())){
    intensR = command["value"].as<int>();
    if (intensR > 1023){
      intensR = 1023;
    }
    JsonObject joRet = jaData.createNestedObject();
    joRet["name"] = command["name"];
    joRet["value"] = intensR;
  }else if (String("ColorG").equals(command["name"].as<String>())){
    intensG = command["value"].as<int>();
    if (intensG > 1023){
      intensG = 1023;
    }
    JsonObject joRet = jaData.createNestedObject();
    joRet["name"] = command["name"];
    joRet["value"] = intensG;
  }else if (String("ColorB").equals(command["name"].as<String>())){
    intensG = command["value"].as<int>();
    if (intensB > 1023){
      intensB = 1023;
    }
    JsonObject joRet = jaData.createNestedObject();
    joRet["name"] = command["name"];
    joRet["value"] = intensB;
  }else if (String("Dimm").equals(command["name"].as<String>())){
    dimmIntens = command["value"].as<int>();
    if (dimmIntens > 100){
      dimmIntens = 100;
    }else if (dimmIntens < 0){
      dimmIntens = 0;
    }
    JsonObject joRet = jaData.createNestedObject();
    joRet["name"] = command["name"];
    joRet["value"] = intensB;
  }
}

void meshGetDataJ(const JsonObject command, JsonArray &jaData){
  if (String("*").equals(command["name"].as<String>())){
    #if (DEBUGLEVEL >= 5)
      Serial.println(" - Get * -");
    #endif
    JsonObject joRet;
    joRet = jaData.createNestedObject();
    joRet["name"] = "Button";
    joRet["value"] = outButton;
    joRet = jaData.createNestedObject();
    joRet["name"] = "ColorR";
    joRet["value"] = intensR;
    joRet = jaData.createNestedObject();
    joRet["name"] = "ColorG";
    joRet["value"] = intensG;
    joRet = jaData.createNestedObject();
    joRet["name"] = "ColorB";
    joRet["value"] = intensB;
    joRet = jaData.createNestedObject();
    joRet["name"] = "Dimm";
    joRet["value"] = dimmIntens;
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
  //------------------------------------------------
  // Loop Time
  //------------------------------------------------
  timeNew = millis();
  timeDelta = timeNew - timeOld;
  timeOld = timeNew;
  
  //------------------------------------------------
  // Button entprellen
  //------------------------------------------------
  inButton = digitalRead(PIN_BUTTON);
  if (outButton != inButton){
    if (timeButtonIn >= DELAY_BUTTON)){
      outButton = inButton;
      timeButtonIn = 0;
    }else{
      timeButtonIn += timeDelta;
    }

  //------------------------------------------------
  // Button Multi-Klick
  //------------------------------------------------
  if (outButton && !oldButton){
    timeButtonCount = 0;
    countButton++;
  }
  oldButton = outButton;
  if (countButton > 0){
    if (timeButtonCount >= DELAY_COUNT)){
      // Button Funktion
      switch (countButton){
        case 1:
          colorMode++;
          break;
        case 2:
          colorMode = 0;
          break;
          
        //TTTTT  OOO  DDDD   OOO 
        //  T   O   O D   D O   O
        //  T   O   O O   D O   O
        //  T    OOO  DDDD   OOO 
      }
      switch (colorMode){
        case 1:
          intensR = 512;
          intensG = 512;
          intensB = 512;
          break;
        case 2:
          intensR = 1023;
          intensG = 1023;
          intensB = 1023;
          break;
        case 3:
          intensR = 0;
          intensG = 1023;
          intensB = 0;
          break;
        case 4:
          intensR = 0;
          intensG = 0;
          intensB = 1023;
          break;
          
        //TTTTT  OOO  DDDD   OOO 
        //  T   O   O D   D O   O
        //  T   O   O O   D O   O
        //  T    OOO  DDDD   OOO 
        default:
          intensR = 0;
          intensG = 0;
          intensB = 0;
          colorMode = 0;
      }
      countButton = 0;
    }else{
      timeButtonCount += timeDelta;
    }
  }

  //------------------------------------------------
  // Button Dauerklick
  //------------------------------------------------
  if (outButton){
    if (inButton){
      //Dimmen aktiv Intensität auf und ab faden, mit Delay am Max-Ausschlag
      if (dimmActiv){
        if (dimmAdd){
          timeButtonDimm += timeDelay;
          if (timeButtonDimm > RANGE_DIMM){
            dimmIntens = 100;
            if (timeButtonDimm >= RANGE_DIMM + DELAY_DIMM){
              timeButtonDimm = RANGE_DIMM;
              dimmAdd = false;
            }
          }else{
            dimmIntens = timeButtonDimm * 100 / RANGE_DIMM;
          }
        }else{
          timeButtonDimm -= timeDelay;
          if (timeButtonDimm < 0){
            dimmIntens = 0;
            if (timeButtonDimm <=  -DELAY_DIMM){
              timeButtonDimm = 0;
              dimmAdd = true;
            }
          }else{
            dimmIntens = timeButtonDimm * 100 / RANGE_DIMM;
          }
        }
      
      // Verzögerung bis Dimmern einsetzt
      }else{ // if (dimmActiv)
        if (timeButtonDimm >= DELAY_DIMM){
          // Startwert berechnen
          timeButtonDimm = dimmIntens * RANGE_DIMM / 100;
          dimmActiv = true;
        }else{
          timeButtonDimm += timeDelay;
        }
      }
    }
    
  // Button nicht getrückt
  }else if (oldButton){ // if (outButton)
    timeButtonDimm = 0;
    dimmActiv = false;
  }
  
  //------------------------------------------------
  // Reboot LED zurück setzen
  //------------------------------------------------
  if (outButton && firstRun){
    LED = false;
    firstRun = false;
  }

  //------------------------------------------------
  // Ausgänge schreiben
  //------------------------------------------------
  digitalWrite(PIN_LED, LED);
  analogWrite(PIN_R, intensR);
  analogWrite(PIN_G, intensG);
  analogWrite(PIN_B, intensB);
  
  
}
