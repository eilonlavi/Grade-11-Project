#include "stdafx.h"
struct GameObject{
	int od;
	int id;
	int	x;
	int y;
	double rot;
	int width;
	int height;
	GameObject(int _od,int _id,int _x,int _y,double _rot,int _width,int _height);
	GameObject();
};