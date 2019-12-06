#version 330 core

uniform sampler2D texUnit;
uniform sampler2D texUnit2;
uniform float texSize;

in vec2 inTexCoord;
out vec4 out_Color;

void main(){

	float texcoordX = inTexCoord.x*4000;
	float texcoordY = inTexCoord.y*4000;

	out_Color = vec4(0.0, 0.0, 0.0, 0.0);
}