/*------------------------------------------------------------------------
 * A demonstration of OpenGL in a  ARGB window 
 *    => support for composited window transparency
 *
 * (c) 2011 by Wolfgang 'datenwolf' Draxinger
 *     See me at comp.graphics.api.opengl and StackOverflow.com
  
 * License agreement: This source code is provided "as is". You
 * can use this source code however you want for your own personal
 * use. If you give this source code to anybody else then you must
 * leave this message in it.
 *
 * This program is based on the simplest possible 
 * Linux OpenGL program by FTB (see info below)
 
  The simplest possible Linux OpenGL program? Maybe...

  (c) 2002 by FTB. See me in comp.graphics.api.opengl

  --
  <\___/>
  / O O \
  \_____/  FTB.

------------------------------------------------------------------------*/
#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <sys/types.h>
#include <time.h>

#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glxext.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xrender.h>
#include <X11/Xutil.h>

#define USE_CHOOSE_FBCONFIG

static void fatalError(const char *why)
{
	fprintf(stderr, "%s", why);
	exit(0x666);
}

static int Xscreen;
static Atom del_atom;
static Colormap cmap;
static Display *Xdisplay;
static XVisualInfo *visual;
static XRenderPictFormat *pict_format;
static GLXFBConfig *fbconfigs, fbconfig;
static int numfbconfigs;
static GLXContext render_context;
static Window Xroot, window_handle;
static GLXWindow glX_window_handle;
static int width, height;

static int VisData[] = {
GLX_RENDER_TYPE, GLX_RGBA_BIT,
GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
GLX_DOUBLEBUFFER, True,
GLX_RED_SIZE, 8,
GLX_GREEN_SIZE, 8,
GLX_BLUE_SIZE, 8,
GLX_ALPHA_SIZE, 8,
GLX_DEPTH_SIZE, 16,
None
};

static int isExtensionSupported(const char *extList, const char *extension)
{
 
  const char *start;
  const char *where, *terminator;
 
  /* Extension names should not have spaces. */
  where = strchr(extension, ' ');
  if ( where || *extension == '\0' )
    return 0;
 
  /* It takes a bit of care to be fool-proof about parsing the
     OpenGL extensions string. Don't be fooled by sub-strings,
     etc. */
  for ( start = extList; ; ) {
    where = strstr( start, extension );
 
    if ( !where )
      break;
 
    terminator = where + strlen( extension );
 
    if ( where == start || *(where - 1) == ' ' )
      if ( *terminator == ' ' || *terminator == '\0' )
        return 1;
 
    start = terminator;
  }
  return 0;
}

static Bool WaitForMapNotify(Display *d, XEvent *e, char *arg)
{
	return d && e && arg && (e->type == MapNotify) && (e->xmap.window == *(Window*)arg);
}

static void describe_fbconfig(GLXFBConfig fbconfig)
{
	int doublebuffer;
	int red_bits, green_bits, blue_bits, alpha_bits, depth_bits;

	glXGetFBConfigAttrib(Xdisplay, fbconfig, GLX_DOUBLEBUFFER, &doublebuffer);
	glXGetFBConfigAttrib(Xdisplay, fbconfig, GLX_RED_SIZE, &red_bits);
	glXGetFBConfigAttrib(Xdisplay, fbconfig, GLX_GREEN_SIZE, &green_bits);
	glXGetFBConfigAttrib(Xdisplay, fbconfig, GLX_BLUE_SIZE, &blue_bits);
	glXGetFBConfigAttrib(Xdisplay, fbconfig, GLX_ALPHA_SIZE, &alpha_bits);
	glXGetFBConfigAttrib(Xdisplay, fbconfig, GLX_DEPTH_SIZE, &depth_bits);

	fprintf(stderr, "FBConfig selected:\n"
		"Doublebuffer: %s\n"
		"Red Bits: %d, Green Bits: %d, Blue Bits: %d, Alpha Bits: %d, Depth Bits: %d\n",
		doublebuffer == True ? "Yes" : "No", 
		red_bits, green_bits, blue_bits, alpha_bits, depth_bits);
}

static void createTheWindow()
{
	XEvent event;
	int x,y, attr_mask;
	XSizeHints hints;
	XWMHints *startup_state;
	XTextProperty textprop;
	XSetWindowAttributes attr = {0,};
	static char *title = "FTB's little OpenGL example - ARGB extension by WXD";

	Xdisplay = XOpenDisplay(NULL);
	if (!Xdisplay) {
		fatalError("Couldn't connect to X server\n");
	}
	Xscreen = DefaultScreen(Xdisplay);
	Xroot = RootWindow(Xdisplay, Xscreen);

	fbconfigs = glXChooseFBConfig(Xdisplay, Xscreen, VisData, &numfbconfigs);
	fbconfig = 0;
	for(int i = 0; i<numfbconfigs; i++) {
		visual = (XVisualInfo*) glXGetVisualFromFBConfig(Xdisplay, fbconfigs[i]);
		if(!visual)
			continue;

		pict_format = XRenderFindVisualFormat(Xdisplay, visual->visual);
		if(!pict_format)
			continue;

		fbconfig = fbconfigs[i];
		if(pict_format->direct.alphaMask > 0) {
			break;
		}
	}

	if(!fbconfig) {
		fatalError("No matching FB config found");
	}

	describe_fbconfig(fbconfig);

	/* Create a colormap - only needed on some X clients, eg. IRIX */
	cmap = XCreateColormap(Xdisplay, Xroot, visual->visual, AllocNone);

	attr.colormap = cmap;
	attr.background_pixmap = None;
	attr.border_pixmap = None;
	attr.border_pixel = 0;
	attr.event_mask =
		StructureNotifyMask |
		EnterWindowMask |
		LeaveWindowMask |
		ExposureMask |
		ButtonPressMask |
		ButtonReleaseMask |
		OwnerGrabButtonMask |
		KeyPressMask |
		KeyReleaseMask;

	attr_mask = 
	//	CWBackPixmap|
		CWColormap|
		CWBorderPixel|
		CWEventMask;

	width = DisplayWidth(Xdisplay, DefaultScreen(Xdisplay))/2;
	height = DisplayHeight(Xdisplay, DefaultScreen(Xdisplay))/2;
	x=width/2, y=height/2;

	window_handle = XCreateWindow(	Xdisplay,
					Xroot,
					x, y, width, height,
					0,
					visual->depth,
					InputOutput,
					visual->visual,
					attr_mask, &attr);

	if( !window_handle ) {
		fatalError("Couldn't create the window\n");
	}

#if USE_GLX_CREATE_WINDOW
	fputs("glXCreateWindow ", stderr);
	int glXattr[] = { None };
	glX_window_handle = glXCreateWindow(Xdisplay, fbconfig, window_handle, glXattr);
	if( !glX_window_handle ) {
		fatalError("Couldn't create the GLX window\n");
	}
#else
	glX_window_handle = window_handle;
#endif

	textprop.value = (unsigned char*)title;
	textprop.encoding = XA_STRING;
	textprop.format = 8;
	textprop.nitems = strlen(title);

	hints.x = x;
	hints.y = y;
	hints.width = width;
	hints.height = height;
	hints.flags = USPosition|USSize;

	startup_state = XAllocWMHints();
	startup_state->initial_state = NormalState;
	startup_state->flags = StateHint;

	XSetWMProperties(Xdisplay, window_handle,&textprop, &textprop,
			NULL, 0,
			&hints,
			startup_state,
			NULL);

	XFree(startup_state);

	XMapWindow(Xdisplay, window_handle);
	XIfEvent(Xdisplay, &event, WaitForMapNotify, (char*)&window_handle);

	if ((del_atom = XInternAtom(Xdisplay, "WM_DELETE_WINDOW", 0)) != None) {
		XSetWMProtocols(Xdisplay, window_handle, &del_atom, 1);
	}
}

static int ctxErrorHandler( Display *dpy, XErrorEvent *ev )
{
    fputs("Error at context creation", stderr);
    return 0;
}

static void createTheRenderContext()
{
	int dummy;
	if (!glXQueryExtension(Xdisplay, &dummy, &dummy)) {
		fatalError("OpenGL not supported by X server\n");
	}

#if USE_GLX_CREATE_CONTEXT_ATTRIB
	#define GLX_CONTEXT_MAJOR_VERSION_ARB       0x2091
	#define GLX_CONTEXT_MINOR_VERSION_ARB       0x2092
	render_context = NULL;
	if( isExtensionSupported( glXQueryExtensionsString(Xdisplay, DefaultScreen(Xdisplay)), "GLX_ARB_create_context" ) ) {
		typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
		glXCreateContextAttribsARBProc glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)glXGetProcAddressARB( (const GLubyte *) "glXCreateContextAttribsARB" );
		if( glXCreateContextAttribsARB ) {
			int context_attribs[] =
			{
				GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
				GLX_CONTEXT_MINOR_VERSION_ARB, 0,
				//GLX_CONTEXT_FLAGS_ARB        , GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
				None
			};

			int (*oldHandler)(Display*, XErrorEvent*) = XSetErrorHandler(&ctxErrorHandler);
			
			render_context = glXCreateContextAttribsARB( Xdisplay, fbconfig, 0, True, context_attribs );

			XSync( Xdisplay, False );
			XSetErrorHandler( oldHandler );

			fputs("glXCreateContextAttribsARB failed", stderr);
		} else {
			fputs("glXCreateContextAttribsARB could not be retrieved", stderr);
		}
	} else {
			fputs("glXCreateContextAttribsARB not supported", stderr);
	}

	if(!render_context)
	{
#else
	{
#endif
		render_context = glXCreateNewContext(Xdisplay, fbconfig, GLX_RGBA_TYPE, 0, True);
		if (!render_context) {
			fatalError("Failed to create a GL context\n");
		}
	}

	if (!glXMakeContextCurrent(Xdisplay, glX_window_handle, glX_window_handle, render_context)) {
		fatalError("glXMakeCurrent failed for window\n");
	}
}

static int updateTheMessageQueue()
{
	XEvent event;
	XConfigureEvent *xc;

	while (XPending(Xdisplay))
	{
		XNextEvent(Xdisplay, &event);
		switch (event.type)
		{
		case ClientMessage:
			if (event.xclient.data.l[0] == del_atom)
			{
				return 0;
			}
		break;

		case ConfigureNotify:
			xc = &(event.xconfigure);
			width = xc->width;
			height = xc->height;
			break;
		}
	}
	return 1;
}

/*  6----7
   /|   /|
  3----2 |
  | 5--|-4
  |/   |/
  0----1

*/

GLfloat cube_vertices[][8] =  {
	/*  X     Y     Z   Nx   Ny   Nz    S    T */
	{-1.0, -1.0,  1.0, 0.0, 0.0, 1.0, 0.0, 0.0}, // 0
	{ 1.0, -1.0,  1.0, 0.0, 0.0, 1.0, 1.0, 0.0}, // 1
	{ 1.0,  1.0,  1.0, 0.0, 0.0, 1.0, 1.0, 1.0}, // 2
	{-1.0,  1.0,  1.0, 0.0, 0.0, 1.0, 0.0, 1.0}, // 3

	{ 1.0, -1.0, -1.0, 0.0, 0.0, -1.0, 0.0, 0.0}, // 4
	{-1.0, -1.0, -1.0, 0.0, 0.0, -1.0, 1.0, 0.0}, // 5
	{-1.0,  1.0, -1.0, 0.0, 0.0, -1.0, 1.0, 1.0}, // 6
	{ 1.0,  1.0, -1.0, 0.0, 0.0, -1.0, 0.0, 1.0}, // 7

	{-1.0, -1.0, -1.0, -1.0, 0.0, 0.0, 0.0, 0.0}, // 5
	{-1.0, -1.0,  1.0, -1.0, 0.0, 0.0, 1.0, 0.0}, // 0
	{-1.0,  1.0,  1.0, -1.0, 0.0, 0.0, 1.0, 1.0}, // 3
	{-1.0,  1.0, -1.0, -1.0, 0.0, 0.0, 0.0, 1.0}, // 6
	
	{ 1.0, -1.0,  1.0,  1.0, 0.0, 0.0, 0.0, 0.0}, // 1
	{ 1.0, -1.0, -1.0,  1.0, 0.0, 0.0, 1.0, 0.0}, // 4
	{ 1.0,  1.0, -1.0,  1.0, 0.0, 0.0, 1.0, 1.0}, // 7
	{ 1.0,  1.0,  1.0,  1.0, 0.0, 0.0, 0.0, 1.0}, // 2
	
	{-1.0, -1.0, -1.0,  0.0, -1.0, 0.0, 0.0, 0.0}, // 5
	{ 1.0, -1.0, -1.0,  0.0, -1.0, 0.0, 1.0, 0.0}, // 4
	{ 1.0, -1.0,  1.0,  0.0, -1.0, 0.0, 1.0, 1.0}, // 1
	{-1.0, -1.0,  1.0,  0.0, -1.0, 0.0, 0.0, 1.0}, // 0

	{-1.0, 1.0,  1.0,  0.0,  1.0, 0.0, 0.0, 0.0}, // 3
	{ 1.0, 1.0,  1.0,  0.0,  1.0, 0.0, 1.0, 0.0}, // 2
	{ 1.0, 1.0, -1.0,  0.0,  1.0, 0.0, 1.0, 1.0}, // 7
	{-1.0, 1.0, -1.0,  0.0,  1.0, 0.0, 0.0, 1.0}, // 6
};

static void draw_cube(void)
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glVertexPointer(3, GL_FLOAT, sizeof(GLfloat) * 8, &cube_vertices[0][0]);
	glNormalPointer(GL_FLOAT, sizeof(GLfloat) * 8, &cube_vertices[0][3]);
	glTexCoordPointer(2, GL_FLOAT, sizeof(GLfloat) * 8, &cube_vertices[0][6]);

	glDrawArrays(GL_QUADS, 0, 24);
}

float const light0_dir[]={0,1,0,0};
float const light0_color[]={78./255., 80./255., 184./255.,1};

float const light1_dir[]={-1,1,1,0};
float const light1_color[]={255./255., 220./255., 97./255.,1};

float const light2_dir[]={0,-1,0,0};
float const light2_color[]={31./255., 75./255., 16./255.,1};

static void redrawTheWindow()
{
	float const aspect = (float)width / (float)height;

	static float a=0;
	static float b=0;
	static float c=0;

	glDrawBuffer(GL_BACK);

	glViewport(0, 0, width, height);

	// Clear with alpha = 0.0, i.e. full transparency
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-aspect, aspect, -1, 1, 2.5, 10);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

#if 0
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif

	glLightfv(GL_LIGHT0, GL_POSITION, light0_dir);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light0_color);

	glLightfv(GL_LIGHT1, GL_POSITION, light1_dir);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light1_color);

	glLightfv(GL_LIGHT2, GL_POSITION, light2_dir);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, light2_color);

	glTranslatef(0., 0., -5.);

	glRotatef(a, 1, 0, 0);
	glRotatef(b, 0, 1, 0);
	glRotatef(c, 0, 0, 1);

	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHTING);

	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	glColor4f(1., 1., 1., 0.5);

	glCullFace(GL_FRONT);
	draw_cube();
	glCullFace(GL_BACK);
	draw_cube();

	a = fmod(a+0.1, 360.);
	b = fmod(b+0.5, 360.);
	c = fmod(c+0.25, 360.);

	struct timespec Ta, Tb;

	clock_gettime(CLOCK_MONOTONIC_RAW, &Ta);
 	glXSwapBuffers(Xdisplay, glX_window_handle);
	clock_gettime(CLOCK_MONOTONIC_RAW, &Tb);
	
	// fprintf(stderr, "glXSwapBuffers returned after %f ms\n", 1e3*((double)Tb.tv_sec + 1e-6*(double)Tb.tv_nsec) - 1e3*((double)Ta.tv_sec + 1e-6*(double)Ta.tv_nsec));	
}

int main(int argc, char *argv[])
{
	createTheWindow();
	createTheRenderContext();

	while (updateTheMessageQueue()) {
		redrawTheWindow();
	}

	return 0;
}

