#version 430

uniform vec4 u_color;
out vec4 o_fragcolor;

void main()
{
	o_fragcolor = u_color;
}
