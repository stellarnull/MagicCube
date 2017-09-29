#include <windows.h>													

#include <gl/gl.h>									
#include <gl/glu.h>														

#include "resource.h"
#include "CAD_A.h"									
#include "math.h"                                                      
#include "ArcBall.h"                                                   
#include "Cube.h"



#define WM_TOGGLEFULLSCREEN (WM_USER+1)									

static BOOL g_isProgramLooping;																													
static BOOL g_createFullScreen;											
extern ArcBallT    ArcBall;                                            
extern Point2fT    MousePt;                                            
extern bool        isClicked;                                          
extern bool        isRClicked;                                         
extern bool isSelected;
extern float zMovement;
extern int order;
extern int textureType;
extern Cube cubes[125];

void TerminateApplication (GL_Window* window)							
{
	PostMessage (window->hWnd, WM_QUIT, 0, 0);							
	g_isProgramLooping = FALSE;											
}

void ToggleFullscreen (GL_Window* window)								
{
	PostMessage (window->hWnd, WM_TOGGLEFULLSCREEN, 0, 0);				
}

void ReshapeGL (int width, int height)									
{
	glViewport (0, 0, (GLsizei)(width), (GLsizei)(height));				
	glMatrixMode (GL_PROJECTION);									
	glLoadIdentity ();												
	gluPerspective (45.0f, (GLfloat)(width)/(GLfloat)(height),			
					1.0f, 100.0f);		
	glMatrixMode (GL_MODELVIEW);							
	glLoadIdentity ();												

    ArcBall.setBounds((GLfloat)width, (GLfloat)height);                
}

BOOL ChangeScreenResolution (int width, int height, int bitsPerPixel)	
{
	DEVMODE dmScreenSettings;											
	ZeroMemory (&dmScreenSettings, sizeof (DEVMODE));					
	dmScreenSettings.dmSize				= sizeof (DEVMODE);				
	dmScreenSettings.dmPelsWidth		= width;						
	dmScreenSettings.dmPelsHeight		= height;						
	dmScreenSettings.dmBitsPerPel		= bitsPerPixel;					
	dmScreenSettings.dmFields			= DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
	if (ChangeDisplaySettings (&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
	{
		return FALSE;													
	}
	return TRUE;														
}

BOOL CreateWindowGL (GL_Window* window,HINSTANCE hInstance)									
{
	DWORD windowStyle = WS_OVERLAPPEDWINDOW;							
	DWORD windowExtendedStyle = WS_EX_APPWINDOW;						

	PIXELFORMATDESCRIPTOR pfd =											
	{
		sizeof (PIXELFORMATDESCRIPTOR),									
		1,													
		PFD_DRAW_TO_WINDOW |											
		PFD_SUPPORT_OPENGL |											
		PFD_DOUBLEBUFFER,											
		PFD_TYPE_RGBA,												
		window->init.bitsPerPixel,										
		0, 0, 0, 0, 0, 0,											
		0,													
		0,													
		0,																
		0, 0, 0, 0,														
		16,																
		0,																
		0,																
		PFD_MAIN_PLANE,													
		0,															
		0, 0, 0															
	};

	RECT windowRect = {0, 0, window->init.width, window->init.height};	

	GLuint PixelFormat;													

	if (window->init.isFullScreen == TRUE)								
	{
		if (ChangeScreenResolution (window->init.width, window->init.height, window->init.bitsPerPixel) == FALSE)
		{
			
			MessageBox (HWND_DESKTOP, "Mode Switch Failed.\nRunning In Windowed Mode.", "Error", MB_OK | MB_ICONEXCLAMATION);
			window->init.isFullScreen = FALSE;							
		}
		else														
		{
			windowStyle = WS_POPUP;										
			windowExtendedStyle |= WS_EX_TOPMOST;						
		}															
	}
	else																
	{
		
		AdjustWindowRectEx (&windowRect, windowStyle, 0, windowExtendedStyle);
	}

	
	window->hWnd = CreateWindowEx (windowExtendedStyle,					
								   window->init.application->className,
								   window->init.title,					
								   windowStyle,							
								   200, 100,								
								   windowRect.right - windowRect.left,	
								   windowRect.bottom - windowRect.top,	
								   HWND_DESKTOP,						
								   LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MENU1)), 									
								   window->init.application->hInstance,
								   window);

	if (window->hWnd == 0)												
	{
		return FALSE;													
	}

	window->hDC = GetDC (window->hWnd);									
	if (window->hDC == 0)												
	{
		DestroyWindow (window->hWnd);									
		window->hWnd = 0;											
		return FALSE;												
	}

	PixelFormat = ChoosePixelFormat (window->hDC, &pfd);				
	if (PixelFormat == 0)										
	{
		ReleaseDC (window->hWnd, window->hDC);							
		window->hDC = 0;									
		DestroyWindow (window->hWnd);								
		window->hWnd = 0;											
		return FALSE;												
	}

	if (SetPixelFormat (window->hDC, PixelFormat, &pfd) == FALSE)		
	{
		ReleaseDC (window->hWnd, window->hDC);							
		window->hDC = 0;									
		DestroyWindow (window->hWnd);								
		window->hWnd = 0;									
		return FALSE;										
	}

	window->hRC = wglCreateContext (window->hDC);						
	if (window->hRC == 0)										
	{
		ReleaseDC (window->hWnd, window->hDC);						
		window->hDC = 0;									
		DestroyWindow (window->hWnd);								
		window->hWnd = 0;											
		return FALSE;												
	}

	if (wglMakeCurrent (window->hDC, window->hRC) == FALSE)
	{
		wglDeleteContext (window->hRC);									
		window->hRC = 0;									
		ReleaseDC (window->hWnd, window->hDC);							
		window->hDC = 0;										
		DestroyWindow (window->hWnd);									
		window->hWnd = 0;										
		return FALSE;											
	}

	ShowWindow (window->hWnd, SW_NORMAL);								
	window->isVisible = TRUE;									

	ReshapeGL (window->init.width, window->init.height);				

	ZeroMemory (window->keys, sizeof (Keys));							

	window->lastTickCount = GetTickCount ();							

	return TRUE;											
													
}

BOOL DestroyWindowGL (GL_Window* window)								
{
	if (window->hWnd != 0)										
	{	
		if (window->hDC != 0)											
		{
			wglMakeCurrent (window->hDC, 0);							
			if (window->hRC != 0)										
			{
				wglDeleteContext (window->hRC);							
				window->hRC = 0;									

			}
			ReleaseDC (window->hWnd, window->hDC);						
			window->hDC = 0;								
		}
		DestroyWindow (window->hWnd);								
		window->hWnd = 0;										
	}

	if (window->init.isFullScreen)										
		ChangeDisplaySettings (NULL,0);									

	return TRUE;													
}

LRESULT CALLBACK WindowProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	
	GL_Window* window = (GL_Window*)(GetWindowLong (hWnd, GWL_USERDATA));

	switch (uMsg)														
	{
       
        case WM_MOUSEMOVE:
            MousePt.s.X = (GLfloat)LOWORD(lParam);
            MousePt.s.Y = (GLfloat)HIWORD(lParam);
            isClicked   = (LOWORD(wParam) & MK_LBUTTON) ? true : false;
            isRClicked  = (LOWORD(wParam) & MK_RBUTTON) ? true : false;
            break;
        case WM_LBUTTONUP:
            isClicked   = false;
            break;
        case WM_RBUTTONUP:
            isRClicked  = false;
            break;
        case WM_LBUTTONDOWN:
            isClicked   = true;
            break;
        case WM_RBUTTONDOWN:
            isRClicked  = true;
            break;
		case WM_MOUSEWHEEL:
			 if( HIWORD(wParam)> WHEEL_DELTA ) 
			 {
				 if(zMovement <= 20)
					 zMovement += 0.5;
			 }
			 else if( HIWORD(wParam) == WHEEL_DELTA ) 
			 {		 
				 if(zMovement >= 8) zMovement -= 0.5;
			 }
			 break;
		case WM_SYSCOMMAND:												
		{
			switch (wParam)												
			{
				case SC_SCREENSAVE:										
				case SC_MONITORPOWER:									
				return 0;										
			}
			break;												
		}
		return 0;												

		case WM_CREATE:												
		{
			CREATESTRUCT* creation = (CREATESTRUCT*)(lParam);			
			window = (GL_Window*)(creation->lpCreateParams);
			SetWindowLong (hWnd, GWL_USERDATA, (LONG)(window));
		}
		return 0;														

		case WM_CLOSE:													
			TerminateApplication(window);								
		return 0;												

		case WM_SIZE:												
			switch (wParam)												
			{
				case SIZE_MINIMIZED:									
					window->isVisible = FALSE;							
				return 0;											

				case SIZE_MAXIMIZED:									
					window->isVisible = TRUE;							
					ReshapeGL (LOWORD (lParam), HIWORD (lParam));		
				return 0;												

				case SIZE_RESTORED:										
					window->isVisible = TRUE;							
					ReshapeGL (LOWORD (lParam), HIWORD (lParam));		
				return 0;												
			}
		break;															

		case WM_KEYDOWN:												
			if ((wParam >= 0) && (wParam <= 255))						
			{
				window->keys->keyDown [wParam] = TRUE;					
				return 0;									
			}
		break;															

		case WM_KEYUP:													
			if ((wParam >= 0) && (wParam <= 255))						
			{
				window->keys->keyDown [wParam] = FALSE;					
				return 0;												
			}
		break;															

		case WM_TOGGLEFULLSCREEN:										
			g_createFullScreen = (g_createFullScreen == TRUE) ? FALSE : TRUE;
			PostMessage (hWnd, WM_QUIT, 0, 0);
		break;															

		case WM_COMMAND:
			{
				switch(LOWORD(wParam))  
				{
				case ID_order2:
					order = 2;
					InitializeCubes(cubes);
					break;
				case ID_order3:
					order = 3;
					InitializeCubes(cubes);
					break;
				case ID_order4:
					order = 4;
					InitializeCubes(cubes);
					break;
				case ID_order5:
					order = 5;
					InitializeCubes(cubes);
					break;
				case ID_texture0:
					glDisable(GL_TEXTURE_2D);
					textureType = 0;
					break;
				case ID_texture1:
					glEnable(GL_TEXTURE_2D);
					textureType = 1;
					break;
				case ID_texture2:
					glEnable(GL_TEXTURE_2D);
					textureType = 2;
					break;
				case ID_light1:
					glDisable(GL_LIGHT2);
					glDisable(GL_LIGHT3);
					glEnable(GL_LIGHT1);
					break;
				case ID_light2:
					glDisable(GL_LIGHT1);
					glDisable(GL_LIGHT3);
					glEnable(GL_LIGHT2);
					break;
				case ID_light3:
					glDisable(GL_LIGHT2);
					glDisable(GL_LIGHT1);
					glEnable(GL_LIGHT3);
					break;
				case ID_bg1:
					glClearColor (	0.0f, 0.0f, 0.0f, 0.5f);
					break;
				case ID_bg2:
					glClearColor (	1.0f, 0.6f, 0.6f, 0.0f);
					break;
				case ID_bg3:
					glClearColor (	0.2f, 0.2f, 0.7f, 0.2f);
					break;

				}
			}
		break;
	}

	return DefWindowProc (hWnd, uMsg, wParam, lParam);					
}

BOOL RegisterWindowClass (Application* application)						
{																		
	
	WNDCLASSEX windowClass;												
	ZeroMemory (&windowClass, sizeof (WNDCLASSEX));						
	windowClass.cbSize			= sizeof (WNDCLASSEX);					
	windowClass.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	
	windowClass.lpfnWndProc		= (WNDPROC)(WindowProc);				
	windowClass.hInstance		= application->hInstance;				
	windowClass.hbrBackground	= (HBRUSH)(COLOR_APPWORKSPACE);			
	windowClass.hCursor			= LoadCursor(NULL, IDC_ARROW);			
	windowClass.lpszClassName	= application->className;				
	if (RegisterClassEx (&windowClass) == 0)							
	{
		
		MessageBox (HWND_DESKTOP, "RegisterClassEx Failed!", "Error", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;													
	}
	return TRUE;														
}


int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	Application			application;									
	GL_Window			window;											
	Keys				keys;											
	BOOL				isMessagePumpActive;							
	MSG					msg;										
	DWORD				tickCount;										

	
	application.className = "OpenGL";									
	application.hInstance = hInstance;									

	
	ZeroMemory (&window, sizeof (GL_Window));							
	window.keys					= &keys;								
	window.init.application		= &application;							
	window.init.title			= "Cube";
	window.init.width			= 640;									
	window.init.height			= 480;									
	window.init.bitsPerPixel	= 32;									
	window.init.isFullScreen	= TRUE;									

	ZeroMemory (&keys, sizeof (Keys));									

	
	window.init.isFullScreen = FALSE;

	
	if (RegisterWindowClass (&application) == FALSE)			
	{
		
		MessageBox (HWND_DESKTOP, "Error Registering Window Class!", "Error", MB_OK | MB_ICONEXCLAMATION);
		return -1;													
	}


 


	g_isProgramLooping = TRUE;											
	g_createFullScreen = window.init.isFullScreen;						
	while (g_isProgramLooping)											
	{
		
		window.init.isFullScreen = g_createFullScreen;					
		if (CreateWindowGL (&window,hInstance) == TRUE)						
		{
			
			if (Initialize (&window, &keys) == FALSE)				
			{
				
				TerminateApplication (&window);							
			}
			else														
			{	
				isMessagePumpActive = TRUE;								
				while (isMessagePumpActive == TRUE)						
				{
					
					if (PeekMessage (&msg, window.hWnd, 0, 0, PM_REMOVE) != 0)
					{
						
						if (msg.message != WM_QUIT)						
						{
							DispatchMessage (&msg);						
						}
						else										
						{
							isMessagePumpActive = FALSE;				
						}
					}
					else											
					{
						if (window.isVisible == FALSE)					
						{
							WaitMessage ();								
						}
						else										
						{
							
							tickCount = GetTickCount ();				
							Update (tickCount - window.lastTickCount);	
							window.lastTickCount = tickCount;			
							Draw();									

							SwapBuffers (window.hDC);					
						}
					}
				}														
			}															

			
			Deinitialize ();											

			DestroyWindowGL (&window);									
		}
		else															
		{
			
			MessageBox (HWND_DESKTOP, "Error Creating OpenGL Window", "Error", MB_OK | MB_ICONEXCLAMATION);
			g_isProgramLooping = FALSE;									
		}
	}																

	UnregisterClass (application.className, application.hInstance);		
	return 0;
}																	