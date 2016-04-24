#include <GL/glew.h>
#include "positiongen.h"

#include "shaderloader/shaderloader.h"
#include "debuggl/debuggl.h"

#include <stdio.h>

static struct {
	GLuint program;
	GLint  w_output;
	GLint  u_time;
} positiongen;

int positiongen_load(void)
{
	debuggl_check( "pre positiongen_load check" );

	char const * const paths[] = { "positiongen.glsl", 0 };
	shader_program_sources const sources[] = { {GL_COMPUTE_SHADER, paths}, {0,0} };

	if( positiongen.program ) {
		debuggl_check( glDeleteProgram(positiongen.program) );
		positiongen.program = 0;
	}

	positiongen.program = shader_program_load_from_files(sources);
	if( !positiongen.program ) {
		fprintf(stderr, "%s failed\n", __func__);
		return -2;
	}

	debuggl_check( positiongen.w_output  = glGetProgramResourceIndex(positiongen.program, GL_SHADER_STORAGE_BLOCK, "w_output") );
	debuggl_check( positiongen.u_time    = glGetProgramResourceLocation(positiongen.program, GL_UNIFORM, "u_time") );

	return 0;
}

int positiongen_launch(
	float t,
	GLuint vbo,
	int width,
	int height )
{
	debuggl_check( glUseProgram(positiongen.program) );
	debuggl_check( glUniform1f(positiongen.u_time, t) );
	debuggl_check( glBindBufferBase(GL_SHADER_STORAGE_BUFFER, positiongen.w_output, vbo) );
	debuggl_check( glDispatchCompute(width/16, height/16, 1) );
	debuggl_check( glBindBufferBase(GL_SHADER_STORAGE_BUFFER, positiongen.w_output, 0) );
	debuggl_check( glUseProgram(0) );
}
