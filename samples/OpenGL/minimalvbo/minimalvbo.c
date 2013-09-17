#include <GL/glew.h>
#include <GL/glut.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

void init();
void display();

/* With the introduction of Buffer Objects, a somewhat questionable decision
 * regarding the reuse of existing API was made. Specifically the reuse of the
 * gl...Pointer functions to supply integers through their `data` parameter
 * which is a pointer can lead to serious complications, should a compiler
 * decide to implement augmented or tagged pointers.
 *
 * In many OpenGL tutorials, and even the reference documentation one can
 * find that an iteger gets cast into a pointer. From the point of view of
 * the C language standard and the interpretation of integers cast to pointer
 * this practice should be strongly discouraged.
 *
 * The far better method is not to cast the offset integer value to a pointer,
 * but to cast the `gl...Pointer` functions to a type signature that defines
 * the `data` parameter to be an integer. To keep the size of the `data`
 * parameter consistent use the C standard type `uintptr_t` which is guaranted
 * to be large enough to hold the integer representation of a pointer.
 *
 * The same also goes for the glDraw...Elements functions.
 *
 * Also see http://stackoverflow.com/a/8284829/524368
 *
 * A Note to C++ folks reading this: In C++ you'd have to make a cast to the
 * L-values type signature so that the compiler doesn't warn and/or error
 * out. In C there's this nice property that a void pointer (`void*`) R-value
 * can be legally assigned to any pointer type L-value. So because of lazyness
 * and because this is messing with type signatures I simply cast to a `void*`
 * which perfectly well assigned to the function pointer signatures instead
 * of writing `= (void(*)(bla bla bla))...` (or doing a lot of typedefs).
 */
void (*glfixVertexOffset)(GLint, GLenum, GLsizei, uintptr_t const)        = (void*)glVertexPointer;
void (*glfixTexCoordOffset)(GLint, GLenum, GLsizei, uintptr_t const)      = (void*)glTexCoordPointer;
void (*glfixDrawElementsOffset)(GLenum, GLsizei, GLenum, uintptr_t const) = (void*)glDrawElements;
/* Those three functions are part of OpenGL-1.1, hence in the OpenGL ABIs of
 * all operating systems with OpenGL support and thus available as symbols
 * of the interface library, without additional initialization.
 *
 * Don't use static const initialization for function pointer obtained
 * through the OpenGL extension mechanism!
 */

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );

	glutCreateWindow("Minimal VBO example");
	glutDisplayFunc(display);
	glutIdleFunc(glutPostRedisplay);

	glewInit();
	init();

	glutMainLoop();

	return 0;
}

GLuint vertexbufId;
GLuint elementbufId;

void init()
{
	glGenBuffers(1, &vertexbufId);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbufId);

	float const cube[5*8]=
	{ /*	 X   Y   Z   U   V  */
		-1, -1, -1,  0,  0,
		 1, -1, -1,  1,  0,
		 1,  1, -1,  1,  1,
		-1,  1, -1,  0,  1,

		-1, -1,  1, -1,  0,
		 1, -1,  1,  0,  0,
		 1,  1,  1,  0,  1,
		-1,  1,  1, -1,  1,
	};

	glBufferData(GL_ARRAY_BUFFER, 5*8*sizeof(float), cube, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);


	glGenBuffers(1, &elementbufId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbufId);

	unsigned int faces[4*6]=
	{
		0, 1, 2, 3,
		1, 5, 6, 2,
		5, 4, 7, 6,
		4, 0, 3, 7,
		3, 2, 6, 7,
		4, 5, 1, 0
	};

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 4*6*sizeof(float), faces, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

static float a = 0., b = 0., c = 0.;

void display()
{
	  int const win_width  = glutGet(GLUT_WINDOW_WIDTH);
	  int const win_height = glutGet(GLUT_WINDOW_HEIGHT);
	float const win_aspect = (float)win_width / (float) win_height;

	glClearColor(0.5, 0.5, 1., 1.);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, win_width, win_height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-win_aspect, win_aspect, -1, 1, 1, 10);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0, 0, -3);
	glRotatef(a += 1.00, 1, 0, 0);
	glRotatef(b += 0.66, 0, 1, 0);
	glRotatef(c += 0.33, 0, 0, 1);
	
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, vertexbufId);
	glfixVertexOffset(  3, GL_FLOAT, 5*sizeof(float), 0);
	glfixTexCoordOffset(2, GL_FLOAT, 5*sizeof(float), 3);
	/* After the data source of a Vertex Array has been set
	 * this setting stays persistent regardless of any other
	 * state changes. So we can unbind the VBO bufferId here
	 * without loosing the connection. We must not delete
	 * the VBO though.
	 */
	glBindBuffer(GL_ARRAY_BUFFER, 0),

	glColor4f(0., 0., 0., 1.),

	/* Just drawing a wireframe */
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbufId);
	glfixDrawElementsOffset(GL_QUADS, 24, GL_UNSIGNED_INT, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glutSwapBuffers();
}

