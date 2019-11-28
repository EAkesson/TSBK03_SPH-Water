#version 330 core

uniform sampler2D texUnit;
uniform sampler2D texUnit2;
uniform float texSize;

in vec2 inTexCoord;
out vec4 out_Color;

void main(){

	float texcoordX = inTexCoord.x*4000;
	float texcoordY = inTexCoord.y*4000;

	//if(mod(texcoordX, 2.0f) == 0 && mod(texcoordY, 2.0f) == 0){
	if(mod(texcoordX, 2.0f) < 1.08 && mod(texcoordX, 2.0f) > 1.0 &&
		mod(texcoordY, 2.0f) < 1.08 && mod(texcoordY, 2.0f) > 1.0){
		out_Color = vec4(1.0, 0.0, 0.0, 1.0);
	}else{
		out_Color = vec4(0.0, 0.0, 0.0, 0.0);
	}
}