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


//#include <common/GL_utilities.h>
#include "Externals/GL_utilities.h"
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



// For fps counter
float lastTime = 0.0f;
int nbFrames = 0;

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
		lastTime = static_cast<float>(currentTime);
	}
}

const int MaxParticles = 100;
Particle ParticlesContainer[MaxParticles];
int nextParticle = 0;
const int WindowWidth = 1024;
const int WindowHeight = 768;

/* ********** */
// For SPH calculations
#define PI 3.14159
#define EPSILON 0.000001

float h = 5.0f;
float referenceDensity = 1.0f;
float pressureConstant = 25.0f;
float viscosity = 0.018f;
float SPHdeltaTime = 0.016f;
float particleMass = 1.0f;
float particleSize = 0.1f;

const float Poly6_Const = static_cast<float>(315.0f / (64.0f * PI * pow(h, 9)));
const float Spiky_Const = static_cast<float>(-45.0f / (PI * pow(h, 6)));
vec3 G{ 0.0f, -9.82f, 0.0 };
/* ********** */

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
	
	if (!initGlew())
		return false;

	return true;
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

			// x: 3->8, y: 12->15, z: 2->8
			/*float  xRel = 3 + (rand() % (8 - 3 + 1));
			float yRel = 12 + (rand() % (15 - 12 + 1));
			float zRel = 2 + (rand() % (8 - 2 + 1));*/


			float xL = 0.3f, xH = 0.8f;
			float yL = 1.2f, yH = 1.5f;
			float zL = 0.2f, zH = 0.8f;
			vec3 tempPos = vec3(
				xL + static_cast<float>(rand()) / static_cast<float>(RAND_MAX / (xH - xL)),
				yL + static_cast<float>(rand()) / static_cast<float>(RAND_MAX / (yH - yL)),
				zL + static_cast<float>(rand()) / static_cast<float>(RAND_MAX / (zH - zL))
			);

			//printf("Pos: x=%f, y=%f, z=%f\n", tempPos.x, tempPos.y, tempPos.z);
			
			ParticlesContainer[nextParticle].pos = tempPos;
			ParticlesContainer[nextParticle].weight = particleMass;
			ParticlesContainer[nextParticle].density = referenceDensity;

			float spread = 1.5f;
			glm::vec3 maindir = glm::vec3(0.0f, 1.0f, 0.0f);
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

			//ParticlesContainer[nextParticle].size = (rand() % 1000) / 2000.0f + 0.1f;

			ParticlesContainer[nextParticle].size = particleSize;

			nextParticle++;
		}
	}
}

void setBoundingBoxWithPosAndSpeed(Particle pA, float deltaTime)
{
	pA.speed += static_cast<float>(deltaTime) * ((pA.force + pA.force2) / pA.density + G) * 0.01f;
	
	vec3 tempPos = pA.pos + static_cast<float>(deltaTime) * pA.speed;

	//printf("Pos: %f, %f, %f\n", tempPos.x, tempPos.y, tempPos.z);

	vec3 xNorm{ 1.0f, 1.0f, 0.0f }, yNorm{ 0.0f, 1.0f, 0.0f }, zNorm{ 0.0f, 0.0f, 1.0f };
	//xNorm *= 0.1f;
	//yNorm *= 0.1f;
	//zNorm *= 0.1f;

	if (tempPos.x < 0.0f)
	{
		//pA.speed *= -1.0f;

		pA.speed -= 2.0f * dot(normalize(pA.speed), xNorm) * xNorm;// *.1f;
		//pA.pos.x = max(pA.pos.x, 0.0f);
	}
	else if (tempPos.x > 1.5f)
	{
		//pA.speed *= -1.0f;

		pA.speed -= 2.0f * dot(normalize(pA.speed), (-1.0f * xNorm)) * (-1.0f * xNorm);// *.1f;
		//pA.pos.x = min(pA.pos.x, 15.0f);
	}

	if (tempPos.y < 0.0f)
	{
		//pA.speed *= -1.0f;

		pA.speed -= 2.0f * dot(normalize(pA.speed), yNorm) * yNorm;// *.1f;
		//pA.pos.y = max(pA.pos.y, 0.0f);
	}
	else if (tempPos.y > 2.0f)
	{
		//pA.speed *= -1.0f;

		pA.speed -= 2.0f * dot(normalize(pA.speed), (-1.0f * yNorm)) * (-1.0f * yNorm);// *.1f;
		//pA.pos.y = min(pA.pos.y, 20.0f);
	}

	if (tempPos.z < 0.0f)
	{
		//pA.speed *= -1.0f;

		pA.speed -= 2.0f * dot(normalize(pA.speed), zNorm) * zNorm;// *.1f;
		//pA.pos.z = max(pA.pos.z, 0.0f);
	}
	else if (tempPos.z > 1.0f)
	{
		//pA.speed *= -1.0f;

		pA.speed -= 2.0f * dot(normalize(pA.speed), (-1.0f * zNorm)) * (-1.0f * zNorm);// *.1f;
		//pA.pos.z = min(pA.pos.z, 10.0f);
	}
	//printf("ParticleCount: %i, i: %i\n", ParticlesCount, i);
	//printf("nextParticle: %i\n", nextParticle);
}

void calcPressure(Particle pA, Particle pB)
{
	pA.density = 0.0f;

	vec3 diff = pA.pos - pB.pos;
	float r2 = dot(diff, diff);
	if (r2 < pow(h, 2))
	{
		float W = Poly6_Const * pow(pow(h, 2) - r2, 3);
		pA.density += pB.weight * W;
	}
	pA.density = max(referenceDensity, pA.density);

	pA.pressure = pressureConstant * (pA.density - referenceDensity);
}

void calcForce(Particle pA, Particle pB)
{
	pA.force = vec3(0.0);
	
	vec3 diff = pA.pos - pB.pos;
	float r = sqrt(dot(diff, diff));
	if (r > 0 && r < h)
	{
		vec3 rNorm = diff / r;
		float W = Spiky_Const * pow(h - r, 2);

		pA.force += (pB.weight / pA.weight) * ((pA.pressure + pB.pressure) / (2.0f * pA.density * pB.density)) * W * rNorm;
	}
	pA.force *= -1.0f;
}

void calcViscosity(Particle pA, Particle pB, float deltaTime)
{
	vec3 tempForce{ 0.0 };

	vec3 diff = pA.pos - pB.pos;
	float r = sqrt(dot(diff, diff));
	if(r > 0 && r < h)
	{
		vec3 rNorm = diff / r;
		float r3 = pow(r, 3);
		float W = -(r3 / (2.0f * pow(h, 3)) + (pow(r, 2) / pow(h, 2)) + h / (2.0f * r)) - 1.0f;

		pA.force2 += (pB.weight / pA.weight) * (1.0f / pB.density) * (pB.speed - pA.speed) * W * rNorm;
	}

	pA.force2 *= viscosity;
}

void checkNeighbour(Particle pA, float deltaTime, int currElement)
{
	for(int i = 0; i < nextParticle; i++)
	{
		//printf("%s", "Hej");
		if(length(pA.pos - ParticlesContainer[i].pos) < EPSILON)
		{
			Particle& pB = ParticlesContainer[i];
			calcPressure(pA, pB);
			if(i != currElement)
			{
				calcForce(pA, pB);
				calcViscosity(pA, pB, deltaTime);
			}
			setBoundingBoxWithPosAndSpeed(pA, deltaTime);
		}
	}
}

int main(void)
{
	//initProgram
	if (!initProgram())
		return -1;
	



	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	// Hide the mouse and enable unlimited movement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set the mouse at the center of the screen
	glfwPollEvents();
	glfwSetCursorPos(window, WindowWidth / 2, WindowHeight / 2);

	// Dark blue background
	glClearColor(0.1f, 0.1f, 0.1f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = loadShaders("Particle.vert", "Particle.frag");

	// Vertex shader
	GLuint CameraRight_worldspace_ID = glGetUniformLocation(programID, "CameraRight_worldspace");
	GLuint CameraUp_worldspace_ID = glGetUniformLocation(programID, "CameraUp_worldspace");
	GLuint ViewProjMatrixID = glGetUniformLocation(programID, "VP");

	// fragment shader
	GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");


	static GLfloat* g_particule_position_size_data = new GLfloat[MaxParticles * 4];
	static GLubyte* g_particule_color_data = new GLubyte[MaxParticles * 4];

	for (int i = 0; i < MaxParticles; i++) {		
		ParticlesContainer[i].cameradistance = -1.0f;
	}


	GLuint Texture = loadDDS("particle.DDS");

	// The VBO containing the 4 vertices of the particles.
	// Thanks to instancing, they will be shared by all particles.
	static const GLfloat g_vertex_buffer_data[] = {
		 -0.5f, -0.5f, 0.0f,
		  0.5f, -0.5f, 0.0f,
		 -0.5f,  0.5f, 0.0f,
		  0.5f,  0.5f, 0.0f,
	};
	GLuint billboard_vertex_buffer;
	glGenBuffers(1, &billboard_vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	// The VBO containing the positions and sizes of the particles
	GLuint particles_position_buffer;
	glGenBuffers(1, &particles_position_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

	// The VBO containing the colors of the particles
	GLuint particles_color_buffer;
	glGenBuffers(1, &particles_color_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
	// Initialize with empty (NULL) buffer : it will be updated later, each frame.
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);



	double lastTime = glfwGetTime();
	do
	{
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		double currentTime = glfwGetTime();
		double delta = currentTime - lastTime;
		lastTime = currentTime;


		// Calc FPS and set title
		calcFPS();

		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();

		// We will need the camera's position in order to sort the particles
		// w.r.t the camera's distance.
		// There should be a getCameraPosition() function in common/controls.cpp, 
		// but this works too.
		glm::vec3 CameraPosition(glm::inverse(ViewMatrix)[3]);

		glm::mat4 ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

		generateNewParticles(&delta);


		// Simulate all particles		
		for (int ParticlesCount = 0; ParticlesCount < nextParticle; ParticlesCount++) {

			Particle& pA = ParticlesContainer[ParticlesCount]; // shortcut
			//printf("Pos: %f, %f, %f\n", pA.pos.x, pA.pos.y, pA.pos.z);

			// Simulate simple physics : gravity only, no collisions
			/*pA.speed += glm::vec3(0.0f, -9.82f, 0.0f) * (float)delta * 0.5f;
			pA.pos += pA.speed * (float)delta;

			pA.pos.x = min(pA.pos.x, 15.0f);
			pA.pos.y = min(pA.pos.y, 20.0f);
			pA.pos.z = min(pA.pos.z, 10.0f);
			
			pA.pos.x = max(pA.pos.x, 0.0f);
			pA.pos.y = max(pA.pos.y, 0.0f);
			pA.pos.z = max(pA.pos.z, 0.0f);*/

			
			for(int i = 0; i < nextParticle; i++)
			{
				if (length(ParticlesContainer[i].pos - pA.pos) > 5.0f)
				{
					continue;
				}
				Particle& pB = ParticlesContainer[i];
				
				pA.density = 0.0f;

				vec3 diff = pA.pos - pB.pos;
				float r2 = dot(diff, diff);
				if (r2 < pow(h, 2))
				{
					float W = Poly6_Const * pow(pow(h, 2) - r2, 3);
					pA.density +=  pB.weight * W;
				}
				pA.density = max(referenceDensity, pA.density);

				pA.pressure = pressureConstant * (pA.density - referenceDensity);
				
				if (i != ParticlesCount)
				{
					pA.force = vec3(0.0f);

					float r = sqrt(dot(diff, diff));
					if (r > 0 && r < h)
					{
						vec3 rNorm = diff / r;
						float W1 = Spiky_Const * pow(h - r, 2);

						float r3 = pow(r, 3);
						float W2 = -(r3 / (2.0f * pow(h, 3)) + (pow(r, 2) / pow(h, 2)) + h / (2.0f * r)) - 1.0f;
						
						pA.force += (pB.weight / pA.weight) * ((pA.pressure + pB.pressure) / (2.0f * pA.density * pB.density)) * W1 * rNorm;
						pA.force2 += (pB.weight / pA.weight) * (1.0f / pB.density) * (pB.speed - pA.speed) * W2 * rNorm;
					}


					// Collision?!??
					if(r < 2 * particleSize && dot(pA.speed, diff) < dot(pB.speed, diff))
					{
						float vNeRel = dot(diff, normalize(diff * -1.0f));
						float jForm = (-(EPSILON + 1.0f) * vNeRel) / (1.0f / pA.weight + 1.0f / pB.weight);
						vec3 Imp = normalize(diff) * jForm;
						//printf("Imp: %f, %f, %f -- vNe: %f -- jForm: %f\n", Imp.x, Imp.y, Imp.z, vNeRel, jForm);
						pA.force = pA.force + (Imp * (-1.0f / static_cast<float>(delta)));
						pB.force = pA.force + (Imp * ( 1.0f / static_cast<float>(delta)));
					}
					
					pA.force *= -1.0f;

					pA.force2 *= viscosity;

					pA.speed += static_cast<float>(delta) * ((pA.force + pA.force2) / pA.density + G) * 0.01f;
					//printf("Length: %f\n", length(pA.speed));
				}

				vec3 tempPos = pA.pos + static_cast<float>(delta) * pA.speed;

				//printf("Pos: %f, %f, %f\n", tempPos.x, tempPos.y, tempPos.z);

				vec3 xNorm{ 1.0f, 1.0f, 0.0f }, yNorm{ 0.0f, 1.0f, 0.0f }, zNorm{ 0.0f, 0.0f, 1.0f };
				//xNorm *= 0.1f;
				//yNorm *= 0.1f;
				//zNorm *= 0.1f;

				if (tempPos.x < 0.0f)
				{
					//pA.speed *= -1.0f;

					pA.speed -= 2.0f * dot(normalize(pA.speed), xNorm) * xNorm *.9f;
					//pA.pos.x = max(pA.pos.x, 0.0f);
				}
				else if (tempPos.x > 1.5f)
				{
					//pA.speed *= -1.0f;

					pA.speed -= 2.0f * dot(normalize(pA.speed), (-1.0f * xNorm)) * (-1.0f * xNorm) *.9f;
					//pA.pos.x = min(pA.pos.x, 15.0f);
				}

				if (tempPos.y < 0.0f)
				{
					//pA.speed *= -1.0f;

					pA.speed -= 2.0f * dot(normalize(pA.speed), yNorm) * yNorm *.9f;
					//pA.pos.y = max(pA.pos.y, 0.0f);
				}
				else if (tempPos.y > 2.0f)
				{
					//pA.speed *= -1.0f;

					pA.speed -= 2.0f * dot(normalize(pA.speed), (-1.0f * yNorm)) * (-1.0f * yNorm) *.9f;
					//pA.pos.y = min(pA.pos.y, 20.0f);
				}

				if (tempPos.z < 0.0f)
				{
					//pA.speed *= -1.0f;

					pA.speed -= 2.0f * dot(normalize(pA.speed), zNorm) * zNorm *.9f;
					//pA.pos.z = max(pA.pos.z, 0.0f);
				}
				else if (tempPos.z > 1.0f)
				{
					//pA.speed *= -1.0f;

					pA.speed -= 2.0f * dot(normalize(pA.speed), (-1.0f * zNorm)) * (-1.0f * zNorm) *.9f;
					//pA.pos.z = min(pA.pos.z, 10.0f);
				}
				//printf("ParticleCount: %i, i: %i\n", ParticlesCount, i);
				//printf("nextParticle: %i\n", nextParticle);

				
			}
			//printf("ParticleCount: %i\n", ParticlesCount);

			//checkNeighbour(pA, delta, ParticlesCount);
			
			
			pA.pos += static_cast<float>(delta) * pA.speed;

			pA.pos.x = min(pA.pos.x, 1.5f);
			pA.pos.y = min(pA.pos.y, 2.0f);
			pA.pos.z = min(pA.pos.z, 1.0f);

			pA.pos.x = max(pA.pos.x, 0.0f);
			pA.pos.y = max(pA.pos.y, 0.0f);
			pA.pos.z = max(pA.pos.z, 0.0f);

			//printf("Speed: %f, %f, %f\n", pA.speed.x, pA.speed.y, pA.speed.z);
			//printf("Pos: %f, %f, %f\n", pA.pos.x, pA.pos.y, pA.pos.z);

			pA.force = vec3(0.0f);

			//checkNeighbour(pA, delta, ParticlesCount);
			
			//printf("Pos: %f, %f, %f\n", pA.speed.x, pA.speed.y, pA.speed.z);
			
			pA.cameradistance = glm::length2(pA.pos - CameraPosition);			

			// Fill the GPU buffer
			g_particule_position_size_data[4 * ParticlesCount + 0] = pA.pos.x;
			g_particule_position_size_data[4 * ParticlesCount + 1] = pA.pos.y;
			g_particule_position_size_data[4 * ParticlesCount + 2] = pA.pos.z;

			g_particule_position_size_data[4 * ParticlesCount + 3] = pA.size;

			g_particule_color_data[4 * ParticlesCount + 0] = pA.r;
			g_particule_color_data[4 * ParticlesCount + 1] = pA.g;
			g_particule_color_data[4 * ParticlesCount + 2] = pA.b;
			g_particule_color_data[4 * ParticlesCount + 3] = pA.a;

			//ParticlesCount++;		
		}

		SortParticles();


		//printf("%d ",ParticlesCount);		


		// Update the buffers that OpenGL uses for rendering.
		// There are much more sophisticated means to stream data from the CPU to the GPU, 
		// but this is outside the scope of this tutorial.
		// http://www.opengl.org/wiki/Buffer_Object_Streaming


		glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
		glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
		glBufferSubData(GL_ARRAY_BUFFER, 0, nextParticle * sizeof(GLfloat) * 4, g_particule_position_size_data);

		glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
		glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
		glBufferSubData(GL_ARRAY_BUFFER, 0, nextParticle * sizeof(GLubyte) * 4, g_particule_color_data);


		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Use our shader
		glUseProgram(programID);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);

		// Same as the billboards tutorial
		glUniform3f(CameraRight_worldspace_ID, ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
		glUniform3f(CameraUp_worldspace_ID, ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);

		glUniformMatrix4fv(ViewProjMatrixID, 1, GL_FALSE, &ViewProjectionMatrix[0][0]);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
		glVertexAttribPointer(
			0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : positions of particles' centers
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
		glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			4,                                // size : x + y + z + size => 4
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// 3rd attribute buffer : particles' colors
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
		glVertexAttribPointer(
			2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			4,                                // size : r + g + b + a => 4
			GL_UNSIGNED_BYTE,                 // type
			GL_TRUE,                          // normalized?    *** YES, this means that the unsigned char[4] will be accessible with a vec4 (floats) in the shader ***
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// These functions are specific to glDrawArrays*Instanced*.
		// The first parameter is the attribute buffer we're talking about.
		// The second parameter is the "rate at which generic vertex attributes advance when rendering multiple instances"
		// http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribDivisor.xml
		glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
		glVertexAttribDivisor(1, 1); // positions : one per quad (its center)                 -> 1
		glVertexAttribDivisor(2, 1); // color : one per quad                                  -> 1

		// Draw the particules !
		// This draws many times a small triangle_strip (which looks like a quad).
		// This is equivalent to :
		// for(i in ParticlesCount) : glDrawArrays(GL_TRIANGLE_STRIP, 0, 4), 
		// but faster.
		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, nextParticle);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);


	delete[] g_particule_position_size_data;

	// Cleanup VBO and shader
	glDeleteBuffers(1, &particles_color_buffer);
	glDeleteBuffers(1, &particles_position_buffer);
	glDeleteBuffers(1, &billboard_vertex_buffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &Texture);
	glDeleteVertexArrays(1, &VertexArrayID);


	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}