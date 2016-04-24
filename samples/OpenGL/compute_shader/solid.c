#include <GL/glew.h>
#include "solid.h"

#include "shaderloader/shaderloader.h"
#include "debuggl/debuggl.h"

#include <stdio.h>

static struct {
	GLint program;
	GLint u_modelview;
	GLint u_projection;
	GLint u_color;
} solid;

int solid_load(void)
{
	debuggl_check( "pre solid_load check" );

	char const * const paths_vs[] = { "mvp.vs.glsl", 0 };
	char const * const paths_fs[] = { "solid.fs.glsl", 0 };
	shader_program_sources const sources[] = {
		{GL_VERTEX_SHADER,   paths_vs},
		{GL_FRAGMENT_SHADER, paths_fs},
		{0,0}
	};

	if( solid.program ) {
		debuggl_check( glDeleteProgram(solid.program) );
		solid.program = 0;
	}

	solid.program = shader_program_load_from_files(sources);
	if( !solid.program ) {
		fprintf(stderr, "%s failed\n", __func__);
		return -2;
	}

	debuggl_check( solid.u_modelview  = glGetProgramResourceLocation(solid.program, GL_UNIFORM, "u_modelview") );
	debuggl_check( solid.u_projection = glGetProgramResourceLocation(solid.program, GL_UNIFORM, "u_projection") );
	debuggl_check( solid.u_color      = glGetProgramResourceLocation(solid.program, GL_UNIFORM, "u_color") );

	return 0;
}

int solid_draw(
	GLenum primitive,
	GLuint vao,
	GLsizei count,
	GLfloat const * const color,
	GLfloat const * const modelview,
	GLfloat const * const projection )
{
	debuggl_check( (void)"pre solid_draw check" );

	debuggl_check( glBindVertexArray(vao) );
	debuggl_check( glUseProgram(solid.program) );
	debuggl_check( glUniform4fv(solid.u_color, 1, color) );
	debuggl_check( glUniformMatrix4fv(solid.u_modelview, 1, GL_FALSE, modelview) );
	debuggl_check( glUniformMatrix4fv(solid.u_projection, 1, GL_FALSE, projection) );
	debuggl_check( glDrawArrays(primitive, 0, count) );
	debuggl_check( glUseProgram(0) );
	debuggl_check( glBindVertexArray(0) );
}
