#include <GL/glut.h>
#include <time.h>
#include <memory.h>

#include "uhr.h"

using namespace uhr;

void init();
void reshape(int width, int height);
void display();
void idle();

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH|GLUT_MULTISAMPLE);

	glutCreateWindow("PocketWatch");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutIdleFunc(idle);
	init();
	
	glutMainLoop();

	return 0;
};

void init()
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glEnable(GL_MULTISAMPLE);
}

float light_position[]={-0.5,0.5,1,0};

void reshape(int width, int height)
{
	glViewport(0,0,width,height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, (float)width/(float)height, 1, 10);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void draw_object(C3DObject const *obj);
void draw_object_rot_z(C3DObject const *obj, float rot);

void display()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0,0,-4);


	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	draw_object(&object_gehaeuse);
	draw_object(&object_boden);
	draw_object(&object_ziffernblatt);
	draw_object(&object_sekziffernblatt);

	time_t now=time(NULL);
	tm tm_time=*localtime(&now);

	draw_object_rot_z(&object_stundenzeiger, (tm_time.tm_min/60.0f+tm_time.tm_hour)*720.0f/24.0f);
	draw_object_rot_z(&object_minutenzeiger, (tm_time.tm_min+tm_time.tm_sec/60.0f)*360.0f/60.0f);
	draw_object_rot_z(&object_sekundenzeiger, tm_time.tm_sec*360.0f/60.0f);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	draw_object(&object_deckel);

	glutSwapBuffers();
}

void idle()
{
	glutPostRedisplay();
}

void draw_object(C3DObject const *obj)
{
	glPushMatrix();
	glMultMatrixf(obj->matrix);

	float tmp[4];
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, obj->material);

	memcpy(tmp, obj->material+4, sizeof(float)*3);
	tmp[3]=1.0f;
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, tmp);

	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, obj->material[7]);

	for(int i=0; i<4; i++)tmp[i]=obj->material[i]*obj->material[8];
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, tmp);

	glBegin(GL_TRIANGLES);
	for(int f=0; f<*obj->nFaces; f++)
	{
		for(int v=0; v<3; v++)
		{
			int i=obj->faces[f][v];
			glNormal3fv(obj->verticies[i][1]);
			glVertex3fv(obj->verticies[i][0]);
		}
	}
	glEnd();

	glPopMatrix();
}

void draw_object_rot_z(C3DObject const *obj, float rot)
{
	glPushMatrix();
	glMultMatrixf(obj->matrix);
	glRotatef(rot,0,0,-1);

	float tmp[4];
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, obj->material);

	memcpy(tmp, obj->material+4, sizeof(float)*3);
	tmp[3]=1.0f;
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, tmp);

	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, obj->material[7]);

	for(int i=0; i<4; i++)tmp[i]=obj->material[i]*obj->material[8];
	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, tmp);

	glBegin(GL_TRIANGLES);
	for(int f=0; f<*obj->nFaces; f++)
	{
		for(int v=0; v<3; v++)
		{
			int i=obj->faces[f][v];
			glNormal3fv(obj->verticies[i][1]);
			glVertex3fv(obj->verticies[i][0]);
		}
	}
	glEnd();

	glPopMatrix();
}
