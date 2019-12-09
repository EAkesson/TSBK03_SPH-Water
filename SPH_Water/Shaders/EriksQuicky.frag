#version 420 core

uniform sampler2D texPos;
uniform sampler2D texVel;
uniform float texSize;
uniform float delta;

in vec2 outTexCoord;
out vec4 out_Color;

void TexToRel(in ivec2 texPos, out vec3 relPos)
{
	relPos.z = float(floor(texPos.x/500) + floor(texPos.y/500)*8) / 63.0 * 10.0;
	relPos.x = float(texPos.x - floor(texPos.x/500)*500) / 500.0 * 15.0;
	relPos.y = float(texPos.y - floor(texPos.y/500)*500) / 500.0 * 20.0;
}

void main(){

	if(texture(texVel, outTexCoord).a > 0.5)
	{
		vec3 posCol;
		ivec2 coordPos = ivec2(outTexCoord*4000);
		TexToRel(coordPos, posCol);
		out_Color = vec4(posCol,1.0);
	}
	else
	{
		out_Color = vec4(0.0);
	}
	
}