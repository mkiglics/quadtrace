#version 450

const vec2 pos[4] = vec2[4](vec2(-1), vec2(1,-1), vec2(-1,1), vec2(1,1));

layout(location = 0) out vec2 vs_out_tex;

void main()
{
	vec2 p = pos[gl_VertexID];
	gl_Position = vec4(p, 0, 1);
	vs_out_tex = (p+1)/2;
}
