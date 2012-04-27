#ifndef VECMATH_H
#define VECMATH_H

double DOT(double a[3], double b[3]);

void ADD(double out[3], double a[3], double b[3]);

void SUB(double out[3], double a[3], double b[3]);

void SCALE(double out[3], double v[3], double scalar);

double length(double v[3]);

void normalize(double out[3], double in[3]);

#endif/*VECMATH_H*/
