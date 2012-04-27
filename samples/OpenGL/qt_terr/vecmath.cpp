#include <math.h>
#include "vecmath.h"

double DOT(double a[3], double b[3])
{
	return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];
}

void ADD(double out[3], double a[3], double b[3])
{
	out[0]=a[0]+b[0];
	out[1]=a[1]+b[1];
	out[2]=a[2]+b[2];
}

void SUB(double out[3], double a[3], double b[3])
{
	out[0]=a[0]-b[0];
	out[1]=a[1]-b[1];
	out[2]=a[2]-b[2];
}

void SCALE(double out[3], double v[3], double scalar)
{
	out[0]=v[0]*scalar;
	out[1]=v[1]*scalar;
	out[2]=v[2]*scalar;
}

double length(double v[3])
{
	return (double)sqrt(v[0]*v[0]+v[1]*v[1]+v[2]*v[2]);
}

void normalize(double out[3], double in[3])
{
	double k=1.0f/length(in);
	SCALE(out, in, k);
}