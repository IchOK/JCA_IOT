/**********************************************
 * Definitionen:
 * Info: 	Datensturkturen für das Mesh-Handling
 * Version:
 * 	V1.0.0	Erstellt	14.06.2020	JCA
 **********************************************/

#ifndef _JCA_IOT_MESH_TYPES_H
#define _JCA_IOT_MESH_TYPES_H

#include "Element/JCA_IOT_ELEMENT_define.h"

namespace JCA{ namespace IOT{ namespace MESH{
  
  /***************************************
   * Client
   ***************************************/
  enum clientState{
    cltInit,
    cltRequesting,
    cltOnline
  };
  typedef struct serverState {
    uint32_t id;
    int32_t wd;
  }serverState;
  typedef struct serverPublish {
    uint32_t timestamp;
    bool logging;
    bool archvig;
    bool alarming;
  }serverPublish;
  typedef struct iArchivData {
    uint32_t timestamp;
    unsigned char elementIndex;
    unsigned char archivIndex;
    unsigned char trigger;
    float value;
  }iArchivData;
  typedef struct iAlarm {
    uint32_t timestamp;
    String   text;
    unsigned char prio;
    unsigned char elementIndex;
    unsigned char alarmIndex;
    unsigned char state;
  }iAlarm;
  typedef struct iError{
    uint16_t type;
    String data;
  }iError;
  
  /***************************************
   * Config
   ***************************************/
  enum configState{
    confInit,
    confReady,
    confGetReply,
    confGetActiv,
    confGetTimeout,
    confSetActiv,
    confSetTimeout
  };

  /***************************************
   * Subscribe
   ***************************************/
  enum subsState{
    init,
    requesting,
    subscribed,
    updated,
    fail
  };
  typedef struct subsElement {
    uint16_t      id;
    subsState     state;
    unsigned char localElementIndex;
    unsigned char localDatapointIndex;
    uint32_t      remoteNodeId;
    char          remoteElementName[JCA_IOT_ELEMENT_NAME_LEN];
    char          remoteDatapointName[JCA_IOT_ELEMENT_NAME_LEN];
    bool          onChange;
    bool          onCycle;
    float         hyst;
    uint32_t      time;
    int32_t wd;
  }subsElement;
  typedef struct publishElement {
    uint16_t      subsId;
    uint32_t      nodeId;
    unsigned char elementIndex;
    unsigned char datapointIndex;
    bool          onChange;
    bool          onCycle;
    float         hyst;
    uint32_t      time;
    int32_t      wd;
  }publishElement;
  typedef struct iSubscribeFail {
    uint16_t      subsId;
    uint32_t      nodeId;
    unsigned char error;
  }iSubscribeFail;
  typedef struct iSubscribeAck {
    uint16_t      subsId;
    uint32_t      nodeId;
  }iSubscribeAck;
  typedef struct iSubscribe {
    char          remoteElementName[JCA_IOT_ELEMENT_NAME_LEN];
    char          remoteDatapointName[JCA_IOT_ELEMENT_NAME_LEN];
    uint32_t      remoteNodeId;
    uint32_t      localNodeId;
    uint16_t      subsId;
    bool          onChange;
    bool          onCylce;
    float         hyst;
    uint32_t      time;
  }iSubscribe;
  typedef struct iPublish {
    uint16_t      subsId;
    uint32_t      nodeId;
    unsigned char type;   //Grund des Publish
    bool          valueBool;
    int32_t       valueInt;
    float         valueFloat;
    unsigned char qc;
  }iPublish;

  /***************************************
   * Handler
   ***************************************/
  typedef struct meshName {
    uint32_t      id;
    char          name[JCA_IOT_ELEMENT_NAME_LEN];
  }meshName;
}}}
#endif