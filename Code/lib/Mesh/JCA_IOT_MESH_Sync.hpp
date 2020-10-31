/**********************************************
 * Class:	JCA_IOT_MESH_Sync
 * Info: 	Die Klasse handelt alle Funktionen rund
 *        um das direkte Schreiben und Lesen von
 *        Datenpunkten innerhalb der Elemente.
 * Version:
 * 	V1.0.0	Erstellt	02.08.2020	JCA
 *		-recvDataPush
 *    -recvDataRequest
 *    -sendDataReply
 **********************************************/

#ifndef _JCA_IOT_MESH_SYNC_H
#define _JCA_IOT_MESH_SYNC_H

//Include extrenal
#include <map>
#include <vector>
#include <ArduinoJson.h>

//Elemente einbinden für Update
#include "JCA_IOT_Debug.h"

#include "Element/JCA_IOT_ELEMENT_Root.hpp"
#include "Element/JCA_IOT_ELEMENT_define.h"
#include "Mesh/JCA_IOT_MESH_define.h"
#include "Mesh/JCA_IOT_MESH_types.h"
#include "Mesh/JCA_IOT_MESH_Client.hpp"

namespace JCA{ namespace IOT{ namespace MESH{
  class cSync{
    public:
      /***************************************
       * Methode: cSync()
       * Info:  Der Konstruktor initialisiert Server-Vektoren
       *        und Status
       ***************************************/
      cSync(){
      }
    
      /***************************************
       * Methode: recvDataPush(JsonObject Data, cClient& Client, std::vector<ELEMENT::cRoot*> *Elements, JsonObject &MeshOut)
       * Info:  Funktion zum empfangen und verarbeiten eines
       *        Data-Push telegramm. Schreibt die Daten direkt 
       *        in den Datenpunkt des Element.
       * Parameter:
       *        Data [JsonObject]
       *            Enthält das empfangene Telegram
       *        Client [JCA::IOT::MESH::cClient]
       *            Stellt die Schnittstelle für Fehlerlogs zur Verfügung
       *        Elements [vector<ELEMENT::cRoot*>]
       *            Vector mit allen Elementen der Node
       *        MeshOut [JsonObject&]
       *            Puffer für ausgehende Mesh-Telegramme
       ***************************************/
      void recvDataPush(JsonObject Data, cClient& Client, std::vector<ELEMENT::cRoot*> *Elements, JsonObject &MeshOut){
        unsigned char error;
        int e;
        int d;
        
        error = JCA_IOT_MESH_ERROR_CODE_ELEMENT;
        // Elemente nach dem Namen durchsuchen
        for (e = 0; e < Elements->size(); e++){
          if(strcmp((*Elements)[e]->Name, Data["dstElement"].as<char*>()) == 0){
            // Das Element wurde gefunden
            error = JCA_IOT_MESH_ERROR_CODE_DATA;
            // .. im Element den Datenpunkt suchen
            for (d = 0; d < (*Elements)[e]->Data.size(); d++){
              if(strcmp((*Elements)[e]->Data[d]->Name, Data["dstData"].as<char*>()) == 0){
                // Es wurde ein passender Datenpunkt gefunden
                error = JCA_IOT_MESH_ERROR_CODE_NONE;
                // Daten anhand des Datentype schreiben
                switch((*Elements)[e]->Data[d]->Type){
                  case JCA_IOT_ELEMENT_DATA_BOOL:
                    (*Elements)[e]->setDataBool(d, Data["value"].as<bool>());
                    break;
                  case JCA_IOT_ELEMENT_DATA_INT:
                    (*Elements)[e]->setDataInt(d, Data["value"].as<int32_t>());
                    break;
                  case JCA_IOT_ELEMENT_DATA_FLOAT:
                    (*Elements)[e]->setDataFloat(d, Data["value"].as<float>());
                    break;
                }
                break;
              }
            }
            break;
          }
        }
        
        // Falls kein passender Datenpunkt gefunden wurde wird ein Fehlerbericht gesendet
        if(error){
          iError ErrorReport;
          
          ErrorReport.type = JCA_IOT_MESH_ERROR_TYPE_PUSH;
          ErrorReport.data = "dstElement:";
          ErrorReport.data += Data["dstElement"].as<String>();
          ErrorReport.data += " dstData:";
          ErrorReport.data += Data["dstData"].as<String>();

          //ErrorReport.data["time"] = true;
          //ErrorReport.data["srcNodeId"] = Data["from"];
          //ErrorReport.data["failNode"] = true;
          //ErrorReport.data["dstElement"] = Data["dstElement"];
          //ErrorReport.data["dstData"] = Data["dstData"];
          //ErrorReport.data["value"] = Data["value"];
          //ErrorReport.data["error"] = error;

          Client.sendError(MeshOut, ErrorReport);
        }
      }
      
      /***************************************
       * Methode: recvDataRequest(JsonObject Data, cClient& Client, std::vector<ELEMENT::cRoot*> *Elements, JsonObject &MeshOut)
       * Info:  Funktion zum empfangen und verarbeiten eines
       *        Data-Request telegramm. liest den Datenpunkt
       *        des Elements und sendet den Wert zurück.
       * Parameter:
       *        Data [JsonObject]
       *            Enthält das empfangene Telegram
       *        Client [JCA::IOT::MESH::cClient]
       *            Stellt die Schnittstelle für Fehlerlogs zur Verfügung
       *        Elements [vector<ELEMENT::cRoot*>]
       *            Vector mit allen Elementen der Node
       *        MeshOut [JsonObject&]
       *            Puffer für ausgehende Mesh-Telegramme
       ***************************************/
      void recvDataRequest(JsonObject Data, cClient& Client, std::vector<ELEMENT::cRoot*> *Elements, JsonObject &MeshOut){
        unsigned char error;
        int e;
        int d;
        
        error = JCA_IOT_MESH_ERROR_CODE_ELEMENT;
        // Elemente nach dem Namen durchsuchen
        for (e = 0; e < Elements->size(); e++){
          if(strcmp((*Elements)[e]->Name, Data["reqElement"].as<char*>()) == 0){
            // Das Element wurde gefunden
            error = JCA_IOT_MESH_ERROR_CODE_DATA;
            // .. im Element den Datenpunkt suchen
            for (d = 0; d < (*Elements)[e]->Data.size(); d++){
              if(strcmp((*Elements)[e]->Data[d]->Name, Data["reqData"].as<char*>()) == 0){
                // Es wurde ein passender Datenpunkt gefunden
                error = JCA_IOT_MESH_ERROR_CODE_NONE;
                
                // Telegramdaten
                char nodeId[11];
                JsonArray DestNode;
                JsonObject Msg;
                
                // prüfen ob für die anfragende Node bereits ein Eintrag im Ausgabespeicher existiert
                sprintf(nodeId, "%d", Data["from"].as<uint32_t>());
                if (MeshOut.containsKey(nodeId)){
                  DestNode = MeshOut[nodeId];
                }else{
                  DestNode = MeshOut.createNestedArray(nodeId);
                }

                // Antwortnachricht erzeugen
                Msg = DestNode.createNestedObject();
                Msg["msgId"] = JCA_IOT_MESH_DATA_REPLY;
                Msg["element"] = Data["srcElement"];
                Msg["data"] = Data["srcData"];

                switch(Data["reqDataType"].as<unsigned char>()){
                  case JCA_IOT_ELEMENT_DATA_BOOL:
                    Msg["value"] = (*Elements)[e]->getDataBool(d);
                    break;
                  case JCA_IOT_ELEMENT_DATA_INT:
                    Msg["value"] = (*Elements)[e]->getDataInt(d);
                    break;
                  case JCA_IOT_ELEMENT_DATA_FLOAT:
                    Msg["value"] = (*Elements)[e]->getDataFloat(d);
                    break;
                }
                break;
              }
            }
            break;
          }
        }
        
        // Falls kein passender Datenpunkt gefunden wurde wird ein Fehlerbericht gesendet
        if(error){
          iError ErrorReport;
          
          ErrorReport.type = JCA_IOT_MESH_ERROR_TYPE_REQUEST;
          ErrorReport.data = "reqElement:";
          ErrorReport.data += Data["reqElement"].as<String>();
          ErrorReport.data += " reqData:";
          ErrorReport.data += Data["reqData"].as<String>();
          //ErrorReport.data["srcNodeId"] = Data["from"];
          //ErrorReport.data["srcElement"] = Data["srcElement"];
          //ErrorReport.data["srcData"] = Data["srcData"];
          //ErrorReport.data["error"] = error;

          Client.sendError(MeshOut, ErrorReport);
        }
      }
  };
}}}
#endif