#pragma once
#ifndef SOLID_H
#define SOLID_H

#include <GL/gl.h>

int solid_load(void);

int solid_draw(
	GLenum primitive,
	GLuint vao,
	GLsizei count,
	GLfloat const * const color,
	GLfloat const * const modelview,
	GLfloat const * const projection );

#endif/*POSITIONGEN_H*/
