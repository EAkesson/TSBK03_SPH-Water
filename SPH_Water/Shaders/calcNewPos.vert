#version 330 core

in vec3 in_Position;
in vec2 in_TexCoord;

out vec2 inTexCoord;

void main(void)
{	
	//Check if particle
	//convert texture to worldcoordinates
	//Move according to delta time (in/gl_posistion) 
		//if outside box put it back and remove force
	//Convert back to texturecoord
	//Send newtexcoords and oldtexcoord to frag and do as usuall. (need both the old coordcolor and the new onec
	inTexCoord = in_TexCoord;
	gl_Position = vec4(in_Position, 1.0);
}
