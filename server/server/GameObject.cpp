#include "stdafx.h"
#include "GameObject.h"

GameObject::GameObject(int _od, int _id, int _x, int _y, double _rot, int _target){
	od=_od;
	id=_id;
	x=_x;
	y=_y;
	rot=_rot;
	target=_target;
	vx=0;
	vy=0;
}
GameObject::GameObject(int _od, int _id, int _x, int _y, double _rot,int _vx,int _vy){
	od=_od;
	id=_id;
	x=_x;
	y=_y;
	rot=_rot;
	vx=_vx;
	vy=_vy;
}

GameObject::GameObject(){
	GameObject(0,0,0,0,0);
}