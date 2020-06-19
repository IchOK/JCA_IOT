/**********************************************
 * Definitionen:
 * Info: 	Definiert allgemeine Konstanten f�r
 *        das interne Mesh-Protokoll
 * Version:
 * 	V1.0.0	Erstellt	12.06.2020	JCA
 **********************************************/

#ifndef _JCA_IOT_MESH_DEFINE_H
#define _JCA_IOT_MESH_DEFINE_H

#define JCA_IOT_MESH_SUBSCRIBE 10
#define JCA_IOT_MESH_SUBSCRIBE_ACK 11
#define JCA_IOT_MESH_SUBSCRIBE_PUBLISH 12
#define JCA_IOT_MESH_SUBSCRIBE_FAIL 19
#define JCA_IOT_MESH_PUSH 20
#define JCA_IOT_MESH_DATA_REQUEST 30
#define JCA_IOT_MESH_DATA_REPLY 31
#define JCA_IOT_MESH_GETCONF_REQUEST 40
#define JCA_IOT_MESH_GETCONF_REPLY 41
#define JCA_IOT_MESH_GETCONF_DATAREQUEST 42
#define JCA_IOT_MESH_GETCONF_DATA 43
#define JCA_IOT_MESH_SETCONF_ANNOUNCE 50
#define JCA_IOT_MESH_SETCONF_DATAREQUEST 51
#define JCA_IOT_MESH_SETCONF_DATA 52
#define JCA_IOT_MESH_SETCONF_DONE 53
#define JCA_IOT_MESH_SRV_PUBLISH 90
#define JCA_IOT_MESH_SRV_REQUEST 91
#define JCA_IOT_MESH_SRV_ARCHIVDATA 92
#define JCA_IOT_MESH_SRV_ALARM 93
#define JCA_IOT_MESH_SRV_ALARMACK 94
#define JCA_IOT_MESH_SRV_FAILLOG 99
#define JCA_IOT_MESH_SRV_TIMEWD 900000
#define JCA_IOT_MESH_SRV_TIMEREQ 6000 //0
#define JCA_IOT_MESH_SRV_TIMERANGE 10000
#endif