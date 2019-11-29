#version 330 core

uniform sampler2D texUnit0;
uniform sampler2D texUnit1;
uniform float texSize_H;
uniform float texSize_W;

in vec2 inTexCoord;
out vec4 out_Color;

void main(){

	float texcoordX = inTexCoord.x*4000;
	float texcoordY = inTexCoord.y*4000;

	if(texcoordX < 100 && texcoordY < 1000){
		if(mod(texcoordX, 2.0f) > 1.0 && mod(texcoordY, 2.0f) > 1.0){
	//if(mod(texcoordX, 2.0f) < 1.1 && mod(texcoordX, 2.0f) > 1.0 &&
		//mod(texcoordY, 2.0f) < 1.1 && mod(texcoordY, 2.0f) > 1.0){
			out_Color = vec4(1.0, 0.0, 0.0, 1.0);
		}else{
			out_Color = vec4(0.0, 0.0, 0.0, 0.0);
		}
	}
	
}