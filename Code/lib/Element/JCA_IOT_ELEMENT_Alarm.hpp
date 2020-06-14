/**********************************************
 * Class:	JCA_IOT_ELEMENT_cAlarm
 * Info: 	cAlarm bildet einen Alarm ab mit allen Funktionen
 * Version:
 * 	V1.0.0	Erstellt	   14.06.2020	JCA
 *    -add Properties
 *       -- Name, Text, State, Timestamp, Prio, wd
 *    -add Methoden
 *       -- cAlarm
 *       -- init
 *       -- config
 *       -- update
 *       -- isGood
 **********************************************/

#ifndef _JCA_IOT_ELEMENT_ALARM_H
#define _JCA_IOT_ELEMENT_ALARM_H

#include "JCA_IOT_ELEMENT_define.h"

namespace JCA{ namespace IOT{ namespace ELEMENT{
  class cAlarm{
    public:
      char Name[JCA_IOT_ELEMENT_NAME_LEN];
      char Text[JCA_IOT_ELEMENT_TEXT_LEN];
      unsigned char QC;
      unsigned char State;
      unsigned char Prio;
      bool Value;
      uint32_t TimestampCome;
      uint32_t TimestampGone;
      uint32_t wd;
      uint16_t Counter;
      
      cAlarm() {
        QC = JCA_IOT_ELEMENT_QC_CREAT;
        State = JCA_IOT_ELEMENT_ALARM_STATE_IDLE;
      }
	  
	   void init(const char* InName) {
        strncpy(Name, InName, JCA_IOT_ELEMENT_NAME_LEN);
        QC = JCA_IOT_ELEMENT_QC_INIT;
      }
      
      void config(const char *InText, const unsigned char InPrio, const unsigned char InState) {
        strncpy(Text, InText, JCA_IOT_ELEMENT_TEXT_LEN);
        Prio = InPrio;
        if (InState != 0) {
          State = InState;
          wd = 0;
        }
        QC = JCA_IOT_ELEMENT_QC_DEFAULT;
      }

      void update(uint32_t DiffMillis) {
        // AlarmNachricht erneut senden, falls nicht Quittiert
        switch (State) {
          case JCA_IOT_ELEMENT_ALARM_STATE_COMESEND:
            wd += DiffMillis;
            if (wd >= JCA_IOT_ELEMENT_ALARM_RESEND_TIME) {
              if (Counter < JCA_IOT_ELEMENT_ALARM_RESEND_MAX) {
                State = JCA_IOT_ELEMENT_ALARM_STATE_COME;
                Counter += 1;
              }else{
                State = JCA_IOT_ELEMENT_ALARM_STATE_COMEACK;
              }
            }
            break;
          case JCA_IOT_ELEMENT_ALARM_STATE_COMEACK:
            if (!Value) {
              State = JCA_IOT_ELEMENT_ALARM_STATE_GONE;
                wd = 0;
                Counter = 0;
            }
            break;
          case JCA_IOT_ELEMENT_ALARM_STATE_GONESEND:
            wd += DiffMillis;
            if (wd >= JCA_IOT_ELEMENT_ALARM_RESEND_TIME) {
              if (Counter < JCA_IOT_ELEMENT_ALARM_RESEND_MAX) {
                State = JCA_IOT_ELEMENT_ALARM_STATE_GONE;
                Counter += 1;
              }else{
                State = JCA_IOT_ELEMENT_ALARM_STATE_IDLE;
              }
            }
            break;
          case JCA_IOT_ELEMENT_ALARM_STATE_IDLE:
            if (Value) {
              State = JCA_IOT_ELEMENT_ALARM_STATE_COME;
                wd = 0;
                Counter = 0;
            }
            break;
          }
        }
      }
      
      void setAlarm(bool InValue, uint32_t InTimestamp) {
        if (isGood()){
          Value = InValue;
          if (Value) {
            TimestampCome = InTimestamp;
            if (State == JCA_IOT_ELEMENT_ALARM_STATE_IDLE) {
              State = JCA_IOT_ELEMENT_ALARM_STATE_COME;
            }
          }else{
            TimestampGone = InTimestamp;
            if (State == JCA_IOT_ELEMENT_ALARM_STATE_COMEACK) {
              State = JCA_IOT_ELEMENT_ALARM_STATE_GONE;
            }
          }
        QC = JCA_IOT_ELEMENT_QC_GOOD;
        }
      }

      void ackAlarm(unsigned char InState) {
        if (InState == State) {
          switch{State) {
            case JCA_IOT_ELEMENT_ALARM_STATE_COMES
              State = JCA_IOT_ELEMENT_ALARM_STATE_COMESEND;
              break;
            case JCA_IOT_ELEMENT_ALARM_STATE_COMESEND
              State = JCA_IOT_ELEMENT_ALARM_STATE_COMEACK;
              break;
            case JCA_IOT_ELEMENT_ALARM_STATE_GONE
              State = JCA_IOT_ELEMENT_ALARM_STATE_GONESEND;
              break;
            case JCA_IOT_ELEMENT_ALARM_STATE_GONESEND
              State = JCA_IOT_ELEMENT_ALARM_STATE_IDLE;
              break;
          }
        }
      }

      bool isGood() {
         return (QC && 0b10000000) != 0b00000000;
      }
  };
   
}}}
#endif