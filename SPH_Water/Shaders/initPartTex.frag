#version 330 core

uniform sampler2D texUnit;
uniform sampler2D texUnit2;
uniform float texSize;

in vec2 inTexCoord;
out vec4 out_Color;

void main(){

	float texcoordX = inTexCoord.x;
	float texcoordY = inTexCoord.y;

	if(mod(texcoordX, 12) == 0 && mod(texcoordY, 12) == 0){
		out_Color = vec4(1.0, 1.0, 0.0, 1.0);
	}
}