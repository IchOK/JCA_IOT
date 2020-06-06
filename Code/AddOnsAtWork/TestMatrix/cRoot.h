#ifndef _CROOT_H
#define _CROOT_H

typedef struct{
  float x, y, z;
}t_Vector;

typedef struct{
  t_Vector sPos;
  float fRadius;
}t_KugelDim;

typedef struct{
  float r, g, b;
}t_Color;

class cRoot{
  // x y z Werte in um
  public:
    cRoot(){;}
    virtual void update(float fSeconds){;}
    float getDistance(t_Vector pos){
      return sqrt(pow(pos.x - _pos.x,2) + pow(pos.y - _pos.y,2) + pow(pos.z - _pos.z,2));
    }
  protected:
    t_Vector _pos;
    t_Vector _move;
    t_Vector _areaMax;
    t_Vector _areaMin;
    float _grow;
    float _acc;
    t_Color _rgb;
    float _cSpeed;
    uint8_t _mode;
};

#endif
