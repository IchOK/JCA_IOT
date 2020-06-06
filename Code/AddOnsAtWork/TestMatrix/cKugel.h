#ifndef _CKUGEL_H
#define _CKUGEL_H

#include "cRoot.h"

class cKugel : public cRoot{
  public:
    cKugel(float solidRadius, float glowRadius, t_Vector sAreaMin, t_Vector sAreaMax, t_Vector sPos, t_Vector sMove, float grow, float accelerate, t_Color sRGB, float colorSpeed, uint8_t colorMode){
      _pos = sPos;
      _move = sMove;
      _areaMax = sAreaMax;
      _areaMin = sAreaMin;
      _grow = grow;
      _acc = accelerate;
      _rgb = sRGB;
      _cSpeed = colorSpeed;
      _solidRad = solidRadius;
      _glowRad = glowRadius;
      _mode = colorMode;
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
    float _solidRad;
    float _glowRad;
};

#endif
