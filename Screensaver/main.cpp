#include <windows.h>
#include <scrnsave.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include "CScene.h"
#include <time.h>
#include <cstring>

//globals used by the function below to hold the screen size
int Width;      
int Height;
        
static CScene* scene;
static DWORD g_startTick = 0;
static POINT g_initialCursorPos = {};

//define a Windows timer 
#define TIMER_1 1 

static void SetWorkingDirectoryToModuleFolder()
{
	char modulePath[MAX_PATH] = {};
	if (GetModuleFileNameA(nullptr, modulePath, MAX_PATH) == 0)
		return;

	char* lastSlash = strrchr(modulePath, '\\');
	if (lastSlash == nullptr)
		return;

	*lastSlash = '\0';
	SetCurrentDirectoryA(modulePath);
}

static stProperties LoadProperties(){

	HKEY hk;
	RegCreateKeyA(HKEY_CURRENT_USER,"Gren_Screen_Save",&hk);
	RegCloseKey(hk);

	stProperties defaults = {200,0.05f,0.1f};
	return defaults;
}

static void InitGL(HWND hWnd, HDC & hDC, HGLRC & hRC)
{
	SetWorkingDirectoryToModuleFolder();

	//WINDOWS
	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory( &pfd, sizeof pfd );
	pfd.nSize = sizeof pfd;
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 24;
	
	hDC = GetDC( hWnd );
  
	int i = ChoosePixelFormat( hDC, &pfd );  
	SetPixelFormat( hDC, i, &pfd );

	hRC = wglCreateContext( hDC );
	wglMakeCurrent( hDC, hRC );

	
	//OPENGL 
	RECT clientRect;
	GetClientRect(hWnd, &clientRect);
	int width = clientRect.right-clientRect.left;
	int height = clientRect.bottom-clientRect.top;

	if (height <= 0)
        height = 1;
    
	double aspectratio = (double)width / height;
	
	glClearColor(0,0,0,1);
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, aspectratio, 0.2f, 80000.f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D );
	glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);
   
	// Setup lighting
	glEnable( GL_LIGHTING );
	glEnable( GL_LIGHT0 );

	float fAmbientColor[] = { 1.0f, 1.0f, 1.0f, 1.0f};
	glLightfv( GL_LIGHT0, GL_AMBIENT, fAmbientColor );

	float fDiffuseColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv( GL_LIGHT0, GL_DIFFUSE, fDiffuseColor );

	float fSpecularColor[] = { .0f, .0f, .0f, 1.0f };
	glLightfv( GL_LIGHT0, GL_SPECULAR, fSpecularColor );

	float fPosition[] = { .0f, .0f, 2.0f, 1.0f };
	glLightfv( GL_LIGHT0, GL_POSITION, fPosition );

	GLfloat ambient_lightModel[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient_lightModel );

	stProperties props = LoadProperties();
	srand(static_cast<unsigned int>(time(nullptr)));
	scene = new CScene(props);
}

static void CloseGL(HWND hWnd, HDC hDC, HGLRC hRC)
{
	delete scene;
	scene = nullptr;
	wglMakeCurrent( NULL, NULL );
	wglDeleteContext( hRC );
	ReleaseDC( hWnd, hDC );
}


// Screen Saver Procedure
extern "C" LRESULT WINAPI ScreenSaverProc(HWND hWnd, UINT message, 
                               WPARAM wParam, LPARAM lParam)
{
  static HDC hDC;
  static HGLRC hRC;
  static RECT rect;

  switch ( message ) {

  case WM_CREATE: 
    // get window dimensions
    GetClientRect( hWnd, &rect );
    Width = rect.right;         
    Height = rect.bottom;
    g_startTick = GetTickCount();
    GetCursorPos(&g_initialCursorPos);
    
    //get configuration from registry if applicable

    //set up OpenGL
    InitGL( hWnd, hDC, hRC );

    //Initialize perspective, viewpoint, and
    //any objects you wish to animate

    //create a timer that ticks every 10 milliseconds

	SetTimer( hWnd, TIMER_1, 10, NULL ); 
    return 0;

  case WM_MOUSEMOVE:
	  {
		  POINT cursorPos = {};
		  GetCursorPos(&cursorPos);

		  const DWORD elapsed = GetTickCount() - g_startTick;
		  const LONG deltaX = cursorPos.x - g_initialCursorPos.x;
		  const LONG deltaY = cursorPos.y - g_initialCursorPos.y;

		  if (elapsed < 1000 || (abs(deltaX) <= 2 && abs(deltaY) <= 2))
			  return 0;

		  return DefScreenSaverProc(hWnd, message, wParam, lParam);
	  }

  case WM_ACTIVATEAPP:
	  if (!wParam && (GetTickCount() - g_startTick) < 1000)
		  return 0;
	  break;

  case WM_ACTIVATE:
	  if (LOWORD(wParam) == WA_INACTIVE && (GetTickCount() - g_startTick) < 1000)
		  return 0;
	  break;

  case WM_NCACTIVATE:
	  if (!wParam && (GetTickCount() - g_startTick) < 1000)
		  return TRUE;
	  break;

  case WM_ERASEBKGND:
	  return 1;
 
  case WM_DESTROY:
	  KillTimer( hWnd, TIMER_1 );
    
	  //delete any objects created during animation
	  //and close down OpenGL nicely

	  CloseGL( hWnd, hDC, hRC );
	  return 0;

  case WM_TIMER:
	  {
		  switch(wParam)
		  {
		  case TIMER_1:
			  scene->Render(hDC);
			  break;
		  }
		  return 0;
	  }
  }

  return DefScreenSaverProc(hWnd, message, wParam, lParam );
}

 
extern "C" BOOL WINAPI ScreenSaverConfigureDialog(HWND hDlg, UINT message,WPARAM wParam, LPARAM lParam)
{


  switch ( message ) 
  {

        case WM_INITDIALOG:

                //get configuration from the registry

                return TRUE;

       case WM_COMMAND:
           switch( LOWORD( wParam ) ) 
                { 

                case IDOK:
					EndDialog( hDlg, LOWORD( wParam ) == IDOK ); 
                    return TRUE; 

                case IDCANCEL: 
                    EndDialog( hDlg, LOWORD( wParam ) == IDOK ); 
                    return TRUE;   
                }

  }     //end command switch

  return FALSE; 
}

extern "C" BOOL WINAPI RegisterDialogClasses(HANDLE hInst)
{
  return TRUE;
}
