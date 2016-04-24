#pragma once
#ifndef POSITIONGEN_H
#define POSITIONGEN_H

#include <GL/gl.h>

int positiongen_load(void);

int positiongen_launch(
	float t,
	GLuint vbo,
	int width,
	int height );

#endif/*POSITIONGEN_H*/
