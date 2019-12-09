#version 330 core

uniform sampler2D texPos;
uniform sampler2D texVel;
uniform int texSize;
uniform int windowWidth;
uniform int windowHeight;

in vec2 outTexCoord;
out vec4 out_Color;

void main()
{
	out_Color = texture(texPos, outTexCoord);
}