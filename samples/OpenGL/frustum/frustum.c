#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

#include <alloca.h>

#include <math.h>

#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include "linmath.h"

#if defined(GLUT_MULTISAMPLE) && defined(GL_MULTISAMPLE)
#define OPTION_GLUT_MULTISAMPLE GLUT_MULTISAMPLE
#define OPTION_MULTISAMPLE 1
#else
#define OPTION_GLUT_MULTISAMPLE 0
#define OPTION_MULTISAMPLE 0
#warning "multisample token(s) not available at compiletime"
#endif

int window_view;
int window_observer;

/* == basic Q^3 vector math functions == */

static void crossproduct(
	double ax, double ay, double az,
	double bx, double by, double bz,
	double *rx, double *ry, double *rz )
{
	*rx = ay*bz - az*by;
	*ry = az*bx - ax*bz;
	*rz = ax*by - ay*bx;
}

static void crossproduct_v(
	double const * const a,
	double const * const b,
	double * const c )
{
	crossproduct(
		a[0], a[1], a[2],
		b[0], b[1], b[2],
		c, c+1, c+2 );
}

static double scalarproduct(
	double ax, double ay, double az,
	double bx, double by, double bz )
{
	return ax*bx + ay*by + az*bz;
}

static double scalarproduct_v(
	double const * const a,
	double const * const b )
{
	return scalarproduct(
		a[0], a[1], a[2],
		b[0], b[1], b[2] );
}

static double length(
	double ax, double ay, double az )
{
	return sqrt(
		scalarproduct(
			ax, ay, az,
			ax, ay, az ) );
}

static double length_v( double const * const a )
{
	return sqrt( scalarproduct_v(a, a) );
}

static double normalize(
	double *x, double *y, double *z)
{
	double const k = 1./length(*x, *y, *z);

	*x *= k;
	*y *= k;
	*z *= k;
}

static double normalize_v( double *a )
{
	double const k = 1./length_v(a);
	a[0] *= k;
	a[1] *= k;
	a[2] *= k;
}

/* == annotation drawing functions == */

void draw_strokestring(void *font, float const size, char const *string)
{
	glPushMatrix();
	float const scale = size * 0.01; /* GLUT character base size is 100 units */
	glScalef(scale, scale, scale);

	char const *c = string;
	for(; c && *c; c++) {
		glutStrokeCharacter(font, *c);
	}
	glPopMatrix();
}

void draw_arrow(
	float ax, float ay, float az,
	float bx, float by, float bz,
	float ah, float bh,
	char const * const annotation,
	float annot_size )
{
	int i;

	GLdouble mv[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, mv);
	
	/* We're assuming the modelview RS part is (isotropically scaled)
	 * orthonormal, so the inverse is the transpose.
	 * The local view direction vector is the 3rd column of the matrix;
	 * assuming the view direction to be the normal on the arrows tangent
	 * space  taking the cross product of this with the arrow direction
	 * yields the binormal to be used as the orthonormal base to the 
	 * arrow direction to be used for drawing the arrowheads */

	double d[3] = {
	      bx - ax,
	      by - ay,
	      bz - az
	};
	normalize_v(d);

	double r[3] = { mv[0], mv[4], mv[8] };
	int rev = scalarproduct_v(d, r) < 0.;

	double n[3] = { mv[2], mv[6], mv[10] };
	{
		double const s = scalarproduct_v(d,n);
		for(int i = 0; i < 3; i++)
			n[i] -= d[i]*s;
	}
	normalize_v(n);

	double b[3];
	crossproduct_v(n, d, b);

	GLfloat const pos[][3] = {
		{ax, ay, az},
		{bx, by, bz},
		{ ax + (0.866*d[0] + 0.5*b[0])*ah,
		  ay + (0.866*d[1] + 0.5*b[1])*ah,
		  az + (0.866*d[2] + 0.5*b[2])*ah },
		{ ax + (0.866*d[0] - 0.5*b[0])*ah,
		  ay + (0.866*d[1] - 0.5*b[1])*ah,
		  az + (0.866*d[2] - 0.5*b[2])*ah },
		{ bx + (-0.866*d[0] + 0.5*b[0])*bh,
		  by + (-0.866*d[1] + 0.5*b[1])*bh,
		  bz + (-0.866*d[2] + 0.5*b[2])*bh },
		{ bx + (-0.866*d[0] - 0.5*b[0])*bh,
		  by + (-0.866*d[1] - 0.5*b[1])*bh,
		  bz + (-0.866*d[2] - 0.5*b[2])*bh }
	};
	GLushort const idx[][2] = {
		{0, 1},
		{0, 2}, {0, 3},
		{1, 4}, {1, 5}
	};
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, pos);

	glDrawElements(GL_LINES, 2*5, GL_UNSIGNED_SHORT, idx);
	glDisableClientState(GL_VERTEX_ARRAY);

	if(annotation) {
		float w = 0;
		for(char const *c = annotation; *c; c++)
			w += glutStrokeWidth(GLUT_STROKE_ROMAN, *c);
		w *= annot_size / 100.;

		float tx = (ax + bx)/2.;
		float ty = (ay + by)/2.;
		float tz = (az + bz)/2.;

		GLdouble r[16] = {
			d[0], d[1], d[2], 0,
			b[0], b[1], b[2], 0,
			n[0], n[1], n[2], 0,
			   0,    0,    0, 1
		};
		glPushMatrix();
		glTranslatef(tx, ty, tz);
		glMultMatrixd(r);
		if(rev)
			glScalef(-1, -1, 1);
		glTranslatef(-w/2., annot_size*0.1, 0);
		draw_strokestring(GLUT_STROKE_ROMAN, annot_size, annotation);
		glPopMatrix();
	}
}

void draw_arc(
	vec3 center,
	vec3 a, vec3 b,	
	float ah, float bh,
	char const * const annotation,
	float annot_size )
{
	a[0] = b[2];
}

void draw_frustum(
	float l, float r, float b, float t,
	float n, float f )
{
	GLfloat const kf = f/n;
	GLfloat const pos[][3] = {
		{0,0,0},
		{l, b, -n},
		{r, b, -n},
		{r, t, -n},
		{l, t, -n},
		{kf*l, kf*b, -f},
		{kf*r, kf*b, -f},
		{kf*r, kf*t, -f},
		{kf*l, kf*t, -f}
	};
	GLushort const idx_tip[][2] = {
		{0, 1},
		{0, 2},
		{0, 3},
		{0, 4}
	};
	GLushort const idx_vol[][2] = {
		{1, 5}, {2, 6},	{3, 7},	{4, 8},
		{1, 2},	{2, 3},	{3, 4},	{4, 1},
		{5, 6},	{6, 7},	{7, 8},	{8, 5}
	};

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, pos);

	glLineWidth(1);
	glLineStipple(2, 0xf3cf);
	glEnable(GL_LINE_STIPPLE);
	glDrawElements(GL_LINES, 2*4, GL_UNSIGNED_SHORT, idx_tip);

	glLineWidth(2);
	glLineStipple(1, 0xffff);
	glDisable(GL_LINE_STIPPLE);
	glDrawElements(GL_LINES, 2*4*3, GL_UNSIGNED_SHORT, idx_vol);
	glDisableClientState(GL_VERTEX_ARRAY);

	glLineWidth(1.5);

	float const text_size = 0.15f;

	draw_arrow(l, 0, 0, 0, 0, 0, 0.1, 0.0, "left",   text_size);
	draw_arrow(0, 0, 0, r, 0, 0, 0.0, 0.1, "right",  text_size);
	draw_arrow(0, b, 0, 0, 0, 0, 0.1, 0.0, "bottom", text_size);
	draw_arrow(0, 0, 0, 0, t, 0, 0.0, 0.1, "top",    text_size);

	draw_arrow(r, 0, 0, r, 0, -n, 0.1, 0.1, "near", text_size);
	draw_arrow(l, 0, 0, l, 0, -n, 0.1, 0.1, "near", text_size);
	draw_arrow(0, t, 0, 0, t, -n, 0.1, 0.1, "near", text_size);
	draw_arrow(0, b, 0, 0, b, -n, 0.1, 0.1, "near", text_size);

	draw_arrow(0, f*t/n, 0, 0, f*t/n, -f, 0.1, 0.1, "far", text_size);
	draw_arrow(0, f*b/n, 0, 0, f*b/n, -f, 0.1, 0.1, "far", text_size);
	draw_arrow(f*l/n, 0, 0, f*l/n, 0, -f, 0.1, 0.1, "far", text_size);
	draw_arrow(f*r/n, 0, 0, f*r/n, 0, -f, 0.1, 0.1, "far", text_size);
}

static void draw_grid1d(
	double ax, double ay, double az,  /* grid advance */
	double dx, double dy, double dz,  /* grid direction */
	float l0, float l1,            /* grid line range */
	int major, int begin, int end, /* major line modulus; grid begin, end */
	float or, float og, float ob   /* origin line color r,g,b */ )
{
	if( begin > end ) {
		int t = begin;
		begin = end;
		end = t;
	}
	unsigned int const N = end - begin + 1;

	GLfloat *pos = alloca(N*6 * sizeof(*pos));
	GLfloat *col = alloca(N*8 * sizeof(*col));

	normalize(&dx, &dy, &dz);

	for(int i = begin; i <= end; i++) {
		int const j = i - begin;
		pos[j*6 + 0] = i*ax + l0*dx;
		pos[j*6 + 1] = i*ay + l0*dy;
		pos[j*6 + 2] = i*az + l0*dz;
		pos[j*6 + 3] = i*ax + l1*dx;
		pos[j*6 + 4] = i*ay + l1*dy;
		pos[j*6 + 5] = i*az + l1*dz;

		GLfloat r,g,b,a;
#if 1
		if( !i ) {
			r = or;
			g = og;
			b = ob;
			a = 1.;
		} else if( !(i % major) ) {
			r = g = b = 0.3;
			a = 0.5;
		} else {
			r = g = b = 0.5;
			a = 0.33;
		}
#else
		r = 1.;
		g = b = 0.;
		a = 1.;
#endif

		col[j*8 + 0] = col[j*8 + 4] = r;
		col[j*8 + 1] = col[j*8 + 5] = g;
		col[j*8 + 2] = col[j*8 + 6] = b;
		col[j*8 + 3] = col[j*8 + 7] = a;
	}
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glDisable(GL_LIGHTING);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, pos);
	glColorPointer(4, GL_FLOAT, 0, col);

	glLineWidth(1);
	glLineStipple(1, 0xffff);
	glDisable(GL_LINE_STIPPLE);

	glDrawArrays(GL_LINES, 0, N*2);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}

/* == scene drawing code == */

#define N_LIGHTS 2
struct {
	GLfloat diffuse[3];
	GLfloat position[4];
} light[N_LIGHTS] = {
	{ {255./255., 202./255., 99./255.}, {-1, 1, 1, 0} },
	{ {30./255, 38./255., 82./255.}, { 0, 1, -0.4, 0} },
};

void draw_scene(void)
{
	glPushMatrix();
	glTranslatef(0, 0, -2.5);

	for(int i = 0; i < N_LIGHTS; i++) {
		glEnable( GL_LIGHT0 + i);
		glLightfv(GL_LIGHT0 + i, GL_DIFFUSE,  light[i].diffuse);
		glLightfv(GL_LIGHT0 + i, GL_POSITION, light[i].position);
	}

	glEnable(GL_LIGHTING);

	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	glPushMatrix();
	glTranslatef(-0.75, 0, -0.5);
	glRotatef(45, 0, 1, 0);

	glColor3f(0.9, 0.9, 0.9);
	glutSolidTeapot(0.6);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.5, 0, 0.5);
	glRotatef(45, 0, 1, 0);

	glColor3f(0.9, 0.9, 0.9);
	glutSolidCube(0.6);
	glPopMatrix();

	glPopMatrix();
}

/* == display functions == */

struct {
	float left, right, bottom, top;
	float near, far;
} frustum = {-1, 1, -1, 1, 1, 4};

void display_observer(float frustum_aspect)
{
	int const win_width  = glutGet(GLUT_WINDOW_WIDTH);
	int const win_height = glutGet(GLUT_WINDOW_HEIGHT);
	float const win_aspect = (float)win_width / (float)win_height;

	glViewport(0, 0, win_width, win_height);
	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
#ifdef USE_ORTHO
	glOrtho(-10*win_aspect, 10*win_aspect, -10, 10, 0, 100);
#else
	gluPerspective(60, win_aspect, 1, 50);
#endif

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if(1) {
		glTranslatef(0, 0, -10);
		glRotatef(15, 1, 0, 0);
		glRotatef(-15, 0, 1, 0);
		glTranslatef(0, 0, 2.5);
	} else {
		gluLookAt(3, 1, -5, 0, 0, -2.5, 0, 1, 0);
	}

#if OPTION_MULTISAMPLE
	glEnable(GL_MULTISAMPLE);
#endif

#ifdef GL_DEPTH_CLAMP
	glEnable(GL_DEPTH_CLAMP);
#endif

	glDisable(GL_LIGHTING);
	glDepthMask(GL_TRUE);
	glColor3f(0.,0.,0.);
	draw_frustum(
		frustum.left,
		frustum.right,
		frustum.bottom,
		frustum.top,
		frustum.near,
		frustum.far );

	glEnable(GL_DEPTH_TEST);
	draw_scene();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE);
	draw_grid1d(
		0, 0.1, 0,
		1, 0, 0,
		-5, 5, 
		10, -50, 50,
		1, 0.3, 0.3 );
	draw_grid1d(
		0.1, 0, 0,
		0, 1, 0,
		-5, 5, 
		10, -50, 50,
		0.3, 1, 0.3 );
	glDepthMask(GL_TRUE);
	glDisable(GL_BLEND);

	glutSwapBuffers();
}

void display_view(int const win_width, int const win_height)
{
	float const win_aspect = (float)win_width / (float)win_height;
	frustum.left = -(frustum.right = win_aspect);

	glViewport(0, 0, win_width, win_height);
	glClearColor(0.3, 0.3, 0.6, 1.);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(
		frustum.left,
		frustum.right,
		frustum.bottom,
		frustum.top,
		frustum.near,
		frustum.far );
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_DEPTH_TEST);

#if OPTION_MULTISAMPLE
	glEnable(GL_MULTISAMPLE);
#endif

	draw_scene();

	glutSwapBuffers();
}

void display(void)
{
	glutSetWindow(window_view);
	int const win_view_width  = glutGet(GLUT_WINDOW_WIDTH);
	int const win_view_height = glutGet(GLUT_WINDOW_HEIGHT);
	float const win_view_aspect = (float)win_view_width / (float)win_view_height;
	display_view(win_view_width, win_view_height);

	glutSetWindow(window_observer);
	display_observer(win_view_aspect);
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE | OPTION_GLUT_MULTISAMPLE);

	window_observer = glutCreateWindow("Observer");
	glutDisplayFunc(display);

	window_view = glutCreateWindow("Frustum View");
	glutDisplayFunc(display);

	glutMainLoop();
	return 0;
}
