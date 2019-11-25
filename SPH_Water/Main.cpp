#include <stdio.h>
#include <stdlib.h>

#include <sstream>
#include <vector>
#include <algorithm>

#include <GL\glew.h>

#include <GLFW/glfw3.h>
GLFWwindow* window;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
using namespace glm;


#include "Externals/GL_utilities.h"
#include "Externals/loadobj.h"
#include "Externals/texture.hpp"
#include "Externals/controls.hpp"

// CPU representation of a particle
#include "Particle.h"
/*struct Particle {
	glm::vec3 pos, speed;
	unsigned char r, g, b, a; // Color
	float size, angle, weight;	
	float cameradistance; // *Squared* distance to the camera. if dead : -1.0f

	bool operator<(const Particle& that) const {
		// Sort in reverse order : far particles drawn first.
		return this->cameradistance > that.cameradistance;
	}
};*/





GLfloat square[] = {
							-1,-1,0,
							-1,1, 0,
							1,1, 0,
							1,-1, 0 };
GLfloat squareTexCoord[] = {
							 0, 0,
							 0, 1,
							 1, 1,
							 1, 0 };
GLuint squareIndices[] = { 0, 1, 2, 0, 2, 3 };

Model* squareModel;


//----------------------Globals-------------------------------------------------
FBOstruct *fboVelocity1, *fboVelocity2, *fboPos1, *fboPos2, *fboScreen;
GLuint physicShader = 0, renderShader = 0;

// For fps counter
double lastTime = 0.0;
int nbFrames = 0;

const int MaxParticles = 10000;
Particle ParticlesContainer[MaxParticles];
int nextParticle = 0;
const int WindowWidth = 1024;
const int WindowHeight = 768;
const int textureSize = 4000;
//---------------------------------------------------------------------------

void calcFPS()
{
	// Measure speed
	double currentTime = glfwGetTime();
	double delta = currentTime - lastTime;
	nbFrames++;
	if (delta >= 1.0) { // If last cout was more than 1 sec ago
		double fps = double(nbFrames) / delta;

		std::stringstream ss;
		ss << "SPH_Water, We promise it looks good" << " [" << fps << " FPS]";

		glfwSetWindowTitle(window, ss.str().c_str());

		nbFrames = 0;
		lastTime = currentTime;
	}
}

void SortParticles() {
	std::sort(&ParticlesContainer[0], &ParticlesContainer[MaxParticles]);
}

bool initGLFW() {
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return false;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(WindowWidth, WindowHeight, "SPH_Water, We promise it looks good", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window);

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	// Hide the mouse and enable unlimited movement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set the mouse at the center of the screen
	glfwPollEvents();
	glfwSetCursorPos(window, WindowWidth / 2, WindowHeight / 2);

	return true;
}

bool initGlew() {
	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return false;
	}
	return true;
}

bool initProgram() {

	//Init GLFW
	if (!initGLFW())
		return false;
	
	//Init Glew
	if (!initGlew())
		return false;

	return true;
}

void initShaders() {
	physicShader = loadShaders("plaintextureshader.vert", "plaintextureshader.frag");
}

void initFBOs() {
	fboVelocity1 = initFBO(textureSize, textureSize, 0);
	fboVelocity1 = initFBO(textureSize, textureSize, 0);
	fboPos1 = initFBO(textureSize, textureSize, 0);
	fboPos1 = initFBO(textureSize, textureSize, 0);
	fboScreen = initFBO(WindowWidth, WindowHeight, 0);
}

void generateNewParticles(double *delta) {
	// Generate 10 new particule each millisecond,
		// but limit this to 16 ms (60 fps), or if you have 1 long frame (1sec),
		// newparticles will be huge and the next frame even longer.
	int newparticles = (int)(*delta*1000.0);
	if (newparticles > (int)(0.016f*1000.0))
		newparticles = (int)(0.016f*1000.0);

	for (int i = 0; i < newparticles; i++) {
		if (nextParticle < MaxParticles) {
			ParticlesContainer[nextParticle].pos = glm::vec3(0, 4, -20.0f);

			float spread = 1.5f;
			glm::vec3 maindir = glm::vec3(0.0f, 10.0f, 0.0f);
			// Very bad way to generate a random direction; 
			// See for instance http://stackoverflow.com/questions/5408276/python-uniform-spherical-distribution instead,
			// combined with some user-controlled parameters (main direction, spread, etc)
			glm::vec3 randomdir = glm::vec3(
				(rand() % 2000 - 1000.0f) / 1000.0f,
				(rand() % 2000 - 1000.0f) / 1000.0f,
				(rand() % 2000 - 1000.0f) / 1000.0f
			);

			ParticlesContainer[nextParticle].speed = maindir + randomdir * spread;


			// Very bad way to generate a random color
			ParticlesContainer[nextParticle].r = rand() % 256;
			ParticlesContainer[nextParticle].g = rand() % 256;
			ParticlesContainer[nextParticle].b = rand() % 256;
			ParticlesContainer[nextParticle].a = 255;//(rand() % 256) / 3;

			ParticlesContainer[nextParticle].size = (rand() % 1000) / 2000.0f + 0.1f;

			nextParticle++;
		}
	}
}

void runFBO(GLuint shader, FBOstruct *in1, FBOstruct *in2, FBOstruct *out) {
	glUseProgram(shader);

	// Many of these things would be more efficiently done once and for all
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glUniform1i(glGetUniformLocation(shader, "texUnit"), 0);
	glUniform1i(glGetUniformLocation(shader, "texUnit2"), 1);
	glUniform1f(glGetUniformLocation(shader, "texSize"), W); //??

	useFBO(out, in1, in2);
	DrawModel(squareModel, shader, "in_Position", NULL, "in_TexCoord");

	glFlush();
}

int main(void)
{
	//initProgram
	if (!initProgram())
		return -1;

	// Dark grey background
	glClearColor(0.1f, 0.1f, 0.1f, 0.0f);
	glClearDepth(1.0);
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	initShaders();
	initFBOs();
	
	//GLuint VertexArrayID;
	//glGenVertexArrays(1, &VertexArrayID);
	//glBindVertexArray(VertexArrayID);

	//// Create and compile our GLSL program from the shaders
	//GLuint programID = loadShaders("Particle.vert", "Particle.frag");

	//// Vertex shader
	//GLuint CameraRight_worldspace_ID = glGetUniformLocation(programID, "CameraRight_worldspace");
	//GLuint CameraUp_worldspace_ID = glGetUniformLocation(programID, "CameraUp_worldspace");
	//GLuint ViewProjMatrixID = glGetUniformLocation(programID, "VP");

	//// fragment shader
	//GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");


	//static GLfloat* g_particule_position_size_data = new GLfloat[MaxParticles * 4];
	//static GLubyte* g_particule_color_data = new GLubyte[MaxParticles * 4];

	//for (int i = 0; i < MaxParticles; i++) {		
	//	ParticlesContainer[i].cameradistance = -1.0f;
	//}



	//GLuint Texture = loadDDS("particle.DDS");

	//// The VBO containing the 4 vertices of the particles.
	//// Thanks to instancing, they will be shared by all particles.
	//static const GLfloat g_vertex_buffer_data[] = {
	//	 -0.5f, -0.5f, 0.0f,
	//	  0.5f, -0.5f, 0.0f,
	//	 -0.5f,  0.5f, 0.0f,
	//	  0.5f,  0.5f, 0.0f,
	//};
	//GLuint billboard_vertex_buffer;
	//glGenBuffers(1, &billboard_vertex_buffer);
	//glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	//// The VBO containing the positions and sizes of the particles
	//GLuint particles_position_buffer;
	//glGenBuffers(1, &particles_position_buffer);
	//glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	//// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	//glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

	//// The VBO containing the colors of the particles
	//GLuint particles_color_buffer;
	//glGenBuffers(1, &particles_color_buffer);
	//glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	//// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	//glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);



	//double lastTime = glfwGetTime();
	
	//do
	//{
	//	// Clear the screen
	//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//	double currentTime = glfwGetTime();
	//	double delta = currentTime - lastTime;
	//	lastTime = currentTime;


	//	// Calc FPS and set title
	//	calcFPS();

	//	computeMatricesFromInputs();
	//	glm::mat4 ProjectionMatrix = getProjectionMatrix();
	//	glm::mat4 ViewMatrix = getViewMatrix();

	//	// We will need the camera's position in order to sort the particles
	//	// w.r.t the camera's distance.
	//	// There should be a getCameraPosition() function in common/controls.cpp, 
	//	// but this works too.
	//	glm::vec3 CameraPosition(glm::inverse(ViewMatrix)[3]);

	//	glm::mat4 ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

	//	generateNewParticles(&delta);


	//	// Simulate all particles		
	//	for (int ParticlesCount = 0; ParticlesCount < nextParticle; ParticlesCount++) {

	//		Particle& p = ParticlesContainer[ParticlesCount]; // shortcut

	//		// Simulate simple physics : gravity only, no collisions
	//		p.speed += glm::vec3(0.0f, -9.82f, 0.0f) * (float)delta * 0.5f;
	//		p.pos += p.speed * (float)delta;
	//		p.pos.y = max(p.pos.y, -10.0f);
	//		p.cameradistance = glm::length2(p.pos - CameraPosition);			

	//		// Fill the GPU buffer
	//		g_particule_position_size_data[4 * ParticlesCount + 0] = p.pos.x;
	//		g_particule_position_size_data[4 * ParticlesCount + 1] = p.pos.y;
	//		g_particule_position_size_data[4 * ParticlesCount + 2] = p.pos.z;

	//		g_particule_position_size_data[4 * ParticlesCount + 3] = p.size;

	//		g_particule_color_data[4 * ParticlesCount + 0] = p.r;
	//		g_particule_color_data[4 * ParticlesCount + 1] = p.g;
	//		g_particule_color_data[4 * ParticlesCount + 2] = p.b;
	//		g_particule_color_data[4 * ParticlesCount + 3] = p.a;

	//		//ParticlesCount++;		
	//	}

	//	SortParticles();


	//	//printf("%d ",ParticlesCount);		


	//	// Update the buffers that OpenGL uses for rendering.
	//	// There are much more sophisticated means to stream data from the CPU to the GPU, 
	//	// but this is outside the scope of this tutorial.
	//	// http://www.opengl.org/wiki/Buffer_Object_Streaming


	//	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	//	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	//	glBufferSubData(GL_ARRAY_BUFFER, 0, nextParticle * sizeof(GLfloat) * 4, g_particule_position_size_data);

	//	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	//	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	//	glBufferSubData(GL_ARRAY_BUFFER, 0, nextParticle * sizeof(GLubyte) * 4, g_particule_color_data);


	//	glEnable(GL_BLEND);
	//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//	// Use our shader
	//	glUseProgram(programID);

	//	// Bind our texture in Texture Unit 0
	//	glActiveTexture(GL_TEXTURE0);
	//	glBindTexture(GL_TEXTURE_2D, Texture);
	//	// Set our "myTextureSampler" sampler to use Texture Unit 0
	//	glUniform1i(TextureID, 0);

	//	// Same as the billboards tutorial
	//	glUniform3f(CameraRight_worldspace_ID, ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
	//	glUniform3f(CameraUp_worldspace_ID, ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);

	//	glUniformMatrix4fv(ViewProjMatrixID, 1, GL_FALSE, &ViewProjectionMatrix[0][0]);

	//	// 1rst attribute buffer : vertices
	//	glEnableVertexAttribArray(0);
	//	glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
	//	glVertexAttribPointer(
	//		0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
	//		3,                  // size
	//		GL_FLOAT,           // type
	//		GL_FALSE,           // normalized?
	//		0,                  // stride
	//		(void*)0            // array buffer offset
	//	);

	//	// 2nd attribute buffer : positions of particles' centers
	//	glEnableVertexAttribArray(1);
	//	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	//	glVertexAttribPointer(
	//		1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
	//		4,                                // size : x + y + z + size => 4
	//		GL_FLOAT,                         // type
	//		GL_FALSE,                         // normalized?
	//		0,                                // stride
	//		(void*)0                          // array buffer offset
	//	);

	//	// 3rd attribute buffer : particles' colors
	//	glEnableVertexAttribArray(2);
	//	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	//	glVertexAttribPointer(
	//		2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
	//		4,                                // size : r + g + b + a => 4
	//		GL_UNSIGNED_BYTE,                 // type
	//		GL_TRUE,                          // normalized?    *** YES, this means that the unsigned char[4] will be accessible with a vec4 (floats) in the shader ***
	//		0,                                // stride
	//		(void*)0                          // array buffer offset
	//	);

	//	// These functions are specific to glDrawArrays*Instanced*.
	//	// The first parameter is the attribute buffer we're talking about.
	//	// The second parameter is the "rate at which generic vertex attributes advance when rendering multiple instances"
	//	// http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribDivisor.xml
	//	glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
	//	glVertexAttribDivisor(1, 1); // positions : one per quad (its center)                 -> 1
	//	glVertexAttribDivisor(2, 1); // color : one per quad                                  -> 1

	//	// Draw the particules !
	//	// This draws many times a small triangle_strip (which looks like a quad).
	//	// This is equivalent to :
	//	// for(i in ParticlesCount) : glDrawArrays(GL_TRIANGLE_STRIP, 0, 4), 
	//	// but faster.
	//	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, nextParticle);

	//	glDisableVertexAttribArray(0);
	//	glDisableVertexAttribArray(1);
	//	glDisableVertexAttribArray(2);

	//	// Swap buffers
	//	glfwSwapBuffers(window);
	//	glfwPollEvents();

	//} // Check if the ESC key was pressed or the window was closed
	//while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
	//	glfwWindowShouldClose(window) == 0);


	//delete[] g_particule_position_size_data;

	//// Cleanup VBO and shader
	//glDeleteBuffers(1, &particles_color_buffer);
	//glDeleteBuffers(1, &particles_position_buffer);
	//glDeleteBuffers(1, &billboard_vertex_buffer);
	//glDeleteProgram(programID);
	//glDeleteTextures(1, &Texture);
	//glDeleteVertexArrays(1, &VertexArrayID);


	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}