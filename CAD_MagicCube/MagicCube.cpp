#include <windows.h>												

#include <gl\gl.h>													
#include <gl\glu.h>													
#include <gl\GLAUX.h>
#include <stdio.h>			
#include "CAD_A.h"													
#include "Cube.h"

#include <math.h>												    
#include "ArcBall.h"												

#pragma comment( lib, "opengl32.lib" )								
#pragma comment( lib, "glu32.lib" )									
#pragma comment( lib, "glaux.lib" )									

#ifndef CDS_FULLSCREEN												
#define CDS_FULLSCREEN 4											
#endif																

#define SPEED 3.0f

GL_Window*	g_window;
Keys*		g_keys;
RECT rcWin;    //当前窗口矩形


GLUquadricObj *quadratic;										

Matrix4fT   Transform   = {  1.0f,  0.0f,  0.0f,  0.0f,				
	0.0f,  1.0f,  0.0f,  0.0f,
	0.0f,  0.0f,  1.0f,  0.0f,
	0.0f,  0.0f,  0.0f,  1.0f };

Matrix3fT   LastRot     = {  1.0f,  0.0f,  0.0f,				
	0.0f,  1.0f,  0.0f,
	0.0f,  0.0f,  1.0f };

Matrix3fT   ThisRot     = {  1.0f,  0.0f,  0.0f,					
	0.0f,  1.0f,  0.0f,
	0.0f,  0.0f,  1.0f };

GLfloat LightAmbient1[]= { 1.0f, 1.0f, 1.0f, 1.0f }; 				// 环境光参数
GLfloat LightDiffuse1[]= { 1.0f, 1.0f, 1.0f, 1.0f };				 // 漫射光参数
GLfloat LightPosition1[]= { 0.0f, 0.0f, 2.0f, 1.0f };				 // 光源位置

GLfloat LightAmbient2[]= { 1.0f, 1.0f, 1.0f, 1.0f }; 				// 环境光参数
GLfloat LightDiffuse2[]= { 1.0f, 1.0f, 1.0f, 1.0f };				 // 漫射光参数
GLfloat LightPosition2[]= { 0.0f, 3.0f, -5.0f, 1.0f };				 // 光源位置

GLfloat LightAmbient3[]= { 0.5f, 0.5f, 0.5f, 1.0f }; 				// 环境光参数
GLfloat LightDiffuse3[]= { 0.5f, 0.0f, 0.5f, 1.0f };				 // 漫射光参数
GLfloat LightSpecular3[]= { 1.0f, 1.0f, 1.0f,1.0f };	
GLfloat LightPosition3[]= { 0.0f, 0.0f, 2.0f,1.0f };				 // 光源位置



ArcBallT    ArcBall(640.0f, 480.0f);				               
Point2fT    MousePt;		//当前的鼠标位置
Point2fT	MousePtLast;	//前一步的鼠标位置

float xMovement = 0;	//图形绘制位置在x方向上的变动
float yMovement = 0;
float zMovement = 10.0f;	//图形绘制位置在z方向上的变动，初始化为6，即向屏幕内移动6个单位
float WndHeight; //用于记录当前的窗口大小
float WndWidth; //用于记录当前的窗口大小
float rotAngle = 0;  //方块旋转角
float delta = 0;
float distance;
float center1[3] = {0,0,0},center2[3]={0,0,0};

bool        isClicked  = false;										
bool        isRClicked = false;										
bool        isDragging = false;					                   
bool		isSelected = false;  //魔方方块被选中
bool		isRotating = false; //魔方正在旋转中时，不接受新的旋转指令
bool		rot90 = false;//旋转到90度了

int order = 3; //魔方阶数
int modelselect  = -1; //离眼睛最近的物件的名字（ID）
int cube1 = -1,cube2 = -1; //鼠标拉动选中的两个魔方块
int face1 = -1,face2 = -1; //鼠标拉动选中的两个小贴面
int rotAxis = -1;  //旋转轴，0为x，1为y，2为z
float layer = 0;   //哪一层的方块在旋转
int textureType = 0;

int eachSurface[6][4] = {
		1,4,3,2,
		1,2,6,5,
		2,3,7,6,
		3,4,8,7,
		4,1,5,8,
		5,6,7,8
	}; //每个表面有哪些点

GLfloat colors[7][3] = {
		0.5f,0.5f,0.5f,
		1,1,1,
		1,0,0,
		0,1,0,
		0,0,1,		
		0,1,1,
		1,1,0
	};//各个表面的初始颜色

GLuint	texture[12];
Cube cubes[125];


void InitializeSurfaces(Cube &cube,float startPoint)
{
	distance = startPoint - 0.5f; 
	bool outside[9] = {0,0,0,0,0,0,0,0,0};
	for(int i = 1;i<9;i++) //判断点是否位于外表面
	{ 
		for(int j = 0;j<3;j++)
		{
			if(cube.points[i][j]==distance||cube.points[i][j]==-distance)
			{
				outside[i] = true;
			}
		}
	}
	for(int k = 0;k < 6;k++) //标记好每个表面的状态
	{
		if(outside[eachSurface[k][0]]&&outside[eachSurface[k][1]]&&outside[eachSurface[k][2]]&&outside[eachSurface[k][3]])
		{
			cube.outerSurface[k] = true;
			cube.surfaceType[k] = k + 1;
		}
		else
		{
			cube.outerSurface[k] = false;
			cube.surfaceType[k] = 0;
		}
	}
}

void InitializeCubes(Cube * cubes)  //初始化每个方块的坐标
{
	int num = 0;
	float x,y,z;
	float startPoint;
	if(order%2 == 1) startPoint = (1-order)/2.0f;
	else startPoint = 0.5f-order/2;
	
	z = startPoint;
	for(int i = 0;i < order;i++)
	{
		y = startPoint;
		for(int j = 0; j < order;j++)
		{
			x = startPoint;
			for(int k = 0; k < order; k ++)
			{
				cubes[num].posID = num;
				//中心点
				cubes[num].points[0][0] = x;
				cubes[num].points[0][1] = y;
				cubes[num].points[0][2] = z;
				//顶点
				cubes[num].points[1][0] = x-0.5f;
				cubes[num].points[1][1] = y-0.5f;
				cubes[num].points[1][2] = z-0.5f;
				cubes[num].points[2][0] = x+0.5f;
				cubes[num].points[2][1] = y-0.5f;
				cubes[num].points[2][2] = z-0.5f;
				cubes[num].points[3][0] = x+0.5f;
				cubes[num].points[3][1] = y+0.5f;
				cubes[num].points[3][2] = z-0.5f;
				cubes[num].points[4][0] = x-0.5f;
				cubes[num].points[4][1] = y+0.5f;
				cubes[num].points[4][2] = z-0.5f;
				cubes[num].points[5][0] = x-0.5f;
				cubes[num].points[5][1] = y-0.5f;
				cubes[num].points[5][2] = z+0.5f;
				cubes[num].points[6][0] = x+0.5f;
				cubes[num].points[6][1] = y-0.5f;
				cubes[num].points[6][2] = z+0.5f;
				cubes[num].points[7][0] = x+0.5f;
				cubes[num].points[7][1] = y+0.5f;
				cubes[num].points[7][2] = z+0.5f;
				cubes[num].points[8][0] = x-0.5f;
				cubes[num].points[8][1] = y+0.5f;
				cubes[num].points[8][2] = z+0.5f;

				InitializeSurfaces(cubes[num],startPoint);
				num++;
				x += 1.0;
			}
			y += 1.0;
		}
		z += 1.0;
	}
}

AUX_RGBImageRec *LoadBMP(char *Filename)				
{
	FILE *File=NULL;									

	if (!Filename)										
	{
		return NULL;									
	}

	File=fopen(Filename,"r");							

	if (File)											
	{
		fclose(File);									
		return auxDIBImageLoad(Filename);				
	}

	return NULL;										
}

int LoadGLTextures()									
{
	int Status=FALSE;									

	AUX_RGBImageRec *TextureImage[12];					

	memset(TextureImage,0,sizeof(void *)*12);           

	// Load The Bitmap, Check For Errors, If Bitmap's Not Found Quit
	if ((TextureImage[0]=LoadBMP("Data/a1.bmp"))&&(TextureImage[1]=LoadBMP("Data/a2.bmp"))
		&&(TextureImage[2]=LoadBMP("Data/a3.bmp"))&&(TextureImage[3]=LoadBMP("Data/a4.bmp"))
		&&(TextureImage[4]=LoadBMP("Data/a5.bmp"))&&(TextureImage[5]=LoadBMP("Data/a6.bmp"))
		&&(TextureImage[6]=LoadBMP("Data/b1.bmp"))&&(TextureImage[7]=LoadBMP("Data/b2.bmp"))
		&&(TextureImage[8]=LoadBMP("Data/b3.bmp"))&&(TextureImage[9]=LoadBMP("Data/b4.bmp"))
		&&(TextureImage[10]=LoadBMP("Data/b5.bmp"))&&(TextureImage[11]=LoadBMP("Data/b6.bmp")))
	{
		Status=TRUE;									

		glEnable(GL_TEXTURE_2D);
		glGenTextures(12, &texture[0]);					
	
		// Typical Texture Generation Using Data From The Bitmap
		for(int i = 0;i<12;i++)
		{
			glBindTexture(GL_TEXTURE_2D, texture[i]);
			glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[i]->sizeX, TextureImage[i]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[i]->data);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);		
		}
	}
	for(int i = 0;i<12;i++)
	{
		if (TextureImage[i])									
		{
			if (TextureImage[i]->data)							
			{
				free(TextureImage[i]->data);					
			}

			free(TextureImage[i]);								
		}
	}
	return Status;										
}

BOOL Initialize (GL_Window* window, Keys* keys)						
{
	g_window	= window;
	g_keys		= keys;

	if (!LoadGLTextures())								
	{
		MessageBox (HWND_DESKTOP, "Cannot find textures! Please check file folder\":\\Data\\\"", "Error", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;									
	}

	// Start Of User Initialization
	isClicked   = false;								           
	isDragging  = false;							               

	glDisable(GL_TEXTURE_2D);
	glClearColor (	0.0f, 0.0f, 0.0f, 0.5f);						
	glClearDepth (1.0f);											
	glDepthFunc (GL_LEQUAL);										
	glEnable (GL_DEPTH_TEST);										
	glShadeModel (GL_FLAT);											
	glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);				

	quadratic=gluNewQuadric();										
	gluQuadricNormals(quadratic, GLU_SMOOTH);						
	gluQuadricTexture(quadratic, GL_TRUE);							

	//glEnable(GL_LIGHT0);											
	//glEnable(GL_LIGHTING);											

	glEnable(GL_COLOR_MATERIAL);									

	glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient1);				// 设置环境光
	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse1);				// 设置漫射光
	glLightfv(GL_LIGHT1, GL_POSITION,LightPosition1);			// 设置光源位置
	glEnable(GL_LIGHT1);							// 启用一号光源

	glLightfv(GL_LIGHT2, GL_AMBIENT, LightAmbient2);				// 设置环境光
	glLightfv(GL_LIGHT2, GL_DIFFUSE, LightDiffuse2);				// 设置漫射光
	glLightfv(GL_LIGHT2, GL_POSITION,LightPosition2);			// 设置光源位置
	glLightf( GL_LIGHT2 , GL_CONSTANT_ATTENUATION , 1.0 );
	glLightf( GL_LIGHT2 , GL_LINEAR_ATTENUATION , 0.01 );
	glLightf( GL_LIGHT2 , GL_QUADRATIC_ATTENUATION , 0.02 );

	glLightfv(GL_LIGHT3, GL_AMBIENT, LightAmbient3);				// 设置环境光
	glLightfv(GL_LIGHT3, GL_DIFFUSE, LightDiffuse3);				// 设置漫射光
	glLightfv(GL_LIGHT3, GL_POSITION,LightPosition3);			// 设置光源位置
	glLightfv(GL_LIGHT3, GL_SPECULAR,LightSpecular3);

	

	glEnable(GL_LIGHTING);



	InitializeCubes(cubes);
	return TRUE;													
}

void Deinitialize (void)											
{
	gluDeleteQuadric(quadratic);
}



void getFaceCenter(int face,float * center)
{
	int cube = face/6;
	int side = face%6;
	center[0] = 0.5f*(cubes[cube].points[eachSurface[side][0]][0]+cubes[cube].points[eachSurface[side][2]][0]);
	center[1] = 0.5f*(cubes[cube].points[eachSurface[side][0]][1]+cubes[cube].points[eachSurface[side][2]][1]);
	center[2] = 0.5f*(cubes[cube].points[eachSurface[side][0]][2]+cubes[cube].points[eachSurface[side][2]][2]);
}

void Update (DWORD milliseconds)									
{
	if (g_keys->keyDown [VK_ESCAPE] == TRUE)						
		TerminateApplication (g_window);							

	if (g_keys->keyDown [VK_F1] == TRUE)							
		ToggleFullscreen (g_window);								


	if (!isDragging)												
	{
		if (isClicked)												
		{
			if(!isRotating)
			{
				isDragging = true;										
				isSelected = true;

				face1 = Draw(); 
				cube1 = face1/6;
			}
		
		}
		else if(isRClicked)
		{
			isDragging = true;										
			LastRot = ThisRot;										
			ArcBall.click(&MousePt);								
		}
	}
	else
	{
		if (isClicked)												
		{
			if(!isRotating)
			{
				int tempSelect = Draw();
				if(tempSelect!= face1 && face2 == -1)
				{
					face2 = tempSelect;				
				}
			}
						
		}
		else if(isRClicked)
		{
			Quat4fT ThisQuat; 
			ArcBall.drag(&MousePt, &ThisQuat);						
			Matrix3fSetRotationFromQuat4f(&ThisRot, &ThisQuat);		
			Matrix3fMulMatrix3f(&ThisRot, &LastRot);				
			Matrix4fSetRotationFromMatrix3f(&Transform, &ThisRot);	
		}
		else													
		{
			isDragging = false;
			isSelected = false;
			if(face2 != -1)
			{
				isRotating = true;

					cube2 = face2/6;
					getFaceCenter(face1,center1);
					getFaceCenter(face2,center2);

					if((center1[0] == center2[0])&&(fabs(center1[0])!= fabs(distance)))
					{
						rotAxis = 0;
						layer = cubes[cube1].points[0][0];
						if((center1[1]*center2[2]-center1[2]*center2[1])>0) delta = SPEED;
						else delta = -SPEED;
					}
					else if((center1[1] == center2[1])&&(fabs(center1[1])!= fabs(distance)))
					{
						rotAxis = 1;
						layer = cubes[cube1].points[0][1];
						if((center1[2]*center2[0]-center1[0]*center2[2])>0) delta = SPEED;
						else delta = -SPEED;
					}
					else if((center1[2] == center2[2])&&(fabs(center1[2])!= fabs(distance)))
					{
						rotAxis = 2;
						layer = cubes[cube1].points[0][2];
						if((center1[0]*center2[1]-center1[1]*center2[0])>0) delta = SPEED;
						else delta = -SPEED;
					}
					else
					{
						isRotating = false;
					}

					face2 = -1;
			}
			cube1 = -1;
			cube2 = -1;
		}
	}
}

void DrawCube(Cube &cube)					
{
	if(isRotating)
	{
		if(cube.points[0][rotAxis] == layer)
		{
			if(rot90)//旋转了90度后，更新每个转完的方块的坐标
			{
				float temp;
				for(int k = 0; k < 9;k++) 
				{
					switch(rotAxis)
					{
					case 0:
						temp = cube.points[k][1];
						cube.points[k][1] = -cube.points[k][2]*((rotAngle>0)?1:-1);
						cube.points[k][2] = temp*((rotAngle>0)?1:-1);
						break;
					case 1:
						temp = cube.points[k][2];
						cube.points[k][2] = -cube.points[k][0]*((rotAngle>0)?1:-1);
						cube.points[k][0] = temp*((rotAngle>0)?1:-1);
						break;
					case 2:
						temp = cube.points[k][0];
						cube.points[k][0] = -cube.points[k][1]*((rotAngle>0)?1:-1);
						cube.points[k][1] = temp*((rotAngle>0)?1:-1);
						break;
					default:break;
					}
				}
			}
			else //若没到90度，则继续旋转
			{
				switch(rotAxis)
				{
				case 0:glRotatef(rotAngle,1,0,0);break;
				case 1:glRotatef(rotAngle,0,1,0);break;
				case 2:glRotatef(rotAngle,0,0,1);break;
				default:break;
				}
			}
			
		}
	}
	
	for(int i = 0; i < 6;i++) //分别绘制一个块的六个面
	{
		if(isSelected)
		{
			glPushName(i+cube.posID*6);
		}

		if(textureType >0)
		{
			glBindTexture(GL_TEXTURE_2D, texture[cube.surfaceType[i]-7 + textureType*6]);
		}
		
		if(textureType == 0) 
		{
			glColor3f(colors[cube.surfaceType[i]][0],colors[cube.surfaceType[i]][1],colors[cube.surfaceType[i]][2]);
		}
		else
		{
			glColor3f(1,1,1);
		}


		//下面针对光照模型设置法线方向
		float direction = 1;
		int dir = 0;
		for(int j = 0;j<3;j++)
		{
			if(cube.points[eachSurface[i][0]][j]==cube.points[eachSurface[i][2]][j])
			{
				dir = j;
				direction = cube.points[eachSurface[i][0]][j]/fabs(cube.points[eachSurface[i][0]][j]);
			}
		}
		switch(dir)
		{
		case 0:
			glNormal3f(direction, 0.0f, 0.0f);
			break;
		case 1:
			glNormal3f( 0.0f, direction,0.0f);
			break;
		case 2:
			glNormal3f(0.0f, 0.0f,direction);
			break;
		}

		// 开始绘制面
		glBegin(GL_QUADS);					
		if(textureType >0)glTexCoord2f(0.0f, 0.0f);
		glVertex3f(cube.points[eachSurface[i][0]][0],cube.points[eachSurface[i][0]][1],cube.points[eachSurface[i][0]][2]);
		if(textureType >0)glTexCoord2f(1.0f, 0.0f);
		glVertex3f(cube.points[eachSurface[i][1]][0],cube.points[eachSurface[i][1]][1],cube.points[eachSurface[i][1]][2]);
		if(textureType >0)glTexCoord2f(1.0f, 1.0f);
		glVertex3f(cube.points[eachSurface[i][2]][0],cube.points[eachSurface[i][2]][1],cube.points[eachSurface[i][2]][2]);
		if(textureType >0)glTexCoord2f(0.0f, 1.0f);
		glVertex3f(cube.points[eachSurface[i][3]][0],cube.points[eachSurface[i][3]][1],cube.points[eachSurface[i][3]][2]);
		glEnd();

		//画边框线 
		glLineWidth(3);
		glColor3f(0,0,0);
		glBegin(GL_LINES); //画边框线
		glVertex3f(cube.points[eachSurface[i][0]][0],cube.points[eachSurface[i][0]][1],cube.points[eachSurface[i][0]][2]);
		glVertex3f(cube.points[eachSurface[i][1]][0],cube.points[eachSurface[i][1]][1],cube.points[eachSurface[i][1]][2]);

		glVertex3f(cube.points[eachSurface[i][1]][0],cube.points[eachSurface[i][1]][1],cube.points[eachSurface[i][1]][2]);
		glVertex3f(cube.points[eachSurface[i][2]][0],cube.points[eachSurface[i][2]][1],cube.points[eachSurface[i][2]][2]);

		glVertex3f(cube.points[eachSurface[i][2]][0],cube.points[eachSurface[i][2]][1],cube.points[eachSurface[i][2]][2]);
		glVertex3f(cube.points[eachSurface[i][3]][0],cube.points[eachSurface[i][3]][1],cube.points[eachSurface[i][3]][2]);

		glVertex3f(cube.points[eachSurface[i][3]][0],cube.points[eachSurface[i][3]][1],cube.points[eachSurface[i][3]][2]);
		glVertex3f(cube.points[eachSurface[i][0]][0],cube.points[eachSurface[i][0]][1],cube.points[eachSurface[i][0]][2]);
		glEnd();
		

		if(isSelected)
		{
			glPopName();
		}
	}
	
	if(isRotating)
	{
		if(!rot90)//画完之后旋转回来
		{
			if(cube.points[0][rotAxis] == layer)
			{
				switch(rotAxis)
				{
				case 0:glRotatef(-rotAngle,1,0,0);break;
				case 1:glRotatef(-rotAngle,0,1,0);break;
				case 2:glRotatef(-rotAngle,0,0,1);break;
				default:break;
				}
			}
		}
	}
	
}

int processHits (GLint hits, GLuint selectBuff[])
{
	modelselect = -1;
	if (hits >0)
	{
		int n=0;  
		unsigned int minz=selectBuff[1];
		unsigned int temp;
		for(int i=1;i<hits;i++)
		{
			temp = selectBuff[1+i*4];
			if (temp<minz) 
			{
				n=i;
				minz=temp;
			}
		}
		modelselect = selectBuff[3+n*4];
	}
	return modelselect;
}

int Draw() //绘制魔方
{
	GetWindowRect(g_window->hWnd,&rcWin);
	WndHeight = float(rcWin.bottom - rcWin.top);
	int id = -1;
	if(isRotating)
	{
		rotAngle += delta;
		if(fabs(rotAngle) >= 90.0f)
		{
			rot90 = true;
		}
	}

	if(isSelected)
	{
		GLuint selectBuf[512],hits = 0;
		GLint viewport[4];//存放可视区参数
		glGetIntegerv(GL_VIEWPORT,viewport);//获取当前视口坐标参数
		glSelectBuffer(512,selectBuf);   //指定将“图元列表”（点击记录）返回到selectBuf数组中
		glRenderMode (GL_SELECT);
		glInitNames();
		
		glMatrixMode(GL_PROJECTION);
		glPushMatrix(); 
		glLoadIdentity();
		gluPickMatrix((GLdouble)MousePt.s.X,(GLdouble)(viewport[3]-MousePt.s.Y),2,2,viewport);//创建用于选择的投影矩阵栈
		gluPerspective(45.0f, (GLfloat) (viewport[2]-viewport[0])/(GLfloat) (viewport[3]-viewport[1]), 0.1f, 1000.0f);
		glMatrixMode(GL_MODELVIEW);							// 选择模型变换矩阵

		// 选择绘制魔方
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);				
		glLoadIdentity();
		glPushMatrix();	
		glTranslatef(xMovement*zMovement/(1.25f*WndHeight),yMovement*zMovement/(1.25f*WndHeight),-zMovement);
		glMultMatrixf(Transform.M);										

		//绘制每个块
		for(int i = 0;i<order*order*order;i++)
		{
			DrawCube(cubes[i]);
		}

		glPopMatrix();
		glMatrixMode(GL_PROJECTION); 
		glPopMatrix(); 
		glMatrixMode(GL_MODELVIEW); 

		hits = glRenderMode (GL_RENDER);
		id = processHits (hits, selectBuf);
		glFlush();
	}
	else
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);				
		glLoadIdentity();											
		glTranslatef(xMovement*zMovement/(1.25f*WndHeight),yMovement*zMovement/(1.25f*WndHeight),-zMovement);

		glPushMatrix();												
		glMultMatrixf(Transform.M);									
		for(int i = 0;i<order*order*order;i++)
		{
			DrawCube(cubes[i]);
		}
		glPopMatrix();												
		glFlush ();														
	}
	if(isRotating)
	{
		if(rot90)
		{
			isRotating = false;
			rot90 = false;
			rotAngle = 0;
			cube1 = -1;
			cube2 = -1;
			face1 = -1;
			face2 = -1;
			isSelected = false;
		}
	}
	return id;
}


