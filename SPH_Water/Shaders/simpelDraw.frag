#version 330 core

uniform sampler2D texUnit0;
uniform sampler2D texUnit1;

in vec2 outTexCoord;
out vec4 out_Color;

void main()
{
	out_Color = texture(texUnit0, outTexCoord);
}