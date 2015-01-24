// server.cpp

/* Link with library file wsock32.lib */

#include "stdafx.h"
#include <iostream>
#include <string>
#include <stdlib.h>
#include <winsock.h>
#include <ctime>
#include <math.h>
#include "GameObject.h"
using namespace std;

#define BUFFER_SIZE 4096
#define SIZE_MR	18
#define SIZE_C 10
#define SIZE_D 6
#define PI 3.14159
#define GAMETIME 100

void usage();
//These set the messages to be sent to the 'current' client (when running)
bool setMessage(char* buf, char command, int od, int id, int x, int y, int rot = 0);//returns true on success
bool setMessage(char* buf, char command, int id);									//returns true on success
int addObj(int od, int x, int y, double rot, int target=0);
int addObj(int od, int x, int y, double rot,int _vx,int _vy);
void delObj(int id);
void moveObj(int id, int x,int y,double rot);

//List of info for each user
int POVID[6]={0,0,0,0,0,0};				//The list of id's
int nBullets[4]={0,0,0,0};				//The number of bullets
char press[4]={NULL,NULL,NULL,NULL};	//The buttons pressed
int points[5]={0,0,0,0,0};				//The # of points won
bool alive[5]={true,true,true,true,true};//The list of alive users

//Map generation
int random(int min=1, int max=100);	//Returns a pseudorandom number from min to max, both inclusive
int O=0;							//The # of objects
int nId=0;							//The # of id's given out
GameObject *go = new GameObject[O];	//The list of objects to put on screen
int nUsers = 0;						//The # of users minus 1, but nUsers=5 means the list is full

//Message info
char message_stack[5][100][SIZE_MR];//The messages to send for each client
int message_stack_size[5]={0,0,0,0,0};//The amount of messages left to send for each client

int main(int argc, char* argv[]){
	WSADATA w;							/* Used to open windows connection */
	unsigned short port_number;			/* Port number to use */
	int a1, a2, a3, a4;					/* Components of address in xxx.xxx.xxx.xxx form */
	int client_length;					/* Length of client struct */
	SOCKET sd;							/* Socket descriptor of server */
	struct sockaddr_in server;			/* Information about the server */
	struct sockaddr_in client;			/* Information about the TEMP client */
	char buffer[BUFFER_SIZE];			/* Where to store received data */
	struct hostent *hp;					/* Information about this computer */
	char host_name[256];				/* Name of the server */

	struct sockaddr_in users[5];		//List of user sock addresses

	//Initializing message stack
	for (int a=0;a<5;a++) for (int b=0;b<100;b++) for (int c=0;c<SIZE_MR;c++)message_stack[a][b][c]=NULL;

	//------------------------BEGIN NETWORK SETUP----------------------------
	for (int i=1;i<argc;i++)
		cout << "arg"<<i<<":"<<argv[i]<<endl;
	/* Interpret command line */
	if (argc == 2){
		/* Use local address */
		if (sscanf_s((char *)argv[1], "%u", &port_number) != 1)
			usage();
	}
	else if (argc == 3){
		/* Copy address */
		if (sscanf_s((char *)argv[1], "%d.%d.%d.%d", &a1, &a2, &a3, &a4) != 4)
			usage();
		if (sscanf_s((char *)argv[2], "%u", &port_number) != 1)
			usage();
	}
	else
		usage();

	/* Open windows connection */
	if (WSAStartup(0x0101, &w) != 0){
		fprintf(stderr, "Could not open Windows connection.\n");
		exit(0);
	}
	/* Open a datagram socket */
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sd == INVALID_SOCKET){
		fprintf(stderr, "Could not create socket.\n");
		WSACleanup();
		exit(0);
	}
	/* Clear out server struct */
	memset((void *)&server, '\0', sizeof(struct sockaddr_in));
	/* Set family and port */
	server.sin_family = AF_INET;
	server.sin_port = htons(port_number);
	/* Set address automatically if desired */
	if (argc == 2){
		/* Get host name of this computer */
		gethostname(host_name, sizeof(host_name));
		hp = gethostbyname(host_name);

		/* Check for NULL pointer */
		if (hp == NULL){
			fprintf(stderr, "Could not get host name.\n");
			closesocket(sd);
			WSACleanup();
			exit(0);
		}
		/* Assign the address */
		server.sin_addr.S_un.S_un_b.s_b1 = hp->h_addr_list[0][0];
		server.sin_addr.S_un.S_un_b.s_b2 = hp->h_addr_list[0][1];
		server.sin_addr.S_un.S_un_b.s_b3 = hp->h_addr_list[0][2];
		server.sin_addr.S_un.S_un_b.s_b4 = hp->h_addr_list[0][3];
	}
	/* Otherwise assign it manually */
	else{
		server.sin_addr.S_un.S_un_b.s_b1 = (unsigned char)a1;
		server.sin_addr.S_un.S_un_b.s_b2 = (unsigned char)a2;
		server.sin_addr.S_un.S_un_b.s_b3 = (unsigned char)a3;
		server.sin_addr.S_un.S_un_b.s_b4 = (unsigned char)a4;
	}

	/* Bind address to socket */
	if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) == -1){
		fprintf(stderr, "Could not bind name to socket.\n");
		closesocket(sd);
		WSACleanup();
		exit(0);
	}

	/* Print out server information */
	printf("Server running on %u.%u.%u.%u\n", (unsigned char)server.sin_addr.S_un.S_un_b.s_b1,
											  (unsigned char)server.sin_addr.S_un.S_un_b.s_b2,
											  (unsigned char)server.sin_addr.S_un.S_un_b.s_b3,
											  (unsigned char)server.sin_addr.S_un.S_un_b.s_b4);
	printf("Press CTRL + C to quit\n");
	//--------------------------------END NETWORK SETUP-----------------------------------

	//Generate map and create stack to send
	for (int i=0;i<50;i++){
		addObj(1,random(-1000,1000),random(-1000,1000),random(0,360));
	}

	/* Loop and get data from clients */
	while (true){

		//The timer to print the current scores
		static int printpoints = 0;
		if(printpoints==500){
			cout<<"Points:"<<endl;
			cout<<" P0 "<<points[0]<<endl;
			cout<<" P1 "<<points[1]<<endl;
			cout<<" P2 "<<points[2]<<endl;
			cout<<" P3 "<<points[3]<<endl;
			cout<<" P4 "<<points[4]<<endl;
			printpoints=0;
		}
		else
			printpoints++;

		client_length = (int)sizeof(struct sockaddr_in);
		//RECEIVE message
		if (recvfrom(sd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client, &client_length) < 0){
			fprintf(stderr, "Could not receive datagram.\n");
			closesocket(sd);
			WSACleanup();
			exit(0);
		}

		string message=buffer;
		//cout<<"|"<<message<<"|"<<endl;
		if (message.substr(0,5)=="space" && alive[message.at(6)-'0']){
			int td=message.at(6)-'0';
			if (nBullets[td]<10){
				double speed=30;
				double s=sin(go[POVID[td]].rot/180*PI);
				s*=speed;
				double c=cos(go[POVID[td]].rot/180*PI);
				c*=speed;
				addObj(2,go[POVID[td]].x,go[POVID[td]].y,go[POVID[td]].rot,s,-c);
				nBullets[td]++;
			}
		}
		if (message.substr(0,4)=="left" && alive[message.at(6)-'0']){
			int td=message.at(6)-'0';
			moveObj(POVID[td],0,0,-8);
		}
		if (message.substr(0,5)=="right" && alive[message.at(6)-'0']){
			int td=message.at(6)-'0';
			moveObj(POVID[td],0,0,8);
		}
		if (message.substr(0,4)=="forw" && alive[message.at(6)-'0']){
			int td=message.at(6)-'0';
			press[td]='f';
		}
		if (message.substr(0,4)=="down" && alive[message.at(6)-'0']){
			int td=message.at(6)-'0';
			press[td]='b';
		}
		if (message.substr(0,4)=="stop" && alive[message.at(6)-'0']){
			int td=message.at(6)-'0';
			press[td]=NULL;
		}
		
		//---------------UPDATE------------------
		if (message.substr(0,6)=="update"){
			int curUser = message.at(6)-'0';
			if (alive[curUser]){
				//1 in 30 chance of creating zombie;
				if (random(0,29)==7){// lucky seven
					//cout<<"Zombie created."<<endl;
					addObj(3,random(-1000,1000),random(-1000,1000),random(0,360), random(0,nUsers-1));
				}
				//doing actions for all objects
				for (int o=0;o<O;o++){
					if(go[o].od==0){//person
						for (int o2=0;o2<O;o2++){
							if (go[o2].od==3){//zombie
								if (alive[curUser] && (go[o].x-go[o2].x)*(go[o].x-go[o2].x)+(go[o].y-go[o2].y)*(go[o].y-go[o2].y)<500){//HIT!
									int deaduser=0;
									for (int i=0;i<nUsers;i++) if (go[o].id=POVID[i]) deaduser=i;
									addObj(4,go[o].x,go[o].y,go[POVID[deaduser]].rot);
									alive[deaduser]=false;
									press[deaduser]=NULL;
									break;
								}
							}
						}
					}
					else if (go[o].od==2){//Bullet
						if (go[o].vx!=0 || go[o].vy!=0){
							moveObj(go[o].id,go[o].vx,go[o].vy,0);
						}
						else delObj(go[o].id);
						if (go[o].x>1000 || go[o].y>1000 || go[o].x<-1000 || go[o].y<-1000) {
							//Deleting bullets
							delObj(go[o].id);
							nBullets[curUser]--;
						}
						else {
							for (int o2=0;o2<O;o2++){
								if (go[o2].od==3){//Zombie
									if ((go[o].x-go[o2].x)*(go[o].x-go[o2].x)+(go[o].y-go[o2].y)*(go[o].y-go[o2].y)<700){//HIT!
										int id1=go[o].id;
										int id2=go[o2].id;
										delObj(id1);
										delObj(id2);
										points[curUser]++;
										break;
									}
								}
							}
						}
					}
					else if(go[o].od==3){//Zombie move
						int speed=6/nUsers;
						go[o].vx = (go[o].x>go[POVID[go[o].target]].x ? -speed : speed);
						go[o].vy = (go[o].y>go[POVID[go[o].target]].y ? -speed : speed);
						moveObj(go[o].id,go[o].vx,go[o].vy,0);
					}
				}
				//movement
				if (press!=NULL){
					if (press[curUser]=='b'){//backwards
						double dist=12;
						double s=sin(go[POVID[curUser]].rot/180*PI);
						s*=dist;
						double c=cos(go[POVID[curUser]].rot/180*PI);
						c*=dist;
						int __id=POVID[curUser];
						moveObj(__id,-s,c,0);
					}
					else if (press[curUser]=='f'){//forwards
						double dist=12;
						double s=sin(go[POVID[curUser]].rot/180*PI);
						s*=dist;
						double c=cos(go[POVID[curUser]].rot/180*PI);
						c*=dist;
						int __id=POVID[curUser];
						moveObj(__id,s,-c,0);
					}
					
				}
			}
			//telling the client the amount of messages to send
			char buf[SIZE_MR];
			setMessage(buf,'n',message_stack_size[curUser]);
			sendto(sd, buf, 5, 0, (struct sockaddr *)&users[curUser], client_length);
			for (int i=0;i<message_stack_size[curUser];i++){
				sendto(sd, message_stack[curUser][i],SIZE_MR, 0, (struct sockaddr *)&users[curUser], client_length);
				for (int c=0;c<SIZE_MR;c++)
					message_stack[curUser][i][c]=NULL;
			}
			message_stack_size[curUser]=0;
		}
		//----------------NEW USER-----------------
		else if (message=="new"){
			if(nUsers<5){
				cout<<"New player.("<<nUsers<<")"<<endl;
				users[nUsers] = client;
				char to_send[2];
				to_send[0] = (char)(nUsers+'0');
				to_send[1] = '\0';
				if(sendto(sd, to_send, 2, 0, (struct sockaddr *)&client, client_length)<0){
					fprintf(stderr, "Error sending datagram to new user.\nThe program will now exit.");
					closesocket(sd);
					WSACleanup();
					exit(0);
				}
				int id=addObj(0,random(-500,500),random(-500,500),90);
				cout << "WTF" << id << endl;
				setMessage(message_stack[nUsers][message_stack_size[nUsers]],'p',id);
				message_stack_size[nUsers]++;
				POVID[nUsers]=id;
				if(nUsers==4)
					cout<<"The user limit has been reached."<<endl;
				nUsers++;
			}
			else{
				cout<<"Another user has attempted to connect."<<endl;
			}

		}
		//---------------DELETE USER----------------
		else if(message.substr(0,6)=="delete"){//needs to be updated
			int curUser = message.at(6)-'0';
			if(curUser!=4 || curUser!=nUsers)
				for(int i=curUser; i<nUsers; i++)
					users[i] = users[i+1];
			nUsers--;
			cout<<"Player "<<curUser<<" has left."<<endl;
		}
	}
	
	closesocket(sd);
	WSACleanup();
	return 0;
}

void usage(){
	fprintf(stderr, "timeserv [server_address] port\n");
	exit(0);
}

bool setMessage(char *buf, char command, int od, int id, int x, int y, int rot){
	//MOVE/ROTATE
	 if(command=='m'){
		int to_send[] = {id,x,y,rot};
		buf[0] = 'm';
		for(int i=0; i<4; i++)
			memcpy(&buf[(i*4)+1], &to_send[i], 4);
		buf[17] = '\0';
		return true;
	}

	//CREATE
	else if(command=='c'){
		int to_send[] = {od,id};
		buf[0] = 'c';
		for(int i=0; i<2; i++)
			memcpy(&buf[(i*4)+1], &to_send[i], 4);
		buf[9] = '\0';
		return true;
	}

	//DELETE
	else if(command=='d'){
		int to_send[] = {id};
		buf[0] = 'd';
		memcpy(&buf[1], &to_send[0], 4);
		buf[5] = '\0';
		return true;
	}

	//RETURN failure
	return false;
}

bool setMessage(char* buf, char command, int id){
	//DELETE
	if(command=='d'){
		int to_send[] = {id};
		buf[0] = 'd';
		memcpy(&buf[1], &to_send[0], 4);
		buf[5] = '\0';
		return true;
	}
	//NUMBER of messages
	if(command=='n'){
		int to_send[] = {id};
		memcpy(&buf[0], &to_send[0], 4);
		buf[4] = '\0';
		return true;
	}
	//P.O.V.
	if(command=='p'){
		int to_send[] = {id};
		buf[0] = 'p';
		memcpy(&buf[1], &to_send[0], 4);
		buf[5] = '\0';
		//cout<< "here";
		return true;
	}
	//RETURN failure
	return false;
}

int random(int min, int max){
	srand((unsigned int)time(0)+rand());
	return ((rand()%(max-min+1))+min);
}

int addObj(int od, int x, int y, double rot, int target){
	GameObject g(od,nId,x,y,rot,target);
	GameObject *temp = new GameObject[O+1];
	for(int i=0; i<O; i++)
		temp[i] = go[i];
	temp[O] = g;
	delete[] go;
	go=temp;

	for (int i=0;i<4;i++){
		setMessage(message_stack[i][message_stack_size[i]%100],'c',od,nId,x,y,(int)(rot*1000));
		message_stack_size[i]++;
		setMessage(message_stack[i][message_stack_size[i]%100],'m',temp[O].od,temp[O].id,temp[O].x,temp[O].y,(int)(temp[O].rot*1000));
		message_stack_size[i]++;
	}
	O++;
	nId++;
	return go[O-1].id;
}
int addObj(int od, int x, int y, double rot,int _vx,int _vy){
	GameObject g(od,nId,x,y,rot);
	g.vx=_vx;
	g.vy=_vy;
	GameObject *temp = new GameObject[O+1];
	for(int i=0; i<O; i++)
		temp[i] = go[i];
	temp[O] = g;
	
	delete[] go;
	go=temp;

	for (int i=0;i<4;i++){
		setMessage(message_stack[i][message_stack_size[i]%100],'c',od,nId,x,y,(int)(rot*1000));
		message_stack_size[i]++;
		setMessage(message_stack[i][message_stack_size[i]%100],'m',temp[O].od,temp[O].id,temp[O].x,temp[O].y,(int)(temp[O].rot*1000));
		message_stack_size[i]++;
	}
	O++;
	nId++;
	return go[O-1].id;
}
void delObj(int id){
	if(O==0) return;

	int count = 0;
	for(int i=0; i<O; i++)
		if(go[i].id==id)
			count++;
	if (count==0) return;
	GameObject *temp = new GameObject[O-1];
	int i1=0, i2=0;
	while(i1<O && i2<O-1){
		if(go[i1].id!=id){
			temp[i2]=go[i1];
			i2++;
		}
		i1++;
	}
	delete[] go;
	for (int i=0;i<4;i++){
		setMessage(message_stack[i][message_stack_size[i]%100],'d',id);
		message_stack_size[i]++;
	}

	go=temp;
	O--;
}

void moveObj(int id, int x,int y,double rot){
	for (int o=0;o<O;o++){
		if (go[o].id==id){
			go[o].x+=x;
			go[o].y+=y;
			go[o].rot+=rot;
			if (go[o].od==0){
				if (go[o].rot>360) go[o].rot-=360;
				if (go[o].rot<0) go[o].rot+=360;
				if (go[o].x<-1000) go[o].x=-1000;
				if (go[o].x>1000) go[o].x=1000;
				if (go[o].y<-1000) go[o].y=-1000;
				if (go[o].y>1000) go[o].y=1000;
			}
			for (int i=0;i<4;i++){
				setMessage(message_stack[i][message_stack_size[i]%100],'m',go[o].od,go[o].id,go[o].x,go[o].y,go[o].rot*1000);
				message_stack_size[i]++;
			}
			break;
		}
	}
}
