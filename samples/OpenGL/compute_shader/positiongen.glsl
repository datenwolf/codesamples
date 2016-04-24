#version 430

layout( local_size_x = 16, local_size_y = 16 ) in;
layout(std430) buffer;
layout(binding = 0) buffer w_output {
	vec4 data[];
};

uniform float u_time;

void main()
{
	uvec3 GlobalSize = gl_WorkGroupSize * gl_NumWorkGroups;
	uint GlobalInvocationIndex =
		  GlobalSize.x * GlobalSize.y * gl_GlobalInvocationID.z
		+ GlobalSize.x * gl_GlobalInvocationID.y
		+ gl_GlobalInvocationID.x;

	vec2 p = 2*vec2(gl_GlobalInvocationID.xy)/GlobalSize.xy - 1;

	float z = sin(10.*p.x + u_time) + sin(30.*p.y + 0.1*u_time);

	data[GlobalInvocationIndex] = vec4(p, z*0.25, 1);
}
