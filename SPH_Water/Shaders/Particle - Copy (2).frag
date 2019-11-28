#version 330 core

// Interpolated values from the vertex shaders
in vec2 UV;
in vec4 particlecolor;

// Ouput data
out vec4 particlePos;
out vec4 particleVel;

uniform sampler2D posTex;
uniform sampler2D velTex;

void main(){
	// Output color = color of the texture at the specified UV
	particlePos = texture( posTex, UV ) * particlecolor;

}