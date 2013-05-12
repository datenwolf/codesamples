#include <GL/glew.h>
#include <GL/glut.h>
#include <GLT/multierror.h>
#include <GLT/shaderloader.h>

typedef enum {
	si_NoError = 0,
	si_ResourceNotFound = 1,
} si_Error_t;

struct {
	GLuint prog;
	GLuint vs;
	GLuint fs;

	GLuint a_position;
	GLuint a_direction;

	GLuint u_mv;
	GLuint u_normal:
	GLuint u_proj;

	GLuint u_lightpos;
} strandshader;

si_Error_t loadStrandShader(void)
{
}

si_Error_t loadGLresources(void)
{
	loadStrandShader();
}

void display(void)
{
}

int main(int argc, char argv[])
{
	si_Error_t err = si_NoError;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutCreateWindow("Illuminated Strands");
	glutDisplayFunc(display);

	if( (err = loadGLResources()) != si_NoError ) {
		return -err;
	}

	glutMainLoop();
	
	return 0;
}

