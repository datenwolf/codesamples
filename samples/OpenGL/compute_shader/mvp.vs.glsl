#version 430

layout(location = 0) in vec4 a_position;

uniform mat4 u_modelview;
uniform mat4 u_projection;

void main()
{
	const mat4 mvp = u_projection * u_modelview;
	gl_Position = mvp * a_position;
}
