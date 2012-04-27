#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <math.h>

#include <string>

#define ILUT_USE_OPENGL 1

#include <GL/glew.h>
#include <GL/glut.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>

using namespace std;

#include "vecmath.h"
#include "terragen.h"
#include "terrain.h"

Terrain Q;

double const water_level=-1.0;

void reshape(int width, int height);
void display();
void keyboard(unsigned char key, int x, int y);

double frand()
{
	return (double)rand()/(double)RAND_MAX;
}

struct RGB
{
	GLubyte r, g, b;
};

int width, height, levels;
int iMaxLOD;
int iMaxCull=7;
double t, k, l;
double scale[3];
double scale_dim;
double fov=1.414/2;
//double fov=0.1;
double near_clip=1;
double far_clip=100000;

void assign_elevations(Terrain *pT, double *buffer, int width, int height, double t, double k,
					   int left=-1, int bottom=-1, int right=-1, int top=-1);

GLuint texID;

void idle()
{
	glutPostRedisplay();
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA|GLUT_DEPTH|GLUT_DOUBLE);
	glutCreateWindow("Terrain");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);

	glewInit();

	if(argc<2)
		return -1;

	std::string terrain_filename(argv[1]);

	double *buffer=NULL;
	read_terrain(terrain_filename.c_str(), &buffer, &width, &height, scale);
	levels = max(width, height);
	levels = log(levels)/log(2);
	scale_dim=sqrt(pow(scale[0]*width, 2)+pow(scale[1]*height, 2)+scale[2]*scale[2]);

	iMaxLOD=levels;

	ilInit();
	iluInit();
	ilutInit();
	ilutRenderer(ILUT_OPENGL);

	std::string img_filename( terrain_filename );
	img_filename.replace(img_filename.length()-5, 4, ".png");

	texID=ilutGLLoadImage(img_filename.c_str());
	glBindTexture(GL_TEXTURE_2D, texID);

	int x, y;
	double min_v = 0, max_v = 0;
	for(y = 0; y < height; y++)
	{
		for( x = 0; x < width; x++)
		{
			min_v = min( buffer[ y*width + x ], min_v );
			max_v = max( buffer[ y*width + x ], max_v );
		}
	}

#if 0
	RGB *img_buffer = new RGB[width*height];
	t=min_v;
	k=255.0/(double)(max_v-min_v);
	l=1.0/(double)(max_v-min_v);
	for(y=0; y<height; y++)
	{
		for(x=0; x<width; x++)
		{
			RGB *rgb=(img_buffer+(y*width+x));
			double const *value=(buffer+(y*width+x));

			Q.track_down((double)x/(double)width, (double)y/(double)height, (*value), levels);

			if(*value>=water_level)
			{
				rgb->r=rgb->g=rgb->b=(*value-t)*k;
			}
			else
			{
				rgb->r=rgb->g=0.5*(*value-t)*k+75;
				rgb->b=(*value-t)*k+128;
			}
			
		}
	}
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glBindTexture(GL_TEXTURE_RECTANGLE_NV, 1);
	glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, img_buffer);

	delete[] img_buffer;
#endif

	assign_elevations(&Q, buffer, width, height, 0, 1);

	delete[] buffer;

	glEnable(GL_CULL_FACE);
	glClearColor(0.7, 0.8, 1.0, 1.0);

	glutMainLoop();

	return 0;
};

void assign_elevations(Terrain *pT, double *buffer, int width, int height, double t, double k,
					   int left, int bottom, int right, int top)
{
	if(left == -1)
		left=0;
	if(bottom == -1)
		bottom=0;
	if(right == -1)
		right=width-1;
	if(top == -1)
		top=height-1;

// WTF, a no-op macro, why?!
// BTW, I wrote that, whatever I was planning, I didn't finish it
// -- Wolfgang
#define PEL(d) (d)

	pT->z[0][0]=( buffer[ PEL(left+bottom*width) ] -t )*k;
	pT->z[0][1]=( buffer[ PEL(right+bottom*width) ] -t )*k;
	pT->z[1][1]=( buffer[ PEL(right+top*width) ] -t )*k;
	pT->z[1][0]=( buffer[ PEL(left+top*width) ] -t )*k;

	pT->z_mean=(
		pT->z[0][0]+
		pT->z[0][1]+
		pT->z[1][1]+
		pT->z[1][0])*0.25;

	if(pT->is_split())
	{
		assign_elevations((Terrain*)pT->quads[0][0], buffer, width, height, t, k,
							left, bottom, left+(right-left)/2, bottom+(top-bottom)/2);

		assign_elevations((Terrain*)pT->quads[0][1], buffer, width, height, t, k,
							left+(right-left)/2, bottom, right, bottom+(top-bottom)/2);

		assign_elevations((Terrain*)pT->quads[1][1], buffer, width, height, t, k,
							left+(right-left)/2, bottom+(top-bottom)/2, right, top);

		assign_elevations((Terrain*)pT->quads[1][0], buffer, width, height, t, k,
							left, bottom+(top-bottom)/2, left+(right-left)/2, top);
	}
}

double view_dot;

void reshape(int width, int height)
{
	glViewport(0,0,width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	double aspect=(double)width/(double)height;

	double x=near_clip*fov;
	double y=x/aspect;

	double RIGHT_UPPER[3]={x, y, near_clip};
	double cam_view[3]={0,0,1};
	normalize(RIGHT_UPPER, RIGHT_UPPER);
	view_dot=DOT(cam_view, RIGHT_UPPER);

	glFrustum(-x, x, -y, y, near_clip, far_clip);
	glMatrixMode(GL_MODELVIEW);
}

bool bDrawQuads=true;
bool bFill=true;
bool bLOD=true;
bool bFrustumCulling=true;
bool bContinuous=true;
bool bAnimation=true;
bool bPaused=false;

enum Camera{follow, bird_view};
int eCam;

void keyboard(unsigned char key, int x, int y)
{
	switch(key)
	{
	case ' ':
		bAnimation=!bAnimation;
		break;

	case 'Q':
	case 'q':
		exit(0);
		break;

	case 'F':
	case 'f':
		bFill=!bFill;
		break;

	case 'H':
	case 'h':
		bFrustumCulling=!bFrustumCulling;
		break;

	case 'L':
	case 'l':
		bLOD=!bLOD;
		break;

	case 'V':
	case 'v':
		eCam++;
		if(eCam>bird_view)
			eCam = 0;
		break;

	case 'C':
	case 'c':
		bContinuous=!bContinuous;
		break;

	case 'p':
	case 'P':
		bPaused=!bPaused;
		break;

	case 's':
	case 'S':
		ilutGLScreenie();

	case '+':
		if(iMaxLOD<levels)
			iMaxLOD++;
		break;

	case '-':
		if(iMaxLOD>0)
			iMaxLOD--;
		break;
	}
	glutPostRedisplay();
}

void draw_quad(Terrain *pQ, int LOD=0, double z1=0, double z2=0, double z3=0, double z4=0);

GLdouble cam_pos[3];//={68.0f*scale[0], 44.0f*scale[1], 47.7*scale[2]*0.5};
double cam_la[3];
double cam_vec[3];
double cam_vec_n0[3];

#define PI 3.141512

double modelview_matrix[16];
double projection_matrix[16];
GLint viewport[4]={0,0,1,1};

void display()
{
	static double alpha=0;
	static double beta=0;

	if(bAnimation)
	{
		alpha+=0.0025*PI;
		beta+=0.001*PI;
		
		if(alpha>2*PI)
			alpha=0;

		if(beta>2*PI)
			beta=0;
	}

	double dx=(1+cos(alpha))*0.5;
	double dy=(1+sin(alpha))*0.5;

	double dz=(1+sin(beta))*0.5;

	cam_pos[0]=dx*width*scale[0];
	cam_pos[1]=dy*height*scale[1];
	cam_pos[2]=max(dz*scale[2]*width*3, (20.0f+((Terrain*)Q.get_at(dx, dy))->z_mean)*scale[2]);
	//cam_pos[2]=(20.0f+((Terrain*)Q.get_at(dx, dy))->z_mean)*scale[2];

	cam_la[0]=width*scale[0]*0.5;
	cam_la[1]=height*scale[1]*0.5;
	cam_la[2]=9.6*scale[2]*0.5;

	cam_vec[0]=cam_la[0]-cam_pos[0];
	cam_vec[1]=cam_la[1]-cam_pos[1];
	cam_vec[2]=cam_la[2]-cam_pos[2];

	normalize(cam_vec_n0, cam_vec);

	normalize(cam_vec, cam_vec);

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	
	glLoadIdentity();
	gluLookAt(	cam_pos[0], cam_pos[1], cam_pos[2],
					cam_la[0], cam_la[1], cam_la[2],
					0,0,1);
	glScalef(1,1,0.5);
	glGetDoublev(GL_MODELVIEW_MATRIX, modelview_matrix);
	glGetDoublev(GL_PROJECTION_MATRIX, projection_matrix);
	glGetIntegerv(GL_VIEWPORT, viewport);

	switch(eCam)
	{
	case follow:
		break;

	case bird_view:
		glLoadIdentity();
		gluLookAt(	width*scale[0]*0.5, height*scale[1]*0.5, scale[2]*500,
					cam_la[0], cam_la[1], cam_la[2],
					0,1,0);
		//glScalef(1,1,0.5);
		break;
	}
	//glTranslatef(-0.5, -0.5, -5);

	glEnable(GL_DEPTH_TEST);
	/*
	if(bDrawTerrain)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_TEXTURE_RECTANGLE_NV);
		glBegin(GL_QUADS);
		glColor3f(1,1,1);
		glTexCoord2i(0,0);
		glVertex2f(0,0);
		glTexCoord2i(width,0);
		glVertex2f(1,0);
		glTexCoord2i(width, height);
		glVertex2f(1,1);
		glTexCoord2i(0,height);
		glVertex2f(0,1);
		glEnd();
	}*/

	glColor3f(1,0,0);
	glPointSize(5);
	glBegin(GL_POINTS);
	glVertex3dv(cam_pos);
	glEnd();

	if(bDrawQuads)
	{
		glColor3f(1,1,1);
		glDisable(GL_TEXTURE_RECTANGLE_NV);
		glEnable(GL_TEXTURE_2D);
		glPolygonMode(GL_FRONT_AND_BACK, bFill?GL_FILL:GL_LINE);
		glBegin(GL_QUADS);
		draw_quad(&Q);
		glEnd();
	}

	glutSwapBuffers();
}

bool in_view(double a[3])
{
	GLdouble winx, winy, winz;

	gluProject(a[0], a[1], a[2], modelview_matrix, projection_matrix, viewport, &winx, &winy, &winz);
	if(	winx>=viewport[0] && winx<=viewport[2] &&
		winy>=viewport[1] && winy<=viewport[3] )
		return true;

	return false;
}

void draw_quad(Terrain *pT, int LOD, double z1, double z2, double z3, double z4)
{
	bool bRefine=true;
	bool bInFront=true;
	
	double Q[3]={pT->x_mid*width*scale[0], pT->y_mid*height*scale[1], pT->z_mean*scale[2]};
	double Q1[3]={pT->x1*width*scale[0], pT->y1*height*scale[1], pT->z[0][0]*scale[2]};
	double Q2[3]={pT->x2*width*scale[0], pT->y1*height*scale[1], pT->z[0][1]*scale[2]};
	double Q3[3]={pT->x2*width*scale[0], pT->y2*height*scale[1], pT->z[1][1]*scale[2]};
	double Q4[3]={pT->x1*width*scale[0], pT->y2*height*scale[1], pT->z[1][0]*scale[2]};	
	double Qt[3];

	double dir[3];
	SUB(dir, Q, cam_pos);
	double dot=DOT(dir, cam_vec);

	double l_n[4];
	double dir_n[3];

	/*
	SUB(dir_n, Q1, cam_pos);
	l_n[0]=length(dir_n);

	SUB(dir_n, Q2, cam_pos);
	l_n[1]=length(dir_n);

	SUB(dir_n, Q3, cam_pos);
	l_n[2]=length(dir_n);

	SUB(dir_n, Q4, cam_pos);
	l_n[3]=length(dir_n);
	*/

	//if(l_n[0]<l_n[2])
	{
		memcpy(Qt, Q1, sizeof(double)*3);
		Qt[2]=z1;
		SUB(dir_n, Qt, cam_pos);
		l_n[0]=length(dir_n);
	}

	//if(l_n[1]<l_n[3])
	{
		memcpy(Qt, Q2, sizeof(double)*3);
		Qt[2]=z2;
		SUB(dir_n, Qt, cam_pos);
		l_n[1]=length(dir_n);
	}

	//if(l_n[2]<l_n[0])
	{
		memcpy(Qt, Q3, sizeof(double)*3);
		Qt[2]=z3;
		SUB(dir_n, Qt, cam_pos);
		l_n[2]=length(dir_n);
	}

	//if(l_n[3]<l_n[1])
	{
		memcpy(Qt, Q4, sizeof(double)*3);
		Qt[2]=z4;
		SUB(dir_n, Qt, cam_pos);
		l_n[3]=length(dir_n);
	}

	double l=length(dir);
	double sz=scale_dim/((double)(1<<LOD)*l*fov);

	double diameter=scale_dim/((double)(1<<LOD));

	double const k=0.05;
	
	double d_lod_plus_1=scale_dim/((double)(1<<(LOD))*k*fov);
	double d_lod=scale_dim/((double)(1<<(LOD-1))*k*fov);

	double lod_scale[4]={1.0, 1.0, 1.0, 1.0};
	double one_minus_lod_scale[4]={0.0, 0.0, 0.0, 0.0};

	bool bClipped = false;
	if(bContinuous && LOD>0)
	{
		for(int i=0; i<4; i++)
		{
			lod_scale[i]=(d_lod-l_n[i])/(d_lod-d_lod_plus_1);

			lod_scale[i]=min(lod_scale[i], 1.0);
			lod_scale[i]=max(lod_scale[i], 0.0);

			one_minus_lod_scale[i]=1.0-lod_scale[i];
		}
	}

	if(bLOD)
	{
		bRefine = sz > k;
	}

	if(bFrustumCulling&&LOD<iMaxCull)
	{
		bInFront=( in_view(Q) ||
		           in_view(Q1) ||
		           in_view(Q2) ||
		           in_view(Q3) ||
		           in_view(Q4) );
	}

	if(bInFront)
	{
		if(pT->is_split() && LOD<iMaxLOD && bRefine)
		{
			double Z1=(Q1[2]+Q2[2])*0.5;
			double Z2=(Q2[2]+Q3[2])*0.5;
			double Z3=(Q3[2]+Q4[2])*0.5;
			double Z4=(Q4[2]+Q1[2])*0.5;

			Q[2]=(Q1[2]+Q2[2]+Q3[2]+Q4[2])*0.25;

			draw_quad( pT->quads[0][0], LOD+1,
			           Q1[2], Z1, Q[2], Z4);

			draw_quad( pT->quads[0][1], LOD+1,
			           Z1, Q2[2], Z2, Q[2]);

			draw_quad( pT->quads[1][1], LOD+1,
			           Q[2], Z2, Q3[2], Z3);

			draw_quad( pT->quads[1][0], LOD+1,
			           Z4, Q[2], Z3, Q4[2]);
		}
		else// if(bInFront)
		{	
			Q1[2]=Q1[2]*lod_scale[0] + z1*one_minus_lod_scale[0];
			Q2[2]=Q2[2]*lod_scale[1] + z2*one_minus_lod_scale[1];
			Q3[2]=Q3[2]*lod_scale[2] + z3*one_minus_lod_scale[2];
			Q4[2]=Q4[2]*lod_scale[3] + z4*one_minus_lod_scale[3];

#define LUM(v) glColor3f(v,v,v)

			if(bClipped)
				glColor3f(1,0,0);
			else
				glColor3f(1,1,1);
			//LUM(one_minus_lod_scale[0]);
			glTexCoord2f(pT->x1, pT->y1);
			glVertex3dv(Q1);

			//LUM(one_minus_lod_scale[1]);
			glTexCoord2f(pT->x2, pT->y1);
			glVertex3dv(Q2);

			//LUM(one_minus_lod_scale[2]);
			glTexCoord2f(pT->x2, pT->y2);
			glVertex3dv(Q3);

			//LUM(one_minus_lod_scale[3]);
			glTexCoord2f(pT->x1, pT->y2);
			glVertex3dv(Q4);
		}
	}
}
