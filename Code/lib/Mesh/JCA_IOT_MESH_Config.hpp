



        //-------------------------------------------------------------------------------------------------------------
        // CONFIG - Node Einstellunegn
        #if DEBUGLEVEL >= 2
          Serial.println(F("  START - global Settings"));
        #endif
        // Das JSON-Dokument -> Objekt
        JsonObject JConfig = JDoc.as<JsonObject>();
        // Ist ein Name vorhanden ..
        if (JConfig.containsKey("name")){
          // .. wird dieser als Node-Name verwendet.
          strncpy(Name, JConfig["name"], JCA_IOT_ELEMENT_NAME_LEN);
        }else{
          // .. sonst wird die Chip-ID als eindeutige Identifikations verwendet.
          itoa(ESP.getChipId(),Name, JCA_IOT_ELEMENT_NAME_LEN);
        }
        #if DEBUGLEVEL >= 2
          Serial.print(F("    Node Name:"));
          Serial.println(Name);
          Serial.println(F("  DONE - global Settings"));
        #endif
        //-------------------------------------------------------------------------------------------------------------

