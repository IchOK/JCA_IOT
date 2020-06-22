/**********************************************
 * Info: 	Die Sammlung enthält alle Klassen des
 *          Element-Vektor Input
 * Version:
 * 	V1.0.0	Erstellt    30.10.2019	JCA
 *		- add Klassen
 *       -- cInput
 *       -- cInputBool
 *       -- cInputInt
 *       -- cInputFloat
 **********************************************/

#ifndef _JCA_IOT_ELEMENT_INPUT_H
#define _JCA_IOT_ELEMENT_INPUT_H

#include "Element/JCA_IOT_ELEMENT_define.h"

namespace JCA{ namespace IOT{ namespace ELEMENT{
  /**********************************************
   * Class:	JCA_IOT_ELEMENT_cInput
   * Info: 	cInput ist die Root-Klasse aller Inputs
   *          und enthält grundlegende Funktionen
   * Version:
   * 	V1.0.0	Erstellt	   30.10.2019  JCA
   *    -add Properties
   *       -- Name, ElementIndex, TagIndex, Tvpe, QC
   *    -add Methoden
   *       -- cInput
   *       -- init
   *       -- config
   *       -- isGood
   **********************************************/
  class cInput{
    public:
      char Name[JCA_IOT_ELEMENT_NAME_LEN];
      unsigned char ElementIndex;
      unsigned char DataIndex;
      unsigned char Type;
      unsigned char QC;
      
      cInput(const unsigned char InType) {
        Type = InType;
        QC = JCA_IOT_ELEMENT_QC_CREAT;
      }
      
      void init(const char* InName) {
        strncpy(Name, InName, JCA_IOT_ELEMENT_NAME_LEN);
        QC = JCA_IOT_ELEMENT_QC_INIT;
      }
      
      void config(const unsigned char Element, const unsigned char Data) {
        ElementIndex = Element;
        DataIndex = Data;
        QC = JCA_IOT_ELEMENT_QC_DEFAULT;
      }
      
      bool isGood() {
        return (QC && 0b10000000) != 0b00000000;
      }
   };
   
  /**********************************************
   * Class:	JCA_IOT_ELEMENT_cInputBool
   * Info: 	Bildet einen Digitalen Eingang des Elements ab
   * Version:
   * 	V1.0.0	Erstellt	   30.10.2019	JCA
   *    -add Properties
   *       -- Value
   *    -add Methoden
   *       -- cInputBool
   **********************************************/
  class cInputBool : public cInput{
    public:
      bool Value;
      
      cInputBool() : cInput(JCA_IOT_ELEMENT_DATA_BOOL) {
        Value = false;
      }
  };
   
  /**********************************************
   * Class:	JCA_IOT_ELEMENT_cInputInt
   * Info: 	Bildet einen Ganzzahl Eingang des Elements ab
   * Version:
   * 	V1.0.0	Erstellt	   30.10.2019	JCA
   *    -add Properties
   *       -- Value
   *    -add Methoden
   *       -- cInputInt
   **********************************************/
  class cInputInt : public cInput{
    public:
      int32_t Value;
      
      cInputInt() : cInput(JCA_IOT_ELEMENT_DATA_INT) {
        Value = 0;
      }
  };
   
  /**********************************************
   * Class:	JCA_IOT_ELEMENT_cInputFloat
   * Info: 	Bildet einen Gleitkommazahl Eingang des Elements ab
   * Version:
   * 	V1.0.0	Erstellt	30.10.2019	JCA
   *    -add Properties
   *       -- Value
   *    -add Methoden
   *       -- cInputFloat
   **********************************************/
  class cInputFloat : public cInput{
    public:
      float Value;
      
      cInputFloat() : cInput(JCA_IOT_ELEMENT_DATA_FLOAT) {
        Value = 0.0;
      }
   };
   
}}}
#endif