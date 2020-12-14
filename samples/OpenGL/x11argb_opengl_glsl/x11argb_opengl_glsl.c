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

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>

#include <sys/types.h>
#include <time.h>

#include <GL/glew.h>
#include <GL/glx.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xrender.h>
#include <X11/Xutil.h>

#define USE_GLX_CREATE_WINDOW 1
#define USE_DOUBLEBUFFER 1

static const GLchar *vertex_shader_source =
"#version 120\n"
"void main()"
"{"
"   gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;"
"   gl_TexCoord[0] = gl_MultiTexCoord0;"
"   gl_FrontColor = gl_Color;"
"   gl_BackColor = gl_Color;"
"}\0";
GLuint shaderVertex = 0;

static const GLchar *fragment_shader_source = 
"#version 120\n"
"uniform sampler2D texCMYK;\n"
"uniform sampler2D texRGB;\n"
"uniform float T;\n"
"const float pi = 3.14159265;\n"
"void main()\n"
"{\n"
"   float ts = gl_TexCoord[0].s;\n"
"   vec2 mod_texcoord = gl_TexCoord[0].st*vec2(1., 2.) + vec2(0, -0.5 + 0.5*sin(T + 1.5*ts*pi));\n"
"   if( mod_texcoord.t < 0. || mod_texcoord.t > 1. ) { discard; }\n"
"   gl_FragColor = -texture2D(texCMYK, mod_texcoord) + texture2D(texRGB, gl_TexCoord[0].st);\n"
"   gl_FragColor.a = 1.;\n"
"}\n\0";
GLuint shaderFragment = 0;

GLuint shaderProgram = 0;

#define TEX_CMYK_WIDTH 2
#define TEX_CMYK_HEIGHT 2
GLubyte textureDataCMYK[TEX_CMYK_WIDTH * TEX_CMYK_HEIGHT][3] = {
    {0x00, 0xff, 0xff}, {0xff, 0x00, 0xff},
    {0xff, 0xff, 0x00}, {0x00, 0x00, 0x00}
};
GLuint texCMYK = 0;

#define TEX_RGB_WIDTH 2
#define TEX_RGB_HEIGHT 2
GLubyte textureDataRGB[TEX_RGB_WIDTH * TEX_RGB_HEIGHT][3] = {
    {0x00, 0x00, 0xff}, {0xff, 0xff, 0xff},
    {0xff, 0x00, 0x00}, {0x00, 0xff, 0x00}
};
GLuint texRGB = 0;

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

static void fatalError(const char *why)
{
	fprintf(stderr, "%s", why);
	exit(0x666);
}

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
#if USE_DOUBLEBUFFER
GLX_DOUBLEBUFFER, True,
#else
GLX_DOUBLEBUFFER, False,
#endif
GLX_RED_SIZE, 8,
GLX_GREEN_SIZE, 8,
GLX_BLUE_SIZE, 8,
GLX_ALPHA_SIZE, 8,
GLX_DEPTH_SIZE, 16,
None
};

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
		CWBackPixmap|
		CWColormap|
		CWBorderPixel|
		CWEventMask;

	width = DisplayWidth(Xdisplay, DefaultScreen(Xdisplay))/2;
	height = DisplayHeight(Xdisplay, DefaultScreen(Xdisplay))/2;
	int const dim = width < height ? width : height;

	window_handle = XCreateWindow(	Xdisplay,
					Xroot,
					0, 0, dim, dim,
					0,
					visual->depth,
					InputOutput,
					visual->visual,
					attr_mask, &attr);

	if( !window_handle ) {
		fatalError("Couldn't create the window\n");
	}

#if USE_GLX_CREATE_WINDOW
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

	hints.width = dim;
	hints.height = dim;
	hints.min_aspect.x = 1;
	hints.min_aspect.y = 1;
	hints.max_aspect.x = 1;
	hints.max_aspect.y = 1;

	hints.flags = USSize|PAspect;

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
				GLX_CONTEXT_FLAGS_ARB        , GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
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

	glewInit();
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

static int check_extensions(void)
{
    if( !GLEW_ARB_vertex_shader ||
        !GLEW_ARB_fragment_shader ) {
        fputs("Required OpenGL functionality not supported by system.\n", stderr);
        return 0;
    }

    return 1;
}

static int check_shader_compilation(GLuint shader)
{
    GLint n;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &n);
    if( n == GL_FALSE ) {
        GLchar *info_log;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &n);
        info_log = malloc(n);
        glGetShaderInfoLog(shader, n, &n, info_log);
        fprintf(stderr, "Shader compilation failed: %*s\n", n, info_log);
        free(info_log);
        return 0;
    }
    return 1;
}

static int init_resources(void)
{
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    glGenTextures(1, &texCMYK);
    glBindTexture(GL_TEXTURE_2D, texCMYK);
    glTexImage2D(GL_TEXTURE_2D, 0,  GL_RGB8, TEX_CMYK_WIDTH, TEX_CMYK_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, textureDataCMYK);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glGenTextures(1, &texRGB);
    glBindTexture(GL_TEXTURE_2D, texRGB);
    glTexImage2D(GL_TEXTURE_2D, 0,  GL_RGB8, TEX_RGB_WIDTH, TEX_RGB_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, textureDataRGB);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    shaderVertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shaderVertex, 1, (const GLchar**)&vertex_shader_source, NULL);
    glCompileShader(shaderVertex);
    if( !check_shader_compilation(shaderVertex) )
        return 0;

    shaderFragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(shaderFragment, 1, (const GLchar**)&fragment_shader_source, NULL);
    glCompileShader(shaderFragment);
    if( !check_shader_compilation(shaderFragment) )
        return 0;

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, shaderVertex);
    glAttachShader(shaderProgram, shaderFragment);
    glLinkProgram(shaderProgram);

    return 1;
}

static void bind_sampler_to_unit_with_texture(GLchar const * const sampler_name, GLuint texture_unit, GLuint texture)
{
        glActiveTexture(GL_TEXTURE0 + texture_unit); 
        glBindTexture(GL_TEXTURE_2D, texture);
        GLuint loc_sampler = glGetUniformLocation(shaderProgram, sampler_name);
        glUniform1i(loc_sampler, texture_unit);
}

static void pushModelview()
{
    GLenum prev_matrix_mode;
    glGetIntegerv(GL_MATRIX_MODE, &prev_matrix_mode);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMatrixMode(prev_matrix_mode);
}

static void popModelview()
{
    GLenum prev_matrix_mode;
    glGetIntegerv(GL_MATRIX_MODE, &prev_matrix_mode);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(prev_matrix_mode);
}


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

static void redrawTheWindow(double T)
{
    int const window_width = width;
    int const window_height = height;
    const float window_aspect = (float)window_width / (float)window_height;

    glDisable(GL_SCISSOR_TEST);

#if 0
    glClearColor(88./255., 95./255., 160./255., 0.);
#endif
    glClearColor(0., 0., 0., 0.);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, window_width, window_height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-window_aspect, window_aspect, -1, 1, 2.5, 10);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0, 0, -5);

    glDisable(GL_BLEND);

    pushModelview();
    glRotatef(T * 0.1 * 180, 0., 1., 0.);
    glRotatef(T * 0.1 *  60, 1., 0., 0.);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glUseProgram(shaderProgram);
    glUniform1f(glGetUniformLocation(shaderProgram, "T"), T);
    bind_sampler_to_unit_with_texture("texCMYK", 0, texCMYK);
    bind_sampler_to_unit_with_texture("texRGB", 1, texRGB);

    draw_cube();
    popModelview();

	struct timespec Ta, Tb;

#if USE_DOUBLEBUFFER
 	glXSwapBuffers(Xdisplay, glX_window_handle);
#else
	glFlush();
	usleep(10000);
#endif
}

static double getftime(void) {
	static long long offset = 0;
	long long t;
	struct timeval timeofday;

	gettimeofday(&timeofday, NULL);
	t = (long long)timeofday.tv_sec * 1000000 + (long long)timeofday.tv_usec;

	if(offset == 0)
		offset = t;

	return (double)(offset - t) * 1.e-6;
}

int main(int argc, char *argv[])
{

	createTheWindow();
	createTheRenderContext();
    
	if( !check_extensions() )
		return -1;

	if( !init_resources() )
		return -1;

	int n_dT_accum = 0;
	float dT_accum = 0;
	while( updateTheMessageQueue() ) {
		float const dT = getftime();
		redrawTheWindow(dT);

		dT_accum += dT;
		++n_dT_accum;

		if( 100 < n_dT_accum ) {
			fprintf(stderr, "%d frames in %fs (~%fFPS)\n",
				n_dT_accum,
				dT_accum,
				(float)n_dT_accum / dT_accum);
			dT_accum = 0.f;
			n_dT_accum = 0;
		}
	}

	return 0;
}

