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

//#pragma comment (lib,"cv.lib")
//#pragma comment (lib,"cxcore.lib")
//#pragma comment (lib,"highgui.lib")
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
float pi = 3.1415926535;
float radian = 57.2957795;
float x = 1.0f;

int h_bins = 360, s_bins = 16, v_bins = 16;
int binsre[360][16][16] = {0};


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

	gluLookAt(35, 30, 0, 0, 0, 0, 0, 1, 0 );

	glMultMatrixd( trball.RotationMatrix);

	DrawObjects(0);


	SwapBuffers(MyDC);


	return;
}

void DrawObjects( int i )
{

	glutBitmapCharacter(GLUT_BITMAP_8_BY_13, 5);
	glPushMatrix();
		glTranslatef(0, 16, 0);
	glColor3f(0.0f, 1.0f, 0.0f);
	for(int j = 1; j <= 16; j++)
	{
		glBegin( GL_LINE_LOOP );
		for(int i = 0; i < 360; i++)
		{
			glVertex3f(j*cos(i/radian), 0, j*sin(i/radian) );
		}
		glEnd();
	}

	glBegin( GL_LINES );
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0,0,0);
	glVertex3f(0,-16,0);
	glVertex3f(0,0,0);
	glVertex3f(0,0,16);
	glVertex3f(0,0,16);
	glVertex3f(0,-16,16);
	glVertex3f(0,-16,16);
	glVertex3f(0,-16,0);
	glEnd();

	glBegin( GL_LINE_LOOP );
	for(int i = 0; i < 360; i++)
	{
		glVertex3f(16*cos(i/radian), -16, 16*sin(i/radian) );
	}
	glEnd();

	glPopMatrix();
	

	double degree = 360.0/h_bins;
	float clr, high;
	float k, m_cr, m_cg, m_cb, sr, sg, sb, m_red, m_green, m_blue;
	int inte = (int)h_bins/6;

	for( int h = 0; h < h_bins; h++)
	{
		for( int s = 0; s < s_bins; s++)
		{
			for( int v = 0; v < v_bins; v++)
			{
				clr = binsre[h][s][v]/255.0f;
				high = binsre[h][s][v]/255.0f*1.5+0.5;

				 k=(h%60)/(float)60;

 

					if((h>=0 && h<=60 )||( h>=300 && h<=360))   // Red 값이 255로 고정된 부분(Magenta~Yellow)

						m_cr=1.0;

					else if(h>60 && h<120)  // Red 값이 감소하는 위치 (Yellow ~ Green)

						m_cr=1.0-k;

					else if(h>240 && h<300) // Red 값이 상승하는 위치 (Blue ~ Magenta)

						m_cr=k;

					else

						m_cr=0;

				 

					if(h>=60 && h<=180)      // Green 값이 고장되어 있는 위치 (Yellow ~ Cyan)

						m_cg=1.0;

					else if(h>180 && h<240) // Green값이 감소하는 위치 (Cyan ~ Blue)

						m_cg=1.0-k;

					else if(h>0 && h<60)     // Green값이 상승하는 위치 (Red ~ Yellow)

						m_cg=k;

					else

						m_cg=0;

				 

					if(h>=180 && h<=300 )   // Blue 값이 고정되어 있는 위치 (Cyan ~ Magenta)

						m_cb=1.0;

					else if(h>300 && h<360) // Blue 값이 감소하는 위치 (Magenta ~ Red)

						m_cb=1.0-k;

					else if(h>120 && h<180) // Blue 값이 상승하는 위치 (Green ~ Cyan)

						m_cb=k;

					else

						m_cb=0;

				 
				 

					sr=m_cr+(1.0-m_cr)*(16-s)/16.0;  // Saturation 값을 받아서 색상에 적용한다.

					sg=m_cg+(1.0-m_cg)*(16-s)/16.00;

					sb=m_cb+(1.0-m_cb)*(16-s)/16.00;


				 

					m_red=sr*v/16;     // bright 값을 받아서 색상에 적용한다.

					m_green=sg*v/16;

					m_blue=sb*v/16;
				

				glPushMatrix();
				glTranslatef(s*cos((degree*h)/radian), v, s*sin((degree*h)/radian));
					if(clr == 0.0f)
					{
						clr = 1.0f;
					}
					glColor3f(m_red, m_green, m_blue);
					
					glScalef(high,high,high);
					if(clr != 1.0f)
					{
						glutSolidSphere(high,20,10);
						//glColor3f(0.0f, 0.0f, 0.0f);
						//glutWireSphere(high,20,10);
					}

					
				glPopMatrix();
			}
		}
	}



	
	
	return;
}

void hist( void )
{
	
	IplImage* src = cvLoadImage("DSCN0481.JPG");

	
	IplImage* hsv = cvCreateImage( cvGetSize(src), 8, 3);

	IplImage* h_plane = cvCreateImage( cvGetSize(src), 8, 1);
	IplImage* s_plane = cvCreateImage( cvGetSize(src), 8, 1);
	IplImage* v_plane = cvCreateImage( cvGetSize(src), 8, 1);

	
	IplImage* planes[] = { h_plane, v_plane, s_plane };
	cvCvtColor(src, hsv, CV_BGR2HSV);
	cvCvtPixToPlane( hsv, h_plane, v_plane, s_plane, 0);

	CvHistogram* hist;

	{
		int hist_size[] = { h_bins, s_bins, v_bins };
		float h_ranges[] = { 0, 180 };
		float s_ranges[] = { 0, 255 };
		float v_ranges[] = { 0, 255 };
		float* ranges[] = {h_ranges, s_ranges, v_ranges};

		hist = cvCreateHist( 3, hist_size, CV_HIST_ARRAY, ranges, 1 );
	}

	cvCalcHist( planes, hist, 0, 0);


	float max_value = 0;
	float min_value = 0;

	cvGetMinMaxHistValue( hist, &min_value, &max_value, 0, 0);
	
	for( int h = 0; h < h_bins; h++)
	{
		for( int s = 0; s < s_bins; s++)
		{
			for( int v = 0; v < v_bins; v++)
			{
				float bin_val = cvQueryHistValue_3D( hist, h, s, v);
				//printf("bin_val = %f\n", bin_val);
				int intensity = cvRound( bin_val * 255/ max_value );
				//printf("intensity = %d\n", intensity);
				binsre[h][s][v] = intensity;
			}
		}
	}
	
}
