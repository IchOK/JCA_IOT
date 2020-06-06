#ifndef _CBALL_H
#define _CBALL_H

#include "cRoot.h"

class cBall : public cRoot{
  public:
    cBall(float minRadius, float maxRadius, float glowRadius, t_Vector sAreaMin, t_Vector sAreaMax, t_Vector sMove, t_Color sRGB, float colorSpeed, uint8_t colorMode){
      _minRad = minRadius;
      _maxRad = maxRadius;
      _moveInit = sMove;
      _areaMax = sAreaMax;
      _areaMin = sAreaMin;
      _grow = 0.0;
      _acc = 0.0;
      _rgb = sRGB;
      _cSpeed = colorSpeed;
      _glowRad = glowRadius;
      _mode = colorMode;
      reInit();
    }
    void checkCrash(t_KugelDim sDim){
      float delta = getDistance(sDim.sPos);
      if(delta < sDim.fRadius){
        reInit();
      }
    }
    void update(float fSeconds){
      //Bewegung
      _pos.x += _move.x * fSeconds;
      _pos.y += _move.y * fSeconds;
      _pos.z += _move.z * fSeconds;
      //Kollision X
      if(_move.x > 0.0){
        if((_pos.x + _solidRad) >= _areaMax.x){
          _move.x *= -1.0;
        }
      }
      if(_move.x < 0.0){
        if((_pos.x - _solidRad) <= _areaMin.x){
          _move.x *= -1.0;
        }
      }
      //Kollision Y
      if(_move.y > 0.0){
        if((_pos.y + _solidRad) >= _areaMax.y){
          _move.y *= -1.0;
        }
      }
      if(_move.y < 0.0){
        if((_pos.y - _solidRad) <= _areaMin.y){
          _move.y *= -1.0;
        }
      }
      //Kollision Z
      if(_move.z > 0.0){
        if((_pos.z + _solidRad) >= _areaMax.z){
          _move.z *= -1.0;
        }
      }
      if(_move.z < 0.0){
        if((_pos.z - _solidRad) <= _areaMin.z){
          _move.z *= -1.0;
        }
      }
      //Beschleunigung
      _move.x += _acc * fSeconds;
      _move.y += _acc * fSeconds;
      _move.z += _acc * fSeconds;
      //Wachstum
      _solidRad += _grow * fSeconds;
      if(_solidRad < 0.0){
        _solidRad = 0.0;
      }
      _glowRad += _grow * fSeconds;
      if(_glowRad < 0.0){
        _glowRad = 0.0;
      }
    }
    void addColorRGB(t_Vector pos, float &r, float &g, float &b){
      float delta = getDistance(pos);
      if(delta <= _solidRad){
        r = r + _rgb.r;
        g = g + _rgb.g;
        b = b + _rgb.b;
      }else if(delta - _solidRad < _glowRad){
        float factor = (_glowRad - (delta - _solidRad)) / _glowRad;
        r = r + _rgb.r * factor;
        g = g + _rgb.g * factor;
        b = b + _rgb.b * factor;
      }
    }
  
  private:
    float _minRad;
    float _maxRad;
    float _solidRad;
    float _glowRad;
    t_Vector _moveInit;
    void reInit(){
      float fRand;
      //Startposition
      fRand = 0.01 * (float)random(0,101);
      _pos.y = (_areaMax.y - _areaMin.y) * fRand + _areaMin.y;
      fRand = 0.01 * (float)random(0,101);
      _pos.z = (_areaMax.z - _areaMin.z) * fRand + _areaMin.z;
      fRand = 0.01 * (float)random(0,101);
      _pos.x = (_areaMax.x - _areaMin.x) * fRand + _areaMax.x;
      //Bewegung
      fRand = 0.01 * (float)random(-100,101);
      _move.y = _moveInit.y * fRand;
      fRand = 0.01 * (float)random(-100,101);
      _move.z = _moveInit.z * fRand;
      fRand = 0.01 * (float)random(50,101);
      _move.x = _moveInit.x * fRand;
      //Durchmesser
      fRand = 0.01 * (float)random(0,101);
      _solidRad = (_maxRad - _minRad) * fRand + _minRad;
    }
};

#endif
