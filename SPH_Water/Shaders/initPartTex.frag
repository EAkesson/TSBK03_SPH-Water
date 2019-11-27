#version 330 core

uniform sampler2D texUnit;
uniform sampler2D texUnit2;
uniform float texSize;

in vec2 inTexCoord;
out vec4 out_Color;

void main(){

	float texcoordX = inTexCoord.x*512;
	float texcoordY = inTexCoord.y*512;

	//if(mod(texcoordX, 2.0f) == 0 && mod(texcoordY, 2.0f) == 0){
	if(mod(texcoordX, 2.0f) == 1.0f){
		out_Color = vec4(1.0, 1.0, 1.0, 1.0);
	}else{
		out_Color = vec4(0.0, 0.0, 0.0, 1.0);
	}
}