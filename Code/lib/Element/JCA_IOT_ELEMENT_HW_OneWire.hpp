/**********************************************
 * Class:  JCA_IOT_ELEMENT_HW_ONEWIRE
 * Info:   Abgeleitete Klasse von JCA_IOT_ELEMENT
 *       Liest die Temperatur von einem DS18B20 Sensor ein.
 * Version:
 *   V1.0.0  Erstellt   24.10.2020  JCA
 *   -add Properties
 *     -- Raw, Resend, LastUpdate
 *     -- Type, Addr, Watchdog, UpdateTime
 *     -- Value
 *   -add Methoden
 *     -- cDS18B20
 *     -- update
 **********************************************/

#ifndef _JCA_IOT_ELEMENT_HW_ONEWIRE_H
#define _JCA_IOT_ELEMENT_HW_ONEWIRE_H

#include "JCA_IOT_Debug.h"

//Include extrenal
#include <OneWire.h>

OneWire JCA_OneWire;

#endif