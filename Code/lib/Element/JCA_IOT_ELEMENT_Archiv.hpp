/**********************************************
 * Class:	JCA_IOT_ELEMENT_cArchiv
 * Info: 	cArchiv bildet einen Archivwert ab mit allen Funktionen
 * Version:
 * 	V1.0.0	Erstellt	   14.06.2020	JCA
 *    -add Properties
 *       -- Name, Value, QC, Trigger, Timestamp, onChange
 *       -- onCycle, ChangeHyst, CycleTime, Counter, LastValue
 *    -add Methoden
 *       -- cArchiv
 *       -- init
 *       -- config
 *       -- update
 *       -- isGood
 **********************************************/

#ifndef _JCA_IOT_ELEMENT_ARCHIV_H
#define _JCA_IOT_ELEMENT_ARCHIV_H

#include "JCA_IOT_ELEMENT_define.h"

namespace JCA{ namespace IOT{ namespace ELEMENT{
  class cArchiv{
    public:
      char Name[JCA_IOT_ELEMENT_NAME_LEN];
      float Value;
      unsigned char QC;
      unsigned char Trigger;
      uint32_t Timestamp;
      bool OnChange;
      bool OnCycle;
      float ChangeHyst;
      uint32_t CycleTime;
      uint32_t Counter;
      float LastValue;
      
      cArchiv() {
        QC = JCA_IOT_ELEMENT_QC_CREAT;
      }
	  
	   void init(const char* InName) {
        strncpy(Name, InName, JCA_IOT_ELEMENT_NAME_LEN);
        QC = JCA_IOT_ELEMENT_QC_INIT;
        Counter = 0;
      }
      
      void config(const bool InOnChange, const bool InOnCycle, const float InHyst, const uint32_t InTime, const float InValue) {
        Value = InValue;
        LastValue = InValue;
        OnChange = InOnChange;
        OnCycle = InOnCycle;
        ChangeHyst = InHyst;
        CycleTime = InTime;
        QC = JCA_IOT_ELEMENT_QC_DEFAULT;
      }

      void update(const uint32_t DiffMillis) {
        if (isGood()){
          // Zyklischen Trigger prüfen
          if (OnCycle) {
            Counter += DiffMillis;
            if (Counter >= CycleTime) {
              Trigger |= JCA_IOT_ELEMENT_TRIGGER_CYCLE;
              Counter -= CycleTime;
            }
          }
          
          // Änderungs-Trigger prüfen
          if (OnChange) {
            if (ChangeHyst <= abs(Value - LastValue) ) {
              Trigger |= JCA_IOT_ELEMENT_TRIGGER_CHANGE;
              LastValue = Value;
            }
          }
        }
      }
      
      void setValue(const float InValue, const uint32_t InTimestamp, const bool Push) {
        switch (QC){
          case JCA_IOT_ELEMENT_QC_DEFAULT:
            QC = JCA_IOT_ELEMENT_QC_GOOD;
            Value = InValue;
            LastValue = Value;
            Trigger = JCA_IOT_ELEMENT_TRIGGER_INIT;
            Timestamp = InTimestamp;
            break;
          case JCA_IOT_ELEMENT_QC_GOOD:
            Value = InValue;
            Timestamp = InTimestamp;
            break;
        }
      }

      bool isGood() {
         return (QC && 0b10000000) != 0b00000000;
      }
  };
   
}}}
#endif