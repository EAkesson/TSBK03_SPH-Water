#version 420 core

uniform sampler2D texPos;
uniform sampler2D texVel;

in vec3 in_Position;
in vec2 in_TexCoord;
in vec3 squareVertices;

out vec2 outTexCoord;
out vec2 UV;

// Values that stay constant for the whole mesh.
uniform vec3 CameraRight_worldspace;
uniform vec3 CameraUp_worldspace;
uniform mat4 VP; // Model-View-Projection matrix, but without the Model (the position is in BillboardPos; the orientation depends on the camera)

float particleSize = 100.0;

void main(void)
{
	if(texture(texPos, in_TexCoord).a > 0.5){
		vec3 particleCenter_wordspace = vec3(texture(texPos, in_TexCoord));
		vec3 vertexPosition_worldspace = particleCenter_wordspace;
										+ CameraRight_worldspace * squareVertices.x * particleSize
										+ CameraUp_worldspace * squareVertices.y * particleSize;
		// Output position of the vertex
		gl_Position = VP * vec4(vertexPosition_worldspace, 1.0f);

		// UV of the vertex. No special space for this one.
		UV = /*squareVertices.xy +*/ vec2(in_Position) * vec2(0.5, 0.5);
		outTexCoord = in_TexCoord;	
		//particlecolor = color; ??
	}
	
}
