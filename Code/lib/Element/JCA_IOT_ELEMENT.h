#ifndef _JCA_IOT_ELEMENT_H
#define _JCA_IOT_ELEMENT_H

//Only include if OneWire is activated
#ifdef JCA_IOT_ELEMENT_ONEWIRE
  #include "Element/JCA_IOT_ELEMENT_HW_OneWire.hpp"
  #include "Element/JCA_IOT_ELEMENT_DS18B20.hpp"
#endif

#include <Element/JCA_IOT_ELEMENT_Root.hpp>
#include "Element/JCA_IOT_ELEMENT_Handler.hpp"
#include "Element/JCA_IOT_ELEMENT_DI.hpp"
#include "Element/JCA_IOT_ELEMENT_DO.hpp"
#include "Element/JCA_IOT_ELEMENT_AI.hpp"
#include "Element/JCA_IOT_ELEMENT_SinGen.hpp"
#include "Element/JCA_IOT_ELEMENT_OR.hpp"
#include "Element/JCA_IOT_ELEMENT_CLOCKSPAN.hpp"
#include "Element/JCA_IOT_ELEMENT_CLOCKPULSE.hpp"
#include "Element/JCA_IOT_ELEMENT_HeatCool.hpp"

#endif
