#version 330 core

uniform sampler2D texUnit1;
uniform sampler2D texUnit2;
uniform int texSize;

in vec2 outTexCoord;
out vec4 out_Color;

void main(){
	out_Color = texture(texUnit1, outTexCoord) + texture(texUnit2, outTexCoord);
}