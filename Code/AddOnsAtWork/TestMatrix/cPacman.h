#ifndef _CPACMAN_H
#define _CPACMAN_H

#define _CPACMAN_OPEN 1.0
#define _CPACMAN_WIDTH 0.9

#include "cRoot.h"

class cPacman : public cRoot{
  public:
    cPacman(float solidRadius, float glowRadius, t_Vector sAreaMin, t_Vector sAreaMax, t_Vector sPos, t_Vector sMove, float grow, float accelerate, t_Color sRGB){
      _pos = sPos;
      _move = sMove;
      _areaMax = sAreaMax;
      _areaMin = sAreaMin;
      _grow = grow;
      _acc = accelerate;
      _rgb = sRGB;
      _solidRad = solidRadius;
      _glowRad = glowRadius;
      _mouth = 0.0;
      _open = true;
    }
    t_KugelDim getDim(){
      t_KugelDim sDim;
      sDim.sPos = _pos;
      sDim.fRadius = _solidRad;
      return sDim;
    }
    void update(float fSeconds){
      //Mund
      if(_open){
        _mouth += fSeconds * _CPACMAN_OPEN * 3.0;
        if(_mouth > _CPACMAN_OPEN){
          _open = false;
        }
      }else{
        _mouth -= fSeconds * _CPACMAN_OPEN * 3.0;
        if(_mouth < 0){
          _open = true;
          _mouth = 0.0;
        }
      }
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
      float anKat;
      float Hypo;
      float alpha;
      if(_move.y > 0){
        anKat = pos.y - (_pos.y - _solidRad * _CPACMAN_WIDTH);
      }else{
        anKat = (_pos.y + _solidRad * _CPACMAN_WIDTH) - pos.y;
      }
      Hypo = sqrt(pow(anKat,2) + pow(pos.x - _pos.x,2));
      alpha = acos(anKat / Hypo);
      if(alpha > _mouth){
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
    }
  
  private:
    float _solidRad;
    float _glowRad;
    float _mouth;
    bool _open;
};

#endif
