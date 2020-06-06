/**********************************************
 * Class:	JCA_IOT_ELEMENT
 * Info: 	Die Klasse ist der Ursprung für alle JCA_IOT_IO
 *          Elemente und enthält die definition der
 *          Datenstrukturen sowie Grundfunktionen für Ein-
 *          und Ausaben, sowie die Schrnittstelle zur
 *          Kommunikation.
 * Version:
 * 	V1.0.0	Erstellt	23.04.2019	JCA
 *		-tData
 *    -tInterface
 *    -tMesh
 *    -addParam
 *    -addValue
 *    -addMesh
 **********************************************/

#ifndef _JCA_IOT_MESH_OTA_H
#define _JCA_IOT_MESH_OTA_H

#define OTA_FN "/ota_fw.json"

#ifdef ESP32
  #include <SPIFFS.h>
  #include <Update.h>
#else
  #include <FS.h>
#endif
#include "base64.h"

namespace JCA{ namespace IOT{ namespace MESH{
  struct tOtaFW {
    #ifdef ESP32
      String hardware = "ESP32";
    #else
      String hardware = "ESP8266";
    #endif
    String nodeType;
    String md5;
    size_t noPart;
    size_t partNo = 0;
  };

}}}
#endif