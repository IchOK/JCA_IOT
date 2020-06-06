#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>

#include "cRoot.h"
#include "cKugel.h"
#include "cBall.h"
#include "cPacman.h"

#define LED_PIN D1

#define LEDS_X 8
#define LEDS_Y 32
#define LEDS 256
#define LEDS_DIF 10.0
#define BRIGHTNESS 255

//LED Matrix
uint32_t LEDcount = LEDS;
t_Vector aLEDs[LEDS];

//Grafikelemente definieren
t_Vector sAreaMin = {0.0, 0.0, 0.0};
t_Vector sAreaMax = {70.0, 310.0, 0.0};
// PAC-MAN
t_Vector sPosPac = {15.0, 0.0, 0.0};
t_Vector sMovePac = {0.0, -30.0, 0.0};
t_Color sRGBPac = {50.0, 50.0, 0.0};
cPacman Pacman(25.0, 5.0, sAreaMin, sAreaMax, sPosPac, sMovePac, 0.0, 0.0, sRGBPac);

// BÄLLE
t_Vector sMove = {-20.0, 5.0, 0.0};
t_Color sColor1 = {50.0, 0.0, 0.0};
cBall Ball_1(10.0, 15.0, 5.0, sAreaMin, sAreaMax, sMove, sColor1, 0.0, 0);
t_Color sColor2 = {0.0, 50.0, 0.0};
cBall Ball_2(10.0, 15.0, 5.0, sAreaMin, sAreaMax, sMove, sColor2, 0.0, 0);
t_Color sColor3 = {0.0, 0.0, 50.0};
cBall Ball_3(10.0, 15.0, 5.0, sAreaMin, sAreaMax, sMove, sColor3, 0.0, 0);

// KUGELN
// nicht verwendet
/*
float solidRadius = 20.0;
float glowRadius = 5.0;
t_Vector sPos = {20.0, 30.0, 0.0};
t_Vector sMove = {10.0, 27.0, 0};
float grow = 0.0;
float accelerate = 0.0;
t_Color sRGB = {0.0, 0.0, 50.0};
float colorSpeed = 0.0;
uint8_t colorMode = 0;
cKugel Kugel1(solidRadius, glowRadius, sAreaMin, sAreaMax, sPos, sMove, grow, accelerate, sRGB, colorSpeed, colorMode);
t_Vector sPos2 = {30.0, 300.0, 0.0};
t_Vector sMove2 = {0, -20.0, 10.0};
t_Color sRGB2 = {0.0, 50.0, 0.0};
cKugel Kugel2(10.0, 15.0, sAreaMin, sAreaMax, sPos2, sMove2, 0.0, 0.0, sRGB2, 0.0, 0);
*/

uint32_t oldMillis;
Adafruit_NeoPixel strip(LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

void setup() {
  Serial.begin(115200);
  Serial.println("Setup start");
  float alpha;
  uint8_t x, y;
  uint32_t n;
  for(y=0;y<LEDS_Y;y++){
    for(x=0;x<LEDS_X;x++){
      n = x + y * LEDS_X;
      if(y%2 == 0){
        aLEDs[n].x = (float)x * LEDS_DIF;
      }else{
        aLEDs[n].x = (LEDS_X - (float)x - 1.0) * LEDS_DIF;
      }
      aLEDs[n].y = (float)y * LEDS_DIF;
      aLEDs[n].z = 0.0;
    }
  }

  
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(BRIGHTNESS); // Set BRIGHTNESS to about 1/5 (max = 255)
  Serial.println("Setup done");
  oldMillis = millis();
}

void loop() {
  t_KugelDim dimPacMan;
  static uint32_t n;
  float r;
  float g;
  float b;
  uint8_t uR;
  uint8_t uG;
  uint8_t uB;
  uint32_t i;
  uint32_t newMillis = millis();
  float diffMillis = (float)(newMillis - oldMillis) / 1000.0;
  oldMillis = newMillis;
//  Kugel1.update(diffMillis);
//  Kugel2.update(diffMillis);
  Ball_1.update(diffMillis);
  Ball_2.update(diffMillis);
  Ball_3.update(diffMillis);
  Pacman.update(diffMillis);
  dimPacMan = Pacman.getDim();
  Ball_1.checkCrash(dimPacMan);
  Ball_2.checkCrash(dimPacMan);
  Ball_3.checkCrash(dimPacMan);
  strip.clear();
  for(i=0;i<LEDcount;i++){
    r = 0;
    g = 0;
    b = 0;
//    Kugel1.addColorRGB(aLEDs[i], r, g, b);
//    Kugel2.addColorRGB(aLEDs[i], r, g, b);
    Ball_1.addColorRGB(aLEDs[i], r, g, b);
    Ball_2.addColorRGB(aLEDs[i], r, g, b);
    Ball_3.addColorRGB(aLEDs[i], r, g, b);
    Pacman.addColorRGB(aLEDs[i], r, g, b);
    //Summe Rot
    if(r > 255){
      uR = 255;
    }else{
      uR = (uint8_t)r;
    }
    //Summe Grün
    if(g > 255){
      uG = 255;
    }else{
      uG = (uint8_t)g;
    }
    //Summe Blau
    if(b > 255){
      uB = 255;
    }else{
      uB = (uint8_t)b;
    }
    strip.setPixelColor(i, uR, uG, uB);
  }
  strip.show();
}
