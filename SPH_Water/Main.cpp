#include <stdio.h>
#include <stdlib.h>

#include <sstream>
#include <vector>
#include <algorithm>

#include <GL/glew.h>

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
FBOstruct *fboParticle1, *fboParticle2, *fboScreen, *fboParticlePos1, *fboParticlePos2, *fboParticleVel1, *fboParticleVel2;
GLuint physicShader = 0, renderShader = 0, initPartTexShader = 0, calcNewPosShader = 0, simpelDrawShader=0, loadTexToFBOShader = 0,
		emptyTextureShader = 0, enQuickie= 0, SPH_Shader = 0, renderWaterBallsShader = 0;

// For fps counter
double lastTime = 0.0;
int nbFrames = 0;

const int MaxParticles = 10000;
//Particle ParticlesContainer[MaxParticles];
//int nextParticle = 0;
const int WindowWidth = 1280;
const int WindowHeight = 720;
const int textureSize = 4000;

static GLbyte posTexture[textureSize][textureSize][4];
static GLbyte velTexture[textureSize][textureSize][4];
static GLuint posTexName, velTexName, posTexImName, velTexImName;
static GLuint billboard_vertex_buffer;
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

			velTexture[x][y][0] = static_cast<GLbyte>(0.0);
			velTexture[x][y][1] = static_cast<GLbyte>(0.0);
			velTexture[x][y][2] = static_cast<GLbyte>(0.0);
			velTexture[x][y][3] = static_cast<GLbyte>(0.0);
		}
	}	
	
	glGenTextures(1, &posTexImName);
	glBindTexture(GL_TEXTURE_2D, posTexImName);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, textureSize, textureSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, textureSize, textureSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, tempTex);

	glGenTextures(1, &velTexImName);
	glBindTexture(GL_TEXTURE_2D, velTexImName);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, textureSize, textureSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, textureSize, textureSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, tempTex);
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

		z = floor(((pos.z / 10.0f) * 63.0f));
		x = (int)((pos.x / 15.0) * 499.0 + (z - floor(z / 8.0) * 8.0) * 500.0);
		y = (int)((pos.y / 20.0) * 499.0 + floor(z / 8.0) * 500.0);

		posTexture[y][x][0] = static_cast<GLbyte>(pos.x*255.0);
		posTexture[y][x][1] = static_cast<GLbyte>(pos.y*255.0);
		posTexture[y][x][2] = static_cast<GLbyte>(pos.z*255.0);
		posTexture[y][x][3] = static_cast<GLbyte>(255.0);

		//Random velocity in any direction [-LO, HI]
		float HI = 1.0f, LO = -1.0f;
		vec3 vel = vec3(
			LO + static_cast<float>(rand()) / static_cast<float>(RAND_MAX / (HI - LO)),
			LO + static_cast<float>(rand()) / static_cast<float>(RAND_MAX / (HI - LO)),
			LO + static_cast<float>(rand()) / static_cast<float>(RAND_MAX / (HI - LO))
		);

		//printf("x%i, y%i \n", x, y);

		// New particle
		//velTexture[y][x][0] = static_cast<GLubyte>(0.0/*vel.x*/);
		//velTexture[y][x][1] = static_cast<GLubyte>(vel.y);
		//velTexture[y][x][2] = static_cast<GLubyte>(0.0/*vel.z*/);
		//velTexture[y][x][3] = static_cast<GLubyte>(255.0);
		velTexture[y][x][0] = static_cast<GLbyte>(0.0);
		velTexture[y][x][1] = static_cast<GLbyte>(0.0);
		velTexture[y][x][2] = static_cast<GLbyte>(0.0);
		velTexture[y][x][3] = static_cast<GLbyte>(255.0);

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

	//posTexture = new GLbyte[textureSize][textureSize][4];
	return true;
}

void initShaders() {	
	initPartTexShader = loadShaders("Shaders/initPartTex.vert", "Shaders/initPartTex.frag");
	physicShader = loadShaders("Shaders/physics.vert", "Shaders/physics.frag");
	calcNewPosShader = loadShaders("Shaders/calcNewPos.vert", "Shaders/calcNewPos.frag");
	simpelDrawShader = loadShaders("Shaders/simpelDraw.vert", "Shaders/simpelDraw.frag");
	loadTexToFBOShader = loadShaders("Shaders/loadTexToFBO.vert", "Shaders/loadTexToFBO.frag");
	emptyTextureShader = loadShaders("Shaders/emptyTexture.vert", "Shaders/emptyTexture.frag");
	enQuickie = loadShaders("Shaders/EriksQuicky.vert", "Shaders/EriksQuicky.frag");
	SPH_Shader = loadShaders("Shaders/SPHShader.vert", "Shaders/SPHShader.frag");
	renderWaterBallsShader = loadShaders("Shaders/renderWaterBalls.vert", "Shaders/renderWaterBalls.frag");
}

void initFBOs() {
	// fboParticle1 = initFBO(textureSize, textureSize, 0);	
	fboParticle2 = initFBO(textureSize, textureSize, 0);

	fboParticlePos1 = initFBO(textureSize, textureSize, 0);
	fboParticlePos2 = initFBO(textureSize, textureSize, 0);
	fboParticleVel1 = initFBO(textureSize, textureSize, 0);
	fboParticleVel2 = initFBO(textureSize, textureSize, 0);
	fboScreen = initFBO(WindowWidth, WindowHeight, 0);

}

void useFBOwithTex(GLuint shader, FBOstruct *posFBO, FBOstruct *velFBO, FBOstruct *out, float deltaTime)
/* OBS first FBO => pos, second FBO => vel*/
{ 
	glUseProgram(shader);
	// Many of these things would be more efficiently done once and for all
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glUniform1i(glGetUniformLocation(shader, "texPos"), 0);
	glUniform1i(glGetUniformLocation(shader, "texVel"), 1);
	glUniform1f(glGetUniformLocation(shader, "texSize"), textureSize); //Dont need here	
	glUniform1f(glGetUniformLocation(shader, "deltaTime"), deltaTime);
	useFBO(out, posFBO, velFBO);
}

void loadTexToFBO(FBOstruct *targetFBO, GLuint textureName) {
	glUseProgram(loadTexToFBOShader);
	useFBO(targetFBO, 0L, 0L);

	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glUniform1i(glGetUniformLocation(loadTexToFBOShader, "texPos"), 0);
	glUniform1i(glGetUniformLocation(loadTexToFBOShader, "texVel"), 1);
	glUniform1f(glGetUniformLocation(loadTexToFBOShader, "texSize"), textureSize); //Dont need here	

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, textureName);	
	glUniform1i(glGetUniformLocation(loadTexToFBOShader, "texUnit2"), 2);

	DrawModel(squareModel, loadTexToFBOShader, "in_Position", NULL, "in_TexCoord");
	glFlush();
}

void loadTempImgTexToShader(GLuint shader) {
	// Bind our texture in Texture Unit 2	
	glBindImageTexture(2, posTexImName, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	//Set our "myTextureSampler" sampler to use Texture Unit 2
	glUniform1i(glGetUniformLocation(shader, "texUnit2"), 2); //TODO: make so textunit follows pos

	// Bind our texture in Texture Unit 3	
	glBindImageTexture(3, velTexImName, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	//Set our "myTextureSampler" sampler to use Texture Unit 3
	glUniform1i(glGetUniformLocation(shader, "texUnit3"), 3); //TODO: make so textunit follows pos
}

void emptyTextures() {
	glUseProgram(emptyTextureShader);
	// Many of these things would be more efficiently done once and for all
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glUniform1i(glGetUniformLocation(emptyTextureShader, "texPos"), 0);
	glUniform1i(glGetUniformLocation(emptyTextureShader, "texVel"), 1);
	
	useFBO(fboParticle2, fboParticlePos1, 0L);

	loadTempImgTexToShader(emptyTextureShader);

	DrawModel(squareModel, emptyTextureShader, "in_Position", NULL, "in_TexCoord");
	glFlush();
}

void renderTexure(GLuint shader, FBOstruct *posFBO, FBOstruct *velFBO) {
	glUseProgram(shader);
	// Many of these things would be more efficiently done once and for all
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glUniform1i(glGetUniformLocation(shader, "texPos"), 0);
	glUniform1i(glGetUniformLocation(shader, "texVel"), 1);
	glUniform1f(glGetUniformLocation(shader, "texSize"), WindowWidth);	
	glUniform1f(glGetUniformLocation(shader, "windowWidth"), WindowWidth);
	glUniform1f(glGetUniformLocation(shader, "windowHeight"), WindowHeight);
	useFBO(0L, posFBO, velFBO);
	DrawModel(squareModel, shader, "in_Position", NULL, "in_TexCoord");
	glFlush();
}

void renderBalls(GLuint shader, GLuint partTex, FBOstruct *posFBO, FBOstruct *velFBO, mat4 ViewMatrix, mat4 ViewProjectionMatrix)
{
	glUseProgram(shader);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glUniform1i(glGetUniformLocation(shader, "texPos"), 0);
	glUniform1i(glGetUniformLocation(shader, "texVel"), 1);
	glUniform1f(glGetUniformLocation(shader, "texSize"), textureSize);
	glUniform1f(glGetUniformLocation(shader, "windowWidth"), WindowWidth);
	glUniform1f(glGetUniformLocation(shader, "windowHeight"), WindowHeight);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, partTex);
	glUniform1i(glGetUniformLocation(shader, "particleSampler"), 2);	

	glUniform3f(glGetUniformLocation(shader, "CameraRight_worldspace"), ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
	glUniform3f(glGetUniformLocation(shader, "CameraUp_worldspace"), ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);
	glUniformMatrix4fv(glGetUniformLocation(shader, "VP"), 1, GL_FALSE, &ViewProjectionMatrix[0][0]);

	GLuint loc = glGetAttribLocation(shader, "squareVertices");
	glEnableVertexAttribArray(loc);
	glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
	glVertexAttribPointer(
		loc,                  // attribute. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		0            // array buffer offset
	);

	//glVertexAttribDivisor(loc, 0);
	
	useFBO(0L, posFBO, velFBO);
	DrawModel(squareModel, shader, "in_Position", NULL, "in_TexCoord");
	glFlush();

	//glDisableVertexAttribArray(loc);
	
}

void display() {

	int framenum = 0;
	double lastTime_Local = glfwGetTime(), currentTime;
	float deltaTime;

	GLuint particleTexture = loadDDS("particle.DDS");

	// The VBO containing the 4 vertices of the particles.
	// Thanks to instancing, they will be shared by all particles.
	static const GLfloat g_vertex_buffer_data[] = {
		 -0.5f, -0.5f, 0.0f,
		  0.5f, -0.5f, 0.0f,
		 -0.5f,  0.5f, 0.0f,
		  0.5f,  0.5f, 0.0f,
	};	
	glGenBuffers(1, &billboard_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	//Upload texture to GPU		
	glGenTextures(1, &posTexName);
	glBindTexture(GL_TEXTURE_2D, posTexName);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, textureSize, textureSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, posTexture);

	glGenTextures(1, &velTexName);
	glBindTexture(GL_TEXTURE_2D, velTexName);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, textureSize, textureSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, velTexture);
	
	//spawn particles?
	loadTexToFBO(fboParticlePos1, posTexName); //TODO change tex
	loadTexToFBO(fboParticleVel1, velTexName); //TODO change tex

	useFBOwithTex(enQuickie, fboParticlePos1, fboParticleVel1, fboParticlePos2, 0.0);
	DrawModel(squareModel, enQuickie, "in_Position", NULL, "in_TexCoord");
	glFlush();
	
	do
	{
		calcFPS();
		currentTime = glfwGetTime();
		deltaTime = (float)(currentTime - lastTime_Local);
		lastTime_Local = currentTime;

		computeMatricesFromInputs();
		mat4 ProjectionMatrix = getProjectionMatrix();
		mat4 ViewMatrix = getViewMatrix();
		mat4 ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
			// We will need the camera's position in order to sort the particles
			// w.r.t the camera's distance.
			// There should be a getCameraPosition() function in common/controls.cpp, 
			// but this works too.
			//glm::vec3 CameraPosition(glm::inverse(ViewMatrix)[3]);
		
		
		useFBOwithTex(SPH_Shader, fboParticlePos2, fboParticleVel1, fboParticleVel2, deltaTime);
		DrawModel(squareModel, SPH_Shader, "in_Position", NULL, "in_TexCoord");
		glFlush();


		useFBOwithTex(calcNewPosShader, fboParticlePos2, fboParticleVel2, fboParticlePos1, deltaTime);
		loadTempImgTexToShader(calcNewPosShader);
		DrawModel(squareModel, calcNewPosShader, "in_Position", NULL, "in_TexCoord");
		glFlush();


		loadTexToFBO(fboParticlePos2, posTexImName);

		loadTexToFBO(fboParticleVel1, velTexImName);

		emptyTextures();
		

		//renderTexure(simpelDrawShader, fboParticlePos2, 0L);
		//renderTexure(simpelDrawShader, fboParticleVel1, 0L);
		renderBalls(renderWaterBallsShader, particleTexture, fboParticlePos2, fboParticleVel1, ViewMatrix, ViewProjectionMatrix);

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
	
	initShaders();
	initFBOs();
	initTexture();
	spawnParticle(100);

	squareModel = LoadDataToModel(
		square, NULL, squareTexCoord, NULL,
		squareIndices, 4, 6);
	
	display();

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}