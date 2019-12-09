#version 420 core

uniform sampler2D texPos;
uniform sampler2D texVel;

uniform layout(rgba32f) image2D texUnit2;
uniform layout(rgba32f) image2D texUnit3;

in vec2 outTexCoord;
out vec4 out_Color;

void main(){

	ivec2 texPixCoord;
	texPixCoord.x = int(floor(outTexCoord.x*4000));
	texPixCoord.y = int(floor(outTexCoord.y*4000));

	imageStore(texUnit2, texPixCoord, vec4(0.0));
	imageStore(texUnit3, texPixCoord, vec4(0.0));

	out_Color = vec4(0.0);
}