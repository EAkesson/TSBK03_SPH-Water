#version 420 core

uniform sampler2D texPos;
uniform sampler2D texVel;

uniform layout(rgba32f) image2D texUnit2;
uniform layout(rgba32f) image2D texUnit3;

uniform float texSize;
uniform float deltaTime;

in vec2 outTexCoord;
out vec4 out_Color;

void clampRelPos(inout vec3 pos)
{	
	if(pos.x < 0.0)
		pos.x = 0.0;
	else if(pos.x > 15.0)
		pos.x = 15.0;
	
	if(pos.y < 0.0)
		pos.y = 0.0;
	else if(pos.y > 20.0)
		pos.y = 20.0;

	if(pos.z < 0.0)
		pos.z = 0.0;
	else if(pos.z > 10.0)
		pos.z = 10.0;
}

void RelToTex(in vec3 relPos, out ivec2 texPos)
{
	clampRelPos(relPos);

	float z = floor(((relPos.z / 10.0) * 63.0));
	texPos.x = int(floor((relPos.x / 15.0) * 500.0) + (z - floor(z / 8.0) * 8.0) * 500.0);
	texPos.y = int(floor((relPos.y / 20.0) * 500.0) + floor(z / 8.0) * 500.0);
}

void TexToRel(in ivec2 texPos, out vec3 relPos)
{
	relPos.z = float(floor(texPos.x/500) + floor(texPos.y/500)*8) / 63.0 * 10.0;
	relPos.x = float(texPos.x - floor(texPos.x/500)*500) / 500.0 * 15.0;
	relPos.y = float(texPos.y - floor(texPos.y/500)*500) / 500.0 * 20.0;
}

void main(){
//	ivec2 texPixCoord;
//	texPixCoord.x = int(floor(outTexCoord.x*4000));
//	texPixCoord.y = int(floor(outTexCoord.y*4000));

//	vec4 particleSpeed = texture(texUnit0, outTexCoord);
//	if(particleSpeed.a > 0.5){
//		ivec2 texPixCoord;
//		vec3 realPos;
//		texPixCoord.x = int(floor(outTexCoord.x*4000));
//		texPixCoord.y = int(floor(outTexCoord.y*4000));
//
//		TexToRel(texPixCoord, realPos);
//
//		vec3 newrealPos = realPos + vec3(texture(texUnit0, outTexCoord)) * deltaTime * 0.01;
//		ivec2 newTexPixCoord = texPixCoord + ivec2(1,1);
//
//		if(newTexPixCoord.x > 3999){
//			newTexPixCoord.x = 3999;
//		}
//		if(newTexPixCoord.y > 3999){
//			newTexPixCoord.y = 3999;
//		}
//
//		//clampTexPos(newTexPixCoord);
//		RelToTex(newrealPos, newTexPixCoord);
//
//		imageStore(texUnit2, newTexPixCoord, particleSpeed);
//
//		out_Color = texture(texUnit0, outTexCoord);
//	}
//
//	out_Color = texture(texUnit0, outTexCoord);
	
	vec4 particleSpeed = texture(texVel, outTexCoord);
	if(true/*particleSpeed.a > 0.5*/)
	{
		ivec2 texPixCoord;
		texPixCoord.x = int(floor(outTexCoord.x*4000));
		texPixCoord.y = int(floor(outTexCoord.y*4000));

		vec3 realPos = vec3(texture(texPos, outTexCoord));
		//TexToRel(texPixCoord, realPos);

		vec3 newrealPos = realPos + vec3(texture(texVel, outTexCoord)) * deltaTime * 0.01;
		ivec2 newTexPixCoord;
		RelToTex(newrealPos, newTexPixCoord);
	
		//newTexPixCoord = texPixCoord + ivec2((newTexPixCoord - texPixCoord) * 0.0008);

		imageStore(texUnit2, newTexPixCoord, vec4(newrealPos, 1.0));
		imageStore(texUnit3, newTexPixCoord, texture(texVel, outTexCoord));

		out_Color = vec4(0.0);		
	}

}