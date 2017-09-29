
#ifndef GL_FRAMEWORK__INCLUDED
#define GL_FRAMEWORK__INCLUDED

#include <windows.h>								

typedef struct {									
	BOOL keyDown [256];								
} Keys;												

typedef struct {									
	HINSTANCE		hInstance;						
	const char*		className;						
} Application;										

typedef struct {									
	Application*		application;				
	char*				title;						
	int					width;						
	int					height;						
	int					bitsPerPixel;				
	BOOL				isFullScreen;				
} GL_WindowInit;									

typedef struct {									
	Keys*				keys;						
	HWND				hWnd;						
	HDC					hDC;					
	HGLRC				hRC;						
	GL_WindowInit		init;						
	BOOL				isVisible;				
	DWORD				lastTickCount;				
} GL_Window;										

void TerminateApplication (GL_Window* window);		

void ToggleFullscreen (GL_Window* window);			


BOOL Initialize (GL_Window* window, Keys* keys);	

void Deinitialize (void);							

void Update (DWORD milliseconds);					

int Draw ();									

#endif											
