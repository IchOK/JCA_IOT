/**********************************************
 * Info:    Die Datei enthält alle allgemeinen
 *          Definitionen des Elements
 * Version:
 *    V1.0.0   Erstellt   30.10.2019   JCA
 **********************************************/

#ifndef _JCA_IOT_ELEMENT_define_H
#define _JCA_IOT_ELEMENT_define_H

#define JCA_IOT_ELEMENT_TRIGGER_CHANGE 0x01
#define JCA_IOT_ELEMENT_TRIGGER_CYCLE 0x02
#define JCA_IOT_ELEMENT_TRIGGER_PUSH 0x04
#define JCA_IOT_ELEMENT_TRIGGER_INIT 0x08

#define JCA_IOT_ELEMENT_ALARM_STATE_IDLE 1
#define JCA_IOT_ELEMENT_ALARM_STATE_COME 2
#define JCA_IOT_ELEMENT_ALARM_STATE_COMESEND 3
#define JCA_IOT_ELEMENT_ALARM_STATE_COMEACK 4
#define JCA_IOT_ELEMENT_ALARM_STATE_GONE 5
#define JCA_IOT_ELEMENT_ALARM_STATE_GONESEND 6
#define JCA_IOT_ELEMENT_ALARM_PRIO_ALARM 1
#define JCA_IOT_ELEMENT_ALARM_PRIO_WARNING 2
#define JCA_IOT_ELEMENT_ALARM_PRIO_INFO 3
#define JCA_IOT_ELEMENT_ALARM_RESEND_TIME 3000
#define JCA_IOT_ELEMENT_ALARM_RESEND_MAX 10

#define JCA_IOT_ELEMENT_NAME_LEN 30
#define JCA_IOT_ELEMENT_TEXT_LEN 30

#define JCA_IOT_ELEMENT_DATA_BOOL 1
#define JCA_IOT_ELEMENT_DATA_INT 2
#define JCA_IOT_ELEMENT_DATA_FLOAT 3

#define JCA_IOT_ELEMENT_TYPE_DI 1
#define JCA_IOT_ELEMENT_TYPE_DO 2
#define JCA_IOT_ELEMENT_TYPE_AI 3
#define JCA_IOT_ELEMENT_TYPE_SINGEN 3

/* Quality-Code Aufbau        QQSSSSLL
                              00xxxxxx   Bad         Schlecht
                              01xxxxxx   Uncertain   Ungewiss
                              10xxxxxx   Good      Gut
                              11xxxxxx   Good      Gut Verkettet
                              xx0000xx   NonSpec      Grund unbekannt
                              xx0001xx   Config      Falsche konfiguration
                              xx0010xx   NotConn      Es besteht keine logische Verbindung
                              xx0011xx   Device      Das Gerät hat einen Fehler
                              xx0100xx   SensDef      Der Sensor hat einen Fehler
                              xx0101xx   LastValue   Verbindungsfehler, letzter Wert verfügbar
                              xx0110xx   CommFault   Verbindungsfehler, kein letzter Wert verfügbar
                              xx0111xx   OutOfServ   Wartung aktiv
                              xx1000xx   SensRange   Sensor auserhalt der Betriebbereichs
                              xx1001xx   ValueRange   Wert ist auserhalb des definierten Bereichs
                              xx1010xx   Override   Der Wert wird lokal überschrieben
                              xx1111xx   InitValue   Startwert
                              xxxxxx00   NotLimit   Wert ist im gültigen Bereich
                              xxxxxx01   LowLimit   Wert ist am unteren Bereich begrenzt
                              xxxxxx10   HighLimit   Wert ist am oberen Bereich begrenzt
                              xxxxxx11   Constant   Wert ist eingefrohren
*/
#define JCA_IOT_ELEMENT_QC_GOOD       0b10000000
#define JCA_IOT_ELEMENT_QC_DEFAULT    0b10111111
#define JCA_IOT_ELEMENT_QC_CREAT      0b01111100
#define JCA_IOT_ELEMENT_QC_INIT       0b01111111
#define JCA_IOT_ELEMENT_QC_COMMFAULT  0b00011000
#define JCA_IOT_ELEMENT_QC_COMMLAST   0b01010100
#define JCA_IOT_ELEMENT_QC_CONFCREAT  0b00000100


#endif