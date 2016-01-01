

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

#pragma comment (lib,"cv.lib")
#pragma comment (lib,"cxcore.lib")
#pragma comment (lib,"highgui.lib")
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
void DrawObjects( int i );				//히스토그램을 그리는 함수
void Histclac( void );						//이미지 파일을 받아서 히스토그램의 가중치를 받아오는 함수
void initHist( void );

/*
IplImage* src;
IplImage* hsv;
IplImage* h_plane;
IplImage* s_plane;
IplImage* v_plane;
*/

float lightPos[3] = { -10.0f, 10.0f, 10.0f };
float pi = 3.1415926535;

int h_bins = 30, s_bins = 32;			//빈의 초기 설정
int **binsre;							//빈을 담을 2차원 배열

float width = 30.0f, height = 30.0f;	//고정된 가로 세로의 값
float dotw = 0.0f, doth = 0.0f;			//고정된 크기의 가로 세로를 갖기 위하여 빈의 값으로 분할하기 위한 값


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
	dotw = width/h_bins;
	doth = height/s_bins;


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

	if(GetAsyncKeyState( VK_DOWN ) && 0x8000)			//h 값의 변화
	{
		if(h_bins < 50)
		{
			h_bins++;
			dotw = width/h_bins;
		}
	}
	if(GetAsyncKeyState( VK_UP) && 0x8000)
	{
		if(h_bins > 10)
		{
			h_bins--;	
			dotw = width/h_bins;
		}
	}
	if(GetAsyncKeyState( VK_RIGHT ) && 0x8000)			//s 값의 변화
	{
		if(s_bins < 50)
		{
			s_bins++;
			doth = height/s_bins;
		}
	}
	if(GetAsyncKeyState( VK_LEFT) && 0x8000)
	{
		if(s_bins > 10)
		{
			s_bins--;
			doth = height/s_bins;
		}
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

	gluLookAt(0, 15, 25, 0, 0, 0, 0, 1, 0 );

	glMultMatrixd( trball.RotationMatrix);

	Histclac();

	DrawObjects(0);


	SwapBuffers(MyDC);


	return;
}


void DrawObjects( int i )
{
	glTranslatef(-15.0f, 0.0f, -16.0f);
	glPushMatrix();
	glColor3f(0.0f, 1.0f, 0.0f);
	glTranslatef(i*1.0f, 0.0f, 0.0f);
	glBegin(GL_LINES);
	glVertex3f(0.0f,0.0f,0.0f);
	glVertex3f(30.0f,0.0f,0.0f);
	glVertex3f(0.0f,0.0f,0.0f);
	glVertex3f(0.0f,30.0f,0.0f);
	glVertex3f(0.0f,0.0f,0.0f);
	glVertex3f(0.0f,0.0f,30.0f);

	glEnd();

	glPopMatrix();

	float clr, high;

	for( int h = 0; h < h_bins; h++)
	{
		for( int s = 0; s < s_bins; s++)
		{		
			clr = binsre[h][s]/255.0f;
			high = binsre[h][s]/255.0f*15;
			glPushMatrix();
			if(clr == 0.0f)
				clr = 1.0f;
			glColor3f(clr, clr, clr);
			glTranslatef(dotw/2, 0.0f, doth/2);
			glTranslatef(h*dotw, high/2, s*doth);
			glScalef(1*dotw,high,1*doth);
			glutSolidCube(1.0);
			glColor3f(0.0f, 0.0f, 0.0f);
			glutWireCube(1.0);
			glPopMatrix();
		}
	}

	for(int m = 0; m < h_bins; m++)
		free(binsre[m]);
	free(binsre);

	return;
}

void Histclac( void )
{
	IplImage* src = cvLoadImage("석양.jpg");

	IplImage* hsv = cvCreateImage( cvGetSize(src), 8, 3);
	cvCvtColor(src, hsv, CV_BGR2HSV);

	IplImage* h_plane = cvCreateImage( cvGetSize(src), 8, 1);
	IplImage* s_plane = cvCreateImage( cvGetSize(src), 8, 1);
	IplImage* v_plane = cvCreateImage( cvGetSize(src), 8, 1);

	IplImage* planes[] = { h_plane, s_plane };
	cvCvtPixToPlane( hsv, h_plane, s_plane, v_plane, 0);

	CvHistogram* hist;
	{
		int hist_size[] = { h_bins, s_bins };
		float h_ranges[] = { 0, 180 };
		float s_ranges[] = { 0, 255 };
		float* ranges[] = {h_ranges, s_ranges};

		hist = cvCreateHist( 2, hist_size, CV_HIST_ARRAY, ranges, 1 );
	}

	cvCalcHist( planes, hist, 0, 0);


	float max_value = 0;

	binsre = (int **)calloc(h_bins, sizeof(int));		//2차원 배열 동적 할당
	for(int m = 0; m < h_bins; m++)
	{
		binsre[m] = (int *)calloc(s_bins, sizeof(int));
	}

	cvGetMinMaxHistValue( hist, 0, &max_value, 0, 0);	//최고 값을 구합니다	

	for( int h = 0; h < h_bins; h++)
	{
		for( int s = 0; s < s_bins; s++)
		{
			float bin_val = cvQueryHistValue_2D( hist, h, s);		//2차원으로 된 히스토그램에서 값을 꺼냄
			//printf("bin_val = %f\n", bin_val);
			int intensity = cvRound( bin_val * 255/ max_value );	//꺼낸 값을 8비트로 변환
			//printf("intensity = %d\n", intensity);
			binsre[h][s] = intensity;								//그것을 꺼내어 배열에 적재
		}
	}
	cvReleaseImage(&src);
	cvReleaseImage(&hsv);
	cvReleaseImage(&h_plane);
	cvReleaseImage(&s_plane);
	cvReleaseImage(&v_plane);
	cvReleaseHist(&hist);
}
