//////////////////////////////////////////////////////////////////////////
//
// main.cpp
// 
// 2010. 3. 12.
// Created by Sun-Jeong Kim
// 
// Description
// - create a window
// - use OpenGL library
// - be useful for animations
//
//////////////////////////////////////////////////////////////////////////


#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include "glut.h"
#include <math.h>
#include <cv.h>
#include <highgui.h>

#include "trackball.h"


MyTrackBall trball;
bool bLButtonDown = false;

// global variables
char *szClassName = "Computer Graphics";

HWND MyMWindowHandle = 0;

HDC MyDC;
HGLRC MyRC;

LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );

bool bSetupPixelFormat( void );
void Resize( const int cx, const int cy );
void InitScene( void );
void FinishScene( void );
void UpdateScene( void );
void DrawScene( void );
void DrawObjects( int i );
void hist( void );


float lightPos[3] = { -10.0f, 10.0f, 10.0f };
float x = 1.0f;	
float pi = 3.1415926535;

int r_bins = 16, g_bins = 16, b_bins = 16;
int binsre[16][16][16] = {0};


#include <sys/timeb.h>
timeb startTime, endTime;

GLUquadricObj *pObj = NULL;



/*
 *	WinMain
 */
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE, LPSTR, int nShowCmd )
{
 	// Registers the window class
	WNDCLASS wc;
	
	wc.style	 = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc	 = (WNDPROC)WndProc;
	wc.cbClsExtra	 = 0;
	wc.cbWndExtra	 = 0;
	wc.hInstance	 = hInstance;
	wc.hIcon	 = LoadIcon( hInstance, IDI_APPLICATION );
	wc.hCursor	 = LoadCursor( 0, IDC_ARROW );
	wc.hbrBackground = (HBRUSH)GetStockObject( WHITE_BRUSH );
	wc.lpszMenuName	 = NULL;
	wc.lpszClassName = szClassName;

	RegisterClass(&wc);

	// Create the main window
	MyMWindowHandle = CreateWindow( szClassName, 
					"Simple OpenGL Example", 
					WS_OVERLAPPEDWINDOW, 
					CW_USEDEFAULT, 
					CW_USEDEFAULT, 
					CW_USEDEFAULT, 
					CW_USEDEFAULT, 
					0, 
					0, 
					hInstance, 
					0 );

	// Make sure that window was created 
	if( !MyMWindowHandle )
		return false;
	
	ShowWindow( MyMWindowHandle, nShowCmd );
	UpdateWindow( MyMWindowHandle );

	// Main message loop
	MSG msg;
	msg.message = WM_NULL;
	while( msg.message != WM_QUIT )
	{
		// If there are Window messages then process them.
		if( PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		// Otherwise, do animation/game stuff.
		else
		{	
			UpdateScene();
			DrawScene();
		}
	}

	UnregisterClass( szClassName, wc.hInstance );
	
	return (int)msg.wParam;
}


/*
 * WndProc: to process messages for the main window
 */
LRESULT CALLBACK WndProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	RECT rect;

	switch( msg ) 
	{
	case WM_CREATE: 
		// Initialize for the OpenGL rendering
		MyDC = GetDC( hWnd ); 
		if( !bSetupPixelFormat() ) 
			DestroyWindow( MyMWindowHandle ); 

		MyRC = wglCreateContext( MyDC ); 
		wglMakeCurrent( MyDC, MyRC ); 

		InitScene();

		break; 
	
	case WM_SIZE: 
		GetClientRect( hWnd, &rect ); 
		Resize( rect.right, rect.bottom ); 
		
		break; 

	case WM_DESTROY:
		// Destroy all about OpenGL
		if( MyRC ) 
			wglDeleteContext( MyRC ); 
		
		if( MyDC ) 
			ReleaseDC( hWnd, MyDC ); 
		
		FinishScene();

		PostQuitMessage( 0 );
		break;
	
	case WM_KEYDOWN:
		if( wParam == VK_ESCAPE ) {
			if( IDYES == MessageBox( NULL, "Are you sure to exit?", "Exit", MB_YESNO ) )
				DestroyWindow( MyMWindowHandle );
		}
		break;
	case WM_LBUTTONDOWN:
		if(!bLButtonDown)
		{
			bLButtonDown = true;
			trball.Start((int)LOWORD(lParam ), (int)HIWORD(lParam ));
		}
		break;

	case WM_LBUTTONUP:
		bLButtonDown = false;
		break;
	case WM_MOUSEMOVE:
		if(bLButtonDown)
		{
			trball.End((int)LOWORD(lParam ), (int)HIWORD(lParam ));
		}
		break;
	
	default:
		return DefWindowProc( hWnd, msg, wParam, lParam );
	}

	return 0;
}

/*
 * bSetupPixelFormat : to setup the format of pixel for OpenGL
 */
bool bSetupPixelFormat( void ) 
{ 
    PIXELFORMATDESCRIPTOR pfd; 
	int pixelformat; 

	pfd.nSize = sizeof( PIXELFORMATDESCRIPTOR ); 
	pfd.nVersion = 1; 
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER; 
	pfd.dwLayerMask = PFD_MAIN_PLANE; 
	pfd.iPixelType = PFD_TYPE_RGBA; 
	pfd.cColorBits = 24;
	pfd.cDepthBits = 16; 
	pfd.cAccumBits = 0; 
	pfd.cStencilBits = 0;

	if( (pixelformat = ChoosePixelFormat( MyDC, &pfd )) == 0 ) { 
		MessageBox( NULL, "ChoosePixelFormat() failed!!!", "Error", MB_OK|MB_ICONERROR ); 
		return false; 
	} 

	if( SetPixelFormat( MyDC, pixelformat, &pfd ) == false ) { 
		MessageBox( NULL, "SetPixelFormat() failed!!!", "Error", MB_OK|MB_ICONERROR ); 
		return false; 
	}
	
	return true; 
}

/*
 * Resize : to re-initialize a scene
 */
void Resize( const int cx, const int cy ) 
{ 
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	glViewport( 0, 0, cx, cy );

	/*double length = 1.0;
	//double n = -1.0;
	double n = 1.5;
	double f = 2.0;

	// 3D orthographic viewing
	if( cx <= cy ) {
		double ratio = length * cy/(double)cx;
        //glOrtho( -length, length, -ratio, ratio, n, f );
		glFrustum( -length, length, -ratio, ratio, n, f );
	}
	else { 
		double ratio = length * cx/(double)cy;
        //glOrtho( -ratio, ratio, -length, length, n, f );
		glFrustum( -ratio, ratio, -length, length, n, f );
	}
	*/

	gluPerspective( 60.0, (double)cx/cy, 0.001, 200);

	glMatrixMode( GL_MODELVIEW );

	trball.Resize( cx, cy);

	return;
}

/*
 * InitScene : to initialize OpenGL rendering parameters
 */
void InitScene( void )
{

	hist();
	glEnable( GL_DEPTH_TEST );

	glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );

	ftime( &startTime );

	glPolygonOffset( 1.0f, 1.0f);
	glEnable( GL_POLYGON_OFFSET_FILL);
	/*glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	float light_pos[4]=	{1.0f, 0.0f, 0.0f, 1.0f};
	float light_ambient0[4]=	{0.2f, 0.2f, 0.2f, 1.0f};
	float light_diffuse0[4]=	{1.0f, 0.0f, 1.0f, 1.0f};
	float light_specular0[4]=	{1.0f, 1.0f, 1.0f, 1.0f};

	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse0);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular0);

	float mat_specular[4]=	{1.0f, 1.0f, 1.0f, 1.0f};

	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 100.0f);
	glShadeModel(GL_SMOOTH);
	*/

	trball.Initialize();

	return;
}

/*
 * FinishScene : to free memory
 */
void FinishScene( void )
{

	gluDeleteQuadric(pObj);
	
	return;
}

/*
 * UpdateScene: to update a scene
 */
void UpdateScene( void )
{
	ftime( &endTime );

	double elapsedTime = (double)(endTime.time-startTime.time) 
						+ (double)(endTime.millitm-startTime.millitm)/1000.0;
	
	startTime = endTime;

	if(GetAsyncKeyState( VK_DOWN ) && 0x8000)			//시간이 증가하거나 감소 함에 따라서 z의 값 변화
	{
		x -= elapsedTime;	
	}
	if(GetAsyncKeyState( VK_UP) && 0x8000)
	{
		x += elapsedTime;	
	}
	
	return;
}

/*
 * DrawScene : to draw a scene
 */
void DrawScene( void )
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	glLoadIdentity();

	gluLookAt(20, 10, 0, 0, 0, 0, 0, 1, 0 );

	glMultMatrixd( trball.RotationMatrix);

	DrawObjects(0);


	SwapBuffers(MyDC);


	return;
}

void DrawObjects( int i )
{
	
	glPushMatrix();
	glColor3f(0.0f, 1.0f, 0.0f);
	glTranslatef(i*1.0f, 0.0f, 0.0f);
	glutWireCube(16.0);
	glPopMatrix();

	
	glPushMatrix();
	glColor3f(1.0f, 0.0f, 0.0f);
	glTranslatef(5.5f, -5.5f, -5.5f);
	glutWireCube(5.0);
	glPopMatrix();
	

	for( int r = 0; r < r_bins; r++)
	{
		for( int g = 0; g < g_bins; g++)
		{
			for( int b = 0; b < b_bins; b++)
			{
				float clr, high;
				clr = binsre[r][g][b]/255.0f;
				high = (binsre[r][g][b]/255.0f)*1.5+0.5;
				glPushMatrix();
				glTranslatef(-7.5f, -7.5f, -7.5f);
				glTranslatef(r*1.0f, g*1.0f, b*1.0f);
					if(clr == 0.0f)
					{
						clr = 1.0f;
					}
					glColor3f(r/16.0f, g/16.0f, b/16.0f);
					
					glScalef(high,high,high);
					if(clr != 1.0f)
					{
						//glutSolidSphere(high,20,10); 
						glutSolidCube(high);
						glColor3f(0.0f, 0.0f, 0.0f);
						//glutWireSphere(high,20,10);
						glutWireCube(high);
					}
					
				glPopMatrix();
			}
		}
	}
	
	return;
}

void hist( void )
{
	IplImage* src = cvLoadImage("석양.jpg");

	IplImage* hsv = cvCreateImage( cvGetSize(src), 8, 3);

	IplImage* r_plane = cvCreateImage( cvGetSize(src), 8, 1);
	IplImage* g_plane = cvCreateImage( cvGetSize(src), 8, 1);
	IplImage* b_plane = cvCreateImage( cvGetSize(src), 8, 1);

	IplImage* planes[] = { r_plane, g_plane, b_plane };
	cvCvtPixToPlane( src, b_plane, g_plane, r_plane, 0);

	CvHistogram* hist;

	{
		int hist_size[] = { r_bins, g_bins, b_bins };
		float r_ranges[] = { 0, 255 };
		float g_ranges[] = { 0, 255 };
		float b_ranges[] = { 0, 255 };
		float* ranges[] = {r_ranges, g_ranges, b_ranges};

		hist = cvCreateHist( 3, hist_size, CV_HIST_ARRAY, ranges, 1 );
	}

	cvCalcHist( planes, hist, 0, 0);


	float max_value = 0;
	float min_value = 0;

	cvGetMinMaxHistValue( hist, &min_value, &max_value, 0, 0);
	
	for( int r = 0; r < r_bins; r++)
	{
		for( int g = 0; g < g_bins; g++)
		{
			for( int b = 0; b < b_bins; b++)
			{
				float bin_val = cvQueryHistValue_3D( hist, r, g, b);
				//printf("bin_val = %f\n", bin_val);
				int intensity = cvRound( bin_val * 255/ max_value );
				//printf("intensity = %d\n", intensity);
				binsre[r][g][b] = intensity;
			}
		}
	}
}
