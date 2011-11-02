#include <math.h>
#include <GL/gl.h>
#include <GL/glut.h>

struct {
	float triangle_rotation;
} scene;

static const GLfloat triangle_vertices[3][3] = {
	{-1., -1., 0.},
	{ 1., -1., 0.},
	{ 0., 0.732, 0.}
};

static void draw_triangle()
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 3*sizeof(GLfloat), &triangle_vertices[0][0]);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDisableClientState(GL_VERTEX_ARRAY);
}

static void display(void)
{
	const int window_width = glutGet(GLUT_WINDOW_WIDTH);
	const int window_height = glutGet(GLUT_WINDOW_HEIGHT);
	if( !window_width || !window_height )
		return;
	const float window_aspect = (float)window_width / (float)window_height;

	glDisable(GL_SCISSOR_TEST);

	glClearColor(0.4, 0.3, 0.5, 1.0);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, window_width, window_height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glFrustum(-window_aspect, window_aspect, -1, 1, 2, 10);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0., 0., -5.);

	glPushMatrix();
	glRotatef(scene.triangle_rotation, 0., 1., 0.);
	draw_triangle();
	glPopMatrix();

	glutSwapBuffers();
}

static void special_key(int key, int x, int y)
{
	switch(key) {
	case GLUT_KEY_LEFT:
		scene.triangle_rotation = fmod(scene.triangle_rotation + 5, 360);
		break;
	case GLUT_KEY_RIGHT:
		scene.triangle_rotation = fmod(scene.triangle_rotation - 5, 360);
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutCreateWindow("Simple input altering scene demo");
	glutDisplayFunc(display);
	glutSpecialFunc(special_key);

	glutMainLoop();
}

