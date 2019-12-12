#version 420 core

uniform sampler2D texPos;
uniform sampler2D texVel;

uniform sampler2D particleSampler;

uniform int texSize;
uniform int windowWidth;
uniform int windowHeight;

in vec2 UV;
in vec2 outTexCoord;
out vec4 out_Color;

void main()
{
	out_Color = texture(particleSampler, UV) * texture(texVel, outTexCoord) + vec4(1.0);
}