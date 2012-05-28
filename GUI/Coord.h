#ifndef Coord_H
#define Coord_H

struct Coord
{

};

struct Coord2D : public Coord
{
  float x,y;
  Coord2D() : x(0), y(0){}
};

struct Coord3D : public Coord
{
  float x,y,z;
  Coord3D() : x(0), y(0), z(0){}
};

#endif
