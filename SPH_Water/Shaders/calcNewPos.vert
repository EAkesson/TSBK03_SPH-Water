#version 330 core

in vec3 in_Position;
in vec2 in_TexCoord;

out vec2 inTexCoord;

void main(void)
{
	inTexCoord = in_TexCoord;
	gl_Position = vec4(in_Position, 1.0);
}
