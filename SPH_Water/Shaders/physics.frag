#version 330 core

uniform sampler2D texUnit;
uniform sampler2D texUnit2;
uniform float texSize_W;
uniform float texSize_H;
uniform float deltaTime;

in vec2 outTexCoord;
out vec4 out_Color;

void main(){
	
	vec4 newSpeedvector = texture(texUnit, outTexCoord);

	if(newSpeedvector.a >= 0.9){
		if(newSpeedvector.y < 5.0){
			newSpeedvector.y += 9.82 * deltaTime * 0.01; 
		}else{
			newSpeedvector.z += 9.82 * deltaTime * 0.1; 
		}
		
	}

	out_Color = newSpeedvector;
}