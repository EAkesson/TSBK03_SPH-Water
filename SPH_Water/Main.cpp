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
//#include "Externals/LittleOBJLoader.h"
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
FBOstruct *fboParticle1, *fboParticle2, *fboScreen;
GLuint physicShader = 0, renderShader = 0, initPartTexShader = 0, calcNewPosShader = 0, simpelDrawShader=0, spawnParticlesShader = 0;

// For fps counter
double lastTime = 0.0;
int nbFrames = 0;

const int MaxParticles = 10000;
//Particle ParticlesContainer[MaxParticles];
//int nextParticle = 0;
const int WindowWidth = 4000;
const int WindowHeight = 4000;
const int textureSize = 4000;

static GLbyte posTexture[textureSize][textureSize][4];
static GLuint posTexName;
//---------------------------------------------------------------------------

void calcFPS()
{
	// Measure speed
	double currentTime = glfwGetTime();
	double delta = currentTime - lastTime;
	nbFrames++;
	if (delta >= 1.0) { // If last cout was more than 1 sec ago
		double fps = static_cast<double>(nbFrames) / delta;

		std::stringstream ss;
		ss << "SPH_Water, We promise it looks good" << " [" << fps << " FPS]";

		glfwSetWindowTitle(window, ss.str().c_str());

		nbFrames = 0;
		lastTime = currentTime;
	}
}

/*void SortParticles() {
	std::sort(&ParticlesContainer[0], &ParticlesContainer[MaxParticles]);
}*/

void initTexture() {
	for (size_t x = 0; x < 4000; x++)
	{
		for (size_t y = 0; y < 4000; y++)
		{
			posTexture[x][y][0] = static_cast<GLbyte>(0.0);
			posTexture[x][y][1] = static_cast<GLbyte>(0.0);
			posTexture[x][y][2] = static_cast<GLbyte>(0.0);
			posTexture[x][y][3] = static_cast<GLbyte>(0.0);
		}
	}
}

void spawnParticle(int numParticles)
{
	int x, y;
	float z;
	for (int i = 0; i < numParticles; ++i)
	{
		// x: 3->8, y: 12->15, z: 2->8
		/*float  xRel = 3 + (rand() % (8 - 3 + 1));
		float yRel = 12 + (rand() % (15 - 12 + 1));
		float zRel = 2 + (rand() % (8 - 2 + 1));*/
		

		float xL = 3.0, xH = 8.0f;
		float yL = 12.0f, yH = 15.0f;
		float zL = 2.0f, zH = 8.0f;
		vec3 pos = vec3(
			xL + static_cast<float>(rand()) / static_cast<float>(RAND_MAX / (xH - xL)),
			yL + static_cast<float>(rand()) / static_cast<float>(RAND_MAX / (yH - yL)),
			zL + static_cast<float>(rand()) / static_cast<float>(RAND_MAX / (zH - zL))
		);

		z = floor(((pos.z / 10.0) * 63.0));
		x = (int)((pos.x / 15.0) * 499.0 + (z - floor(z / 8.0) * 8.0) * 500.0);
		y = (int)((pos.y / 20.0) * 499.0 + floor(z / 8.0) * 500.0);

		//Random velocity in any direction [-LO, HI]
		float HI = 255.0f, LO = -255.0f;
		vec3 vel = vec3(
			LO + static_cast<float>(rand()) / static_cast<float>(RAND_MAX / (HI - LO)),
			LO + static_cast<float>(rand()) / static_cast<float>(RAND_MAX / (HI - LO)),
			LO + static_cast<float>(rand()) / static_cast<float>(RAND_MAX / (HI - LO))
		);

		//printf("x%i, y%i \n", x, y);

		// New particle
		posTexture[y][x][0] = static_cast<GLubyte>(vel.x);
		posTexture[y][x][1] = static_cast<GLubyte>(vel.y);
		posTexture[y][x][2] = static_cast<GLubyte>(vel.z);
		posTexture[y][x][3] = static_cast<GLubyte>(255.0);
		/*posTexture[i][i][0] = static_cast<GLbyte>(255.0);
		posTexture[i][i][1] = static_cast<GLbyte>(255.0);
		posTexture[i][i][2] = static_cast<GLbyte>(0.0);
		posTexture[i][i][3] = static_cast<GLbyte>(255.0);*/
	}	
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
		//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set the mouse at the center of the screen
		//glfwPollEvents();
		//glfwSetCursorPos(window, WindowWidth / 2, WindowHeight / 2);

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
	//physicShader = loadShaders("plaintextureshader.vert", "plaintextureshader.frag");
	initPartTexShader = loadShaders("Shaders/initPartTex.vert", "Shaders/initPartTex.frag");
	physicShader = loadShaders("Shaders/physics.vert", "Shaders/physics.frag");
	calcNewPosShader = loadShaders("Shaders/calcNewPos.vert", "Shaders/calcNewPos.frag");
	simpelDrawShader = loadShaders("Shaders/simpelDraw.vert", "Shaders/simpelDraw.frag");
	spawnParticlesShader = loadShaders("Shaders/spawnParticles.vert", "Shaders/spawnParticles.frag");
}

void initFBOs() {
	fboParticle1 = initFBO(textureSize, textureSize, 0);
	//fboParticle1 = initFBO(textureSize, textureSize, 0);
	fboParticle2 = initFBO(textureSize, textureSize, 0);	
	fboScreen = initFBO(WindowWidth, WindowHeight, 0);

}

void runFBO(GLuint shader, FBOstruct *in1, FBOstruct *in2, FBOstruct *out, bool isBigTexture) {
	glUseProgram(shader);

	// Many of these things would be more efficiently done once and for all
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glUniform1i(glGetUniformLocation(shader, "texUnit"), 0);
	glUniform1i(glGetUniformLocation(shader, "texUnit2"), 1);
	if (!isBigTexture) {
		glUniform1f(glGetUniformLocation(shader, "texSize_W"), WindowWidth);
		glUniform1f(glGetUniformLocation(shader, "texSize_H"), WindowHeight);
	}
	else 
	{
		glUniform1i(glGetUniformLocation(shader, "texSize_W"), textureSize);
		glUniform1i(glGetUniformLocation(shader, "texSize_H"), textureSize);
	}

	

	useFBO(out, in1, in2);
	DrawModel(squareModel, shader, "in_Position", NULL, "in_TexCoord");

	glFlush();
}

void renderTexure(GLuint shader, FBOstruct *in1, FBOstruct *in2) {
	glUseProgram(shader);
	// Many of these things would be more efficiently done once and for all
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glUniform1i(glGetUniformLocation(shader, "texUnit0"), 0);
	glUniform1i(glGetUniformLocation(shader, "texUnit1"), 1);
	glUniform1f(glGetUniformLocation(shader, "texSize"), WindowWidth); //Dont need here
	glUniform1f(glGetUniformLocation(shader, "texSize_H"), WindowWidth); //Dont need here
	useFBO(0L, in1, in2);
	DrawModel(squareModel, shader, "in_Position", NULL, "in_TexCoord");
	glFlush();
}

void display() {


	//Init texture?		
		glUseProgram(spawnParticlesShader);
		useFBO(fboParticle1, 0L, 0L);
		// Many of these things would be more efficiently done once and for all
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glUniform1i(glGetUniformLocation(spawnParticlesShader, "texUnit0"), 0);
		glUniform1i(glGetUniformLocation(spawnParticlesShader, "texUnit1"), 1);
		glUniform1f(glGetUniformLocation(spawnParticlesShader, "texSize_W"), WindowWidth); //Dont need here
		glUniform1f(glGetUniformLocation(spawnParticlesShader, "texSize_H"), WindowWidth); //Dont need here

		glGenTextures(1, &posTexName);
		glBindTexture(GL_TEXTURE_2D, posTexName);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureSize, textureSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, posTexture);
		glGenerateMipmap(GL_TEXTURE_2D);		
		// Bind our texture in Texture Unit 2
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, posTexName);
		//Set our "myTextureSampler" sampler to use Texture Unit 2
		glUniform1i(glGetUniformLocation(spawnParticlesShader, "texUnit2"), 2);
		

		DrawModel(squareModel, spawnParticlesShader, "in_Position", NULL, "in_TexCoord");
		glFlush();

		

	//runFBO(initPartTexShader, fboPos1, 0L, 0L, true);
	//spawn particles?
	int framenum = 0;
	double TobySucks = glfwGetTime();
	do
	{
		//calcFPS();

		double currentTime = glfwGetTime();
		float delta = (float) currentTime - TobySucks;
		TobySucks = currentTime;
		
		glUseProgram(physicShader);
		// Many of these things would be more efficiently done once and for all
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glUniform1i(glGetUniformLocation(physicShader, "texUnit"), 0);
		glUniform1i(glGetUniformLocation(physicShader, "texUnit2"), 1);
		glUniform1f(glGetUniformLocation(physicShader, "texSize_W"), WindowWidth); //Dont need here
		glUniform1f(glGetUniformLocation(physicShader, "texSize_H"), WindowWidth); //Dont need here
		glUniform1f(glGetUniformLocation(physicShader, "deltaTime"), delta);
		useFBO(fboParticle2, fboParticle1, 0L);
		DrawModel(squareModel, initPartTexShader, "in_Position", NULL, "in_TexCoord");
		glFlush();

		glUseProgram(simpelDrawShader);
		// Many of these things would be more efficiently done once and for all
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glUniform1i(glGetUniformLocation(simpelDrawShader, "texUnit"), 0);
		glUniform1i(glGetUniformLocation(simpelDrawShader, "texUnit2"), 1);
		glUniform1f(glGetUniformLocation(simpelDrawShader, "texSize_W"), WindowWidth); //Dont need here
		glUniform1f(glGetUniformLocation(simpelDrawShader, "texSize_H"), WindowWidth); //Dont need here
		glUniform1f(glGetUniformLocation(simpelDrawShader, "deltaTime"), delta);
		useFBO(fboParticle1, fboParticle2, 0L);
		DrawModel(squareModel, simpelDrawShader, "in_Position", NULL, "in_TexCoord");
		glFlush();

		renderTexure(simpelDrawShader, fboParticle2, 0L);

		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

}

int main(void)
{
	
	//initProgram
	if (!initProgram())
		return -1;

	dumpInfo(); //shaderinfo???

	// Dark grey background
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
	glClearDepth(1.0);
	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	initTexture();
	spawnParticle(1000);
	initShaders();
	initFBOs();

	squareModel = LoadDataToModel(
		square, NULL, squareTexCoord, NULL,
		squareIndices, 4, 6);

	display();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}