#include <GL/glew.h>
#include <GL/glut.h>
#include <stdbool.h>
#include <stdio.h>

#include "debuggl/debuggl.h"
#include "linmath.h/linmath.h"

#include "positiongen.h"
#include "solid.h"
#include "stats.h"

#define HAS_ARCBALL 0

static struct {
	int width;
	int height;
	float aspect;
} window;

static struct {
	GLuint vbo;
	GLuint vao;
	int grid[2];
} vertexbuffer;

static GLuint timerquery;

#if HAS_ARCBALL
#define MAT4X4_IDENTITY {{1,0,0,0},{0,1,0,0},{0,0,1,0},{0,0,0,1}}
static struct dragging {
	bool left;
	vec2 v;
} pointer_dragging;
static mat4x4 view = MAT4X4_IDENTITY;
static mat4x4 arcball = MAT4X4_IDENTITY;
#endif

static struct stats_running drawtime_stats = STATS_RUNNING_INIT;

static
int create_vertexbuffer(int rows, int cols)
{
	size_t const dim = 4;

	debuggl_check( glGenVertexArrays(1, &vertexbuffer.vao) );
	debuggl_check( glBindVertexArray(vertexbuffer.vao) );

	debuggl_check( glGenBuffers(1, &vertexbuffer.vbo) );
	debuggl_check( glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer.vbo) );

	debuggl_check( glBufferStorage(GL_ARRAY_BUFFER,
		sizeof(GLfloat)*dim * rows * cols,
		NULL, 0 ) );

	debuggl_check( glEnableVertexAttribArray(0) );
	debuggl_check( glVertexAttribPointer(0, dim, GL_FLOAT, GL_FALSE, 0, 0) );

	vertexbuffer.grid[0] = rows;
	vertexbuffer.grid[1] = cols;

	debuggl_check( glBindBuffer(GL_ARRAY_BUFFER, 0) );
	debuggl_check( glBindVertexArray(0) );

	return 0;
}

static 
int load_shaders(void)
{
	int rc;
	(void)(0
	|| (rc= positiongen_load() )
	|| (rc= solid_load() )
	);
	return rc;
}

static
int create_resources(void)
{
	int rc;
	(void)(0
	|| (rc= create_vertexbuffer(1024, 1024) )
	|| (rc= load_shaders() )
	);

	if( !rc ) {
		debuggl_check( glGenQueries(1, &timerquery) );
	}

	return rc;
}

static
void keyboard(unsigned char key, int x, int y)
{
	switch(key) {
	case 'r':
	case 'R':
		load_shaders();
		glutPostRedisplay();
		break;
	}
}

#if HAS_ARCBALL
static
void pointer_button(int button, int state, int x, int y)
{
	if( GLUT_LEFT_BUTTON == button ) {
		if( GLUT_UP == state ) {
			pointer_dragging.left = false;

			mat4x4_mul(view, arcball, view);
			mat4x4_orthonormalize(view, view);
			mat4x4_identity(arcball);
		}
		else {
			pointer_dragging.left = true;

			pointer_dragging.v[0] =  2.f * (float)x / window.width  - 1.f;
			pointer_dragging.v[1] = -2.f * (float)y / window.height + 1.f;
		}
	}
}

static
void pointer_drag_motion(int x, int y)
{
	if( pointer_dragging.left ) {
		vec2 motion_v = {
			 2.f * (float)x / window.width  - 1.f,
			-2.f * (float)y / window.height + 1.f
		};

		mat4x4_identity(arcball);
		mat4x4_arcball(arcball, arcball,
			pointer_dragging.v,
			motion_v,
			1 );
		glutPostRedisplay();
	}
}
#endif

static
void reshape(int w, int h)
{
	window.width = w;
	window.height = h;
	window.aspect = (float)w / (float)h;

	stats_running_reset(&drawtime_stats);

	glutPostRedisplay();
}

static
void display(void)
{
	mat4x4 proj;
	mat4x4_identity(proj);

	float const fov = 0.5;
	mat4x4_frustum(proj,
		-window.aspect*fov,
		 window.aspect*fov,
		-fov,
		 fov, 1, 5);

	mat4x4 mv;
	mat4x4_identity(mv);
	mat4x4_translate(mv, 0, 0, -3);
#if HAS_ARCBALL
	mat4x4_mul(mv, mv, arcball);
	mat4x4_mul(mv, mv, view);
#endif

	glViewport(0,0,window.width,window.height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	positiongen_launch(
		glutGet(GLUT_ELAPSED_TIME)*0.001,
		vertexbuffer.vbo,
		vertexbuffer.grid[0],
		vertexbuffer.grid[1] );

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	float const white[4] = {1., 1., 1., 0.01};
	glBeginQuery(GL_TIME_ELAPSED, timerquery);
	solid_draw(
		GL_POINTS,
		vertexbuffer.vao,
		vertexbuffer.grid[0] * vertexbuffer.grid[1],
		white,
		mv[0], proj[0]);
	glEndQuery(GL_TIME_ELAPSED);

	glutSwapBuffers();

	GLuint timeresult;
	glGetQueryObjectuiv(timerquery, GL_QUERY_RESULT, &timeresult);
	stats_running_push(&drawtime_stats, timeresult*0.001);
}

enum {
	win_width_inc = 64, win_height_inc = 64,
	win_width_max = 1024, win_height_max = 1024
};
static int win_width = win_width_inc, win_height = win_height_inc;

static
void idle(void)
{
	if( 499 < stats_running_N(&drawtime_stats) ) {
		printf("%4d x %4d: ( %7.1f +/- %7.1f )us\n",
			win_width, win_height,
			stats_running_mean(&drawtime_stats),
			sqrt(stats_running_variance(&drawtime_stats)) );

		win_width += win_width_inc;
		if( win_width_max < win_width ) {
			win_height += win_height_inc;
			if( win_height_max < win_height ) {
				exit(0);
			}
			win_width = win_width_inc;
		}
		glutReshapeWindow(win_width, win_height);
	}
	glutPostRedisplay();
}

static
void gldebugcallback(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar *message,
	const void *userParam)
{
	fprintf(stderr, "(GL) %s\n", message);
}

static
void glinfodump(void)
{
	printf(
		"OpenGL vendor:   %s\n"
		"OpenGL renderer: %s\n"
		"OpenGL version:  %s\n",
		"  GLSL version:  %s\n",
		glGetString(GL_VENDOR),
		glGetString(GL_RENDERER),
		glGetString(GL_VERSION),
		glGetString(GL_SHADING_LANGUAGE_VERSION) );
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowPosition(0,0);
	glutInitWindowSize(win_width, win_height);
	glutCreateWindow("compute shader point overdraw benchmark");
	if( GLEW_OK != glewInit() ) { return 1; }

	glDebugMessageCallback((GLDEBUGPROC)gldebugcallback, NULL);
	glEnable(GL_DEBUG_OUTPUT);

	if( create_resources() ) { return 2; }

	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
#if HAS_ARCBALL
	glutMouseFunc(pointer_button);
	glutMotionFunc(pointer_drag_motion);
#endif
	glutDisplayFunc(display);
	glutIdleFunc(idle);

	glutMainLoop();
	return 0;
}
