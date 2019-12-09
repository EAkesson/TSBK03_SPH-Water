#version 330 core

uniform sampler2D texPos;
uniform sampler2D texVel;
uniform sampler2D texUnit2;
uniform int texSize;

in vec2 outTexCoord;
out vec4 out_Color;

void main(){
	out_Color = texture(texUnit2, outTexCoord);
}