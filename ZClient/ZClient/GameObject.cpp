#include "stdafx.h"
#include "GameObject.h"
GameObject::GameObject(int _od=0,int _id=0,int _x=0,int _y=0,double _rot=0,int _width=0,int _height=0){
	od=_od;
	id=_id;
	x=_x;
	y=_y;
	rot=_rot;
	width=_width;
	height=_height;
}
GameObject::GameObject(){
	GameObject(0,0,0,0,0,0,0);
}