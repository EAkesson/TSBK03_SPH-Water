#version 330 core

uniform sampler2D texUnit;
uniform sampler2D texUnit2;
uniform float texSize_W;
uniform float texSize_H;
uniform float deltaTime;

in vec2 outTexCoord;
out vec4 out_Color;

void main(){

	//float texcoordX = outTexCoord.x*4000;
	//float texcoordY = outTexCoord.y*4000;

	out_Color = texture(texUnit, outTexCoord);
}