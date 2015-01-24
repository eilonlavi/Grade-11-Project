#include "stdafx.h"
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include "GameObject.h"
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <time.h>
#include <winsock.h>
#include <math.h>

#define TIMER_STEP 1
#define STEPTIME 100
#define PI 3.14159
int O=0;		//The # of objects.
int POV=-1;		//The point of view for this client.
int wid=950;	//Width and
int hei=680;	//height of screen.
GameObject *obj = new GameObject[O];//List of objects.
char user[2];						//User id number.
char* IP="192.168.1.187";
/**ALL IMAGES**/
WCHAR* imgAdr[]={L"person.gif",L"tree.gif",L"bullet.png",L"Zomb.png",L"Dead.gif"};
WCHAR* bg(L"bg.jpg");
/**FUNCTIONS**/
void KeyCheck();
void addObj(int od, int id, int x, int y, double rot);
void delObj(int id);
int getNum(char c1,char c2,char c3,char c4);
/**NETWORK STUFF**/
	WSADATA w;								/* Used to open Windows connection */
	unsigned short port_number;				/* The port number to use */
	SOCKET sd;								/* The socket descriptor */
	int server_length;						/* Length of server struct */
	std::string send_buffer = "GET TIME\r\n";/* Data to send */
	//time_t current_time;					/* Time received */
	struct hostent *hp;						/* Information about the server */
	struct sockaddr_in server;				/* Information about the server */
	struct sockaddr_in client;				/* Information about the client */
	int a1, a2, a3, a4;						/* Server address components in xxx.xxx.xxx.xxx form */
	int b1, b2, b3, b4;						/* Client address components in xxx.xxx.xxx.xxx form */
	char host_name[256];					/* Host name of this computer */
	std::string username;
	bool pressed=false;
/**END**/

VOID OnPaint(HDC hdc){
   Graphics graphics(hdc);
   Pen      pen(Color(0, 0, 255, 255));
   //Draw bg
   Image ibg(bg);
   graphics.TranslateTransform(wid/2,hei/2);
   graphics.DrawImage(&ibg,-wid/2,-hei/2);
   graphics.ResetTransform();
   //If point of view is not set
   if (POV==-1){
	   for (int o=0;o<O;o++){
		   Image img(imgAdr[obj[o].od]);
		   graphics.TranslateTransform(obj[o].x,obj[o].y);
		   if (obj[o].rot!=0)
		   graphics.RotateTransform(obj[o].rot);
		   
		   graphics.DrawImage(&img,-obj[o].width/2,-obj[o].height/2);
		   graphics.ResetTransform();
	   }
   }
   //Otherwise...
   else {
	   GameObject *p;
	   for (int i=0;i<O;i++){
		   if (obj[i].id==POV){
			   p=&obj[i];
				break;
		   }
	   }
	   for (int o=0;o<O;o++){
			if (((obj[o].x-p->x)*(obj[o].x-p->x)+(obj[o].y-p->y)*(obj[o].y-p->y))>((wid*wid+hei*hei)/4)) continue;
			Image img(imgAdr[obj[o].od]);
			graphics.TranslateTransform(wid/2,hei/2);
			graphics.RotateTransform(-p->rot);
			graphics.TranslateTransform(obj[o].x-p->x,obj[o].y-p->y);

			graphics.RotateTransform(obj[o].rot);

			graphics.DrawImage(&img,-obj[o].width/2,-obj[o].height/2);
			graphics.ResetTransform();
	   }
   }
}

VOID Step(){
	//GET SERVER PACKET
	char send[] = {'u','p','d','a','t','e',user[0],user[1],'\0'};
	sendto(sd,send,8,0,(struct sockaddr *) &server,server_length);
	server_length=(int )sizeof(struct sockaddr_in);
	int updates=1;
	char mess[5];
	if (recvfrom(sd, mess,5,0,(struct sockaddr *) &server, &server_length)>0){ //how many updates?
		updates=getNum(mess[0],mess[1],mess[2],mess[3]);
	}
	for (int i=0;i<updates;i++){
		char message[18];
		if (recvfrom(sd, message ,18,0,(struct sockaddr *) &server, &server_length)>0){
			if (message[0]=='m'){ // move
				int id=getNum(message[1],message[2],message[3],message[4]);
				int x=getNum(message[5],message[6],message[7],message[8]);
				int y=getNum(message[9],message[10],message[11],message[12]);
				double rot=getNum(message[13],message[14],message[15],message[16])/1000;
				for (int o=0;o<O;o++){
					if (obj[o].id==id){
						obj[o].x=x;
						obj[o].y=y;
						obj[o].rot=rot;
						break;
					}
				}
			}
			else if(message[0]=='c'){ // create
				int od=getNum(message[1],message[2],message[3],message[4]);
				int id=getNum(message[5],message[6],message[7],message[8]);
				addObj(od,id,0,0,0);
			}
			else if(message[0]=='d'){ // delete
				int id=getNum(message[1],message[2],message[3],message[4]);
				delObj(id);
			}
			else if(message[0]=='p'){// change POV
				POV=getNum(message[1],message[2],message[3],message[4]);
			}
		}
	}
	KeyCheck();
}
VOID KeyCheck(){
	if (GetAsyncKeyState(VK_LEFT)){
		char send[] = {'l','e','f','t','-','-',user[0],user[1],'\0'};
		sendto(sd,send,8,0,(struct sockaddr *) &server,server_length);
		pressed=true;
	}
	else if (GetAsyncKeyState(VK_RIGHT)){
		char send[] = {'r','i','g','h','t','-',user[0],user[1],'\0'};
		sendto(sd,send,8,0,(struct sockaddr *) &server,server_length);
		pressed=true;
	}
	if (GetAsyncKeyState(VK_UP)){
		if (!pressed){
			char send[] = {'f','o','r','w','-','-',user[0],user[1],'\0'};
			sendto(sd,send,8,0,(struct sockaddr *) &server,server_length);
			pressed=true;
		}
	}
	else if (GetAsyncKeyState(VK_DOWN)){
		if (!pressed){
			char send[] = {'d','o','w','n','-','-',user[0],user[1],'\0'};
			sendto(sd,send,8,0,(struct sockaddr *) &server,server_length);
			pressed=true;
		}
	}
	else if (pressed){
		char send[] = {'s','t','o','p','-','-',user[0],user[1],'\0'};
		sendto(sd,send,8,0,(struct sockaddr *) &server,server_length);
		pressed=false;
	}
	if (GetAsyncKeyState(VK_SPACE)){
		char send[] = {'s','p','a','c','e','-',user[0],user[1],'\0'};
		sendto(sd,send,8,0,(struct sockaddr *) &server,server_length);
	}
}

VOID Create(){

	//CONNECT TO SERVER
	char* argv[]={"ZClient",IP,"1527"};
	int argc=3;
	/* Make sure command line is correct */
	if (argc != 3 && argc != 4){
		argv[1]="192.168.1.187";
		argv[2]="1528";
		argc=3;
		//usage();
	}
	if (sscanf_s((char *)argv[1], "%d.%d.%d.%d", &a1, &a2, &a3, &a4) != 4){
		exit(0);
	}
	if (sscanf_s((char *)argv[2], "%u", &port_number) != 1){
		exit(0);
	}
	if (argc == 4){
		if (sscanf_s((char *)argv[3], "%d.%d.%d.%d", &b1, &b2, &b3, &b4) != 4){
			exit(0);
		}
	}

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

	/* Set server address */
	server.sin_addr.S_un.S_un_b.s_b1 = (unsigned char)a1;
	server.sin_addr.S_un.S_un_b.s_b2 = (unsigned char)a2;
	server.sin_addr.S_un.S_un_b.s_b3 = (unsigned char)a3;
	server.sin_addr.S_un.S_un_b.s_b4 = (unsigned char)a4;

	/* Clear out client struct */
	memset((void *)&client, '\0', sizeof(struct sockaddr_in));

	/* Set family and port */
	client.sin_family = AF_INET;
	client.sin_port = htons(0);

	if (argc == 3){
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
		client.sin_addr.S_un.S_un_b.s_b1 = hp->h_addr_list[0][0];
		client.sin_addr.S_un.S_un_b.s_b2 = hp->h_addr_list[0][1];
		client.sin_addr.S_un.S_un_b.s_b3 = hp->h_addr_list[0][2];
		client.sin_addr.S_un.S_un_b.s_b4 = hp->h_addr_list[0][3];
	}
	else{
		client.sin_addr.S_un.S_un_b.s_b1 = (unsigned char)b1;
		client.sin_addr.S_un.S_un_b.s_b2 = (unsigned char)b2;
		client.sin_addr.S_un.S_un_b.s_b3 = (unsigned char)b3;
		client.sin_addr.S_un.S_un_b.s_b4 = (unsigned char)b4;
	}

	/* Bind local address to socket */
	if (bind(sd, (struct sockaddr *)&client, sizeof(struct sockaddr_in)) == -1){
		fprintf(stderr, "Cannot bind address to socket.\n");
		closesocket(sd);
		WSACleanup();
		exit(0);
	}
	server_length = sizeof(struct sockaddr_in);
	sendto(sd,"new",8,0,(struct sockaddr *) &server,server_length);  //client sends "new"
	recvfrom(sd,user,2,0,(struct sockaddr *) &server,&server_length); //server sends back new user id number.
}

VOID addObj(int od, int id, int x, int y, double rot){
	int width = Image(imgAdr[od]).GetWidth();
	int height = Image(imgAdr[od]).GetHeight();
	GameObject g(od,id,x,y,rot,width,height);
	GameObject *temp = new GameObject[O+1];
	for (int o=0;o<O;o++){
		temp[o]=obj[o];
	}
	temp[O]=g;
	delete[] obj;
	obj=temp;
	O++;
}

VOID delObj(int id){
	if (O==0) return; //server made a mistake

	int count=0;
	for (int i=0;i<O;i++) if (obj[i].id==id) count++;
	GameObject *temp=new GameObject[O-count];
	int i1=0;
	int i2=0;
	while (i1<O && i2<O-count){
		if (obj[i1].id!=id){
			temp[i2]=obj[i1];
			i2++;
		}
		i1++;
	}
	delete[] obj;
	obj=temp;
	
	//delete[] temp; Sorry, got it wrong.(-K)
	O-=count;
}

int getNum(char c1,char c2,char c3,char c4){
	char arr[]={c1,c2,c3,c4};
	return *((int*)arr);
}

//WINDOW PROCESSES
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, INT iCmdShow){
   HWND                hWnd;
   MSG                 msg;
   WNDCLASS            wndClass;
   GdiplusStartupInput gdiplusStartupInput;
   ULONG_PTR           gdiplusToken;
   
   // Initialize GDI+.
   GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
   
   wndClass.style          = CS_HREDRAW | CS_VREDRAW;
   wndClass.lpfnWndProc    = WndProc;
   wndClass.cbClsExtra     = 0;
   wndClass.cbWndExtra     = 0;
   wndClass.hInstance      = hInstance;
   wndClass.hIcon          = LoadIcon(NULL, IDI_APPLICATION);
   wndClass.hCursor        = LoadCursor(NULL, IDC_ARROW);
   wndClass.hbrBackground  = (HBRUSH)GetStockObject(WHITE_BRUSH);
   wndClass.lpszMenuName   = NULL;
   wndClass.lpszClassName  = TEXT("GettingStarted");
   
   RegisterClass(&wndClass);
   
   hWnd = CreateWindow(
      TEXT("GettingStarted"),   // window class name
      TEXT("ZOMBIEZZZ!!1!!!"),  // window caption
      WS_OVERLAPPEDWINDOW,      // window style
      CW_USEDEFAULT,            // initial x position
      CW_USEDEFAULT,            // initial y position
      CW_USEDEFAULT,            // initial x size
      CW_USEDEFAULT,            // initial y size
      NULL,                     // parent window handle
      NULL,                     // window menu handle
      hInstance,                // program instance handle
      NULL);                    // creation parameters
	  
   ShowWindow(hWnd, iCmdShow);
   UpdateWindow(hWnd);
   
   while(GetMessage(&msg, NULL, 0, 0))
   {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
   }
   
   GdiplusShutdown(gdiplusToken);
   return msg.wParam;
}  // WinMain

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, 
   WPARAM wParam, LPARAM lParam)
{
   HDC          hdc;
   PAINTSTRUCT  ps;
   int win_width=wid;
   int win_height=hei;
   switch(message)
   {
   case WM_CREATE:
	   Create();
		SetTimer(hWnd,TIMER_STEP,STEPTIME,NULL);
		RECT clientRect;
		GetClientRect(hWnd,&clientRect);
		return 0;
   case WM_PAINT:
	  /* RECT Client_Rect;
		GetClientRect(hWnd,&Client_Rect);
		win_width = Client_Rect.right - Client_Rect.left;
		win_height = Client_Rect.bottom + Client_Rect.left;
		PAINTSTRUCT ps;
		HDC Memhdc;
		HDC hdc;
		HBITMAP Membitmap;
		hdc = BeginPaint(hWnd, &ps);
		Memhdc = CreateCompatibleDC(hdc);
		Membitmap = CreateCompatibleBitmap(hdc, win_width, win_height);
		SelectObject(Memhdc, Membitmap);
		OnPaint(hdc);
		BitBlt(hdc, 0, 0, win_width, win_height, Memhdc, 0, 0, SRCCOPY);
		DeleteObject(Membitmap);
		DeleteDC    (Memhdc);
		DeleteDC    (hdc);
		EndPaint(hWnd, &ps);*/ //:( didn't quite work...
      hdc = BeginPaint(hWnd, &ps);
      OnPaint(hdc);
      EndPaint(hWnd, &ps);
      return 0;
   case WM_DESTROY:
      PostQuitMessage(0);
	  char tosend[8];//delete user and send message to server
	  tosend[0]='d';
	  tosend[1]='e';
	  tosend[2]='l';
	  tosend[3]='e';
	  tosend[4]='t';
	  tosend[5]='e';
	  tosend[6]=user[0];
	  tosend[7]='\0';
	  sendto(sd,tosend,8,0,(struct sockaddr*)&server,server_length);
	  delete[] obj;
      return 0;
   case WM_TIMER:
	   switch (wParam){
	   case TIMER_STEP:
			Step();
			InvalidateRect(hWnd,NULL,false);
		  break;
	   }
	   return 0;
   case WM_ERASEBKGND:
		return TRUE;

   default:
      return DefWindowProc(hWnd, message, wParam, lParam);
   }
} // WndProc
