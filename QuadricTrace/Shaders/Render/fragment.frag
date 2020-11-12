#version 450

layout(location = 0) in vec2 fs_in_tex;
out vec4 fs_out_col;

uniform sampler2D frame;

void main()
{
	fs_out_col = vec4(texture(frame, fs_in_tex).rgb, 1);
}
