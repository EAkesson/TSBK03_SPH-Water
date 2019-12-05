#version 330 core

in vec3 in_Position;
in vec2 in_TexCoord;

uniform sampler2D texUnit;
uniform sampler2D texUnit2;
uniform float texSize_W;
uniform float texSize_H;
uniform float deltaTime;

out vec2 outTexCoord;

void RelToTex(in vec3 relPos, out vec2 texPos)
{
	float z = floor(((relPos.z / 10.0) * 63.0));
	texPos.x = int((relPos.x / 15.0) * 499.0 + (z - floor(z / 8.0) * 8.0) * 500.0);
	texPos.y = int((relPos.y / 20.0) * 499.0 + floor(z / 8.0) * 500.0);
}

void TexToRel(in vec2 texPos, out vec3 relPos)
{
	relPos.z = floor(texPos.x/500) + floor(texPos.y/500)*8.0;
	relPos.x = texPos.x - floor(texPos.x/500)*500.0;
	relPos.y = texPos.y - floor(texPos.y/500)*500.0;
}

void main(void)
{	
	vec4 speed = texture(texUnit, in_TexCoord);

	//Check if particle
	if(speed.a > 0.5){
		//convert texture to worldcoordinates
		//Move according to delta time (in/gl_posistion) 
			//if outside box put it back and remove force
		//Convert back to texturecoord
		//Send newtexcoords and oldtexcoord to frag and do as usuall. (need both the old coordcolor and the new onec
		vec3 newRelPos = in_Position + (1.0, 1.0, 1.0);//vec3(speed * deltaTime);

		outTexCoord = in_TexCoord;
		gl_Position = vec4(newRelPos, 1.0);
	}
	else
	{
		outTexCoord = in_TexCoord;
		gl_Position = vec4(in_Position, 1.0);
	}
	
	
}
