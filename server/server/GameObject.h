
class GameObject{

public:
	GameObject(int _od, int _id, int _x, int _y, double _rot, int _target = 0);
	GameObject(int _od, int _id, int _x, int _y, double _rot,int _vx,int _vy);
	GameObject();
	int od;
	int id;
	int x;
	int y;
	double rot;
	int vx;
	int vy;
	int target;//only used for zombies, who they chase
};