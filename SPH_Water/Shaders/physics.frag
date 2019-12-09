#version 330 core

uniform sampler2D texPos;
uniform sampler2D texVel;
uniform float texSize;
uniform float deltaTime;

in vec2 outTexCoord;
out vec4 out_Color;

void main(){
	
	vec4 newSpeedvector = texture(texVel, outTexCoord);

	if(newSpeedvector.a >= 0.5){		
		newSpeedvector.y += 9.82 * deltaTime * 0.01; 	//Only gravity
	}

	out_Color = newSpeedvector;
}