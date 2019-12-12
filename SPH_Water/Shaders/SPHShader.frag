#version 420 core

uniform sampler2D texPos;
uniform sampler2D texVel;
uniform float texSize;
uniform float deltaTime;

in vec2 outTexCoord;
out vec4 out_Color;


const float PI_F = 3.1415;
const float EPSILON = 0.000001;

// Temporary variables
struct Particle {
	float density;
	vec3 pos;
	vec3 vel;
	vec3 force;
};

int numParticles = 1000;

float particleMass = 10.0; 
float particleRadius = 1.0; 
float h = 5.0;
float referenceDensity = 1.0; 
float pressureConstant = 0.1; 
float viscosity = 0.018;

// Constants
float Poly6_const = 315.0 / (64.0 * PI_F * pow(h, 9));
float Spiky_const = -45.0 / (PI_F * pow(h, 6));
//float deltaTime = 0.016; // For 60fps
vec3 G = vec3(0, -9.8, 0.2);

/*void RelToTex(in vec3 relPos, out vec2 texPos)
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
}*/

void calcPressure(inout Particle pA, in Particle pB) {

	pA.density = 0.0;
	
	vec3 diff = pA.pos - pB.pos;
	float r2 = dot(diff, diff);
	if(r2 < pow(h, 2)) {
		float W = Poly6_const * pow(pow(h, 2) - r2, 3);
		pA.density += particleMass * W;
	}

	pA.density = max(referenceDensity, pA.density);
}

void calcForce(inout Particle pA, in Particle pB) {

	pA.force = vec3(0.0);

	vec3 diff = pA.pos - pB.pos;
	float r = sqrt(dot(diff, diff));
	if(r > 0 && r < h) {
		vec3 rNorm = diff / r;
		float W = Spiky_const * pow(h - r, 2);

		pA.force += ((pressureConstant * (pA.density - referenceDensity) + pressureConstant * (pB.density - referenceDensity)) / (2.0 * pA.density * pB.density)) * W * rNorm;
	}
	
	pA.force *= -1.0;
}

void calcViscosity(inout Particle pA, in Particle pB) {

	vec3 tempForce = vec3(0.0);

	vec3 diff = pA.pos - pB.pos;
	float r = sqrt(dot(diff, diff));
	if(r > 0 && r < h) {
		vec3 rNorm = diff / r;
		float r3 = pow(r, 3);
		float W = -(r3 / (2 * pow(h, 3)) + (pow(r, 2) / pow(h, 2)) + h / (2.0 * r)) - 1.0;

		tempForce += (1.0 / pB.density) * (pB.vel - pA.vel) * W * rNorm;
	}

	tempForce *= viscosity;

	pA.vel += 0.01 * deltaTime * ((pA.force + tempForce) / pA.density + G);
	pA.pos += deltaTime * pA.vel;
	pA.force = vec3(0.0);
}

void neighbouringParticles(inout Particle pA) {

	ivec2 zSquare = ivec2(floor(outTexCoord.x / 500), floor(outTexCoord.y / 500));

	float xLO, xHI, yLO, yHI;

	if((outTexCoord.x - ((7.0 / texSize) * zSquare.x * 500.0)) > 0.0)
	{
		xLO = outTexCoord.x - (7.0 / texSize);
	}
	else
	{
		xLO = (500.0 / texSize) * zSquare.x;
	}

	if((outTexCoord.x + ((7.0 / texSize) * zSquare.x * 500.0)) < 500.0)
	{
		xHI = outTexCoord.x + (7.0 / texSize);
	}
	else
	{
		xHI = (500.0 / texSize) * (zSquare.x + 1.0);
	}

	if((outTexCoord.y - ((7.0 / texSize) * zSquare.y * 500.0)) > 0.0)
	{
		yLO = outTexCoord.y - (7.0 / texSize);
	}
	else
	{
		yLO = (500.0 / texSize) * zSquare.y;
	}

	if((outTexCoord.y + ((7.0 / texSize) * zSquare.y * 500.0)) < 500.0)
	{
		yHI = outTexCoord.y + (7.0 / texSize);
	}
	else
	{
		yHI = (500.0 / texSize) * (zSquare.y + 1.0);
	}

	//float xLO = (outTexCoord.x - ((7.0 / texSize) * zSquare.x * 500.0)) > 0.0 ? outTexCoord.x - (7.0 / texSize) : (500.0 / texSize) * zSquare.x;
	//float xHI = (outTexCoord.x + ((7.0 / texSize) * zSquare.x * 500.0)) < 500.0 ? outTexCoord.x + (7.0 / texSize) : (500.0 / texSize) * (zSquare.x + 1.0);
	//float yLO = (outTexCoord.y - ((7.0 / texSize) * zSquare.y * 500.0)) > 0.0 ? outTexCoord.y - (7.0 / texSize) : (500.0 / texSize) * zSquare.y;
	//float yHI = (outTexCoord.y + ((7.0 / texSize) * zSquare.y * 500.0)) < 500.0 ? outTexCoord.y + (7.0 / texSize) : (500.0 / texSize) * (zSquare.y + 1.0);
	int zLO = (zSquare.x + zSquare.y - 7) > 0 ?  zSquare.x + zSquare.y - 7 : 0;
	int zHI = (zSquare.x + zSquare.y + 7) < 63 ? zSquare.x + zSquare.y + 7 : 63;

	for(float x = xLO; x < xHI; x += (1.0/texSize)) {
		for(float y = yLO; y < yHI; y += (1.0/texSize)){
			for(int z = zLO; z < zHI; z++) {

				vec2 currTex = vec2((z-floor(z / 8.0)) * 500.0 + x , floor(z / 8.0) * 500.0 + y);

				vec4 currVel = texture(texVel, currTex);
				if(currVel.a > 0.8) {
					Particle pB = Particle(1.0, vec3(texture(texPos, currTex)), vec3(currVel), vec3(0.0));
					calcForce(pA, pB);
					if(length(outTexCoord - currTex) > EPSILON) {
						calcPressure(pA, pB);
						calcViscosity(pA, pB);
					}
				}
			}
		}
	}
}

void main(void)
{

	Particle pA = Particle(1.0, vec3(texture(texPos, outTexCoord)), vec3(texture(texVel, outTexCoord)), vec3(0.0));

	neighbouringParticles(pA);

	out_Color = vec4(pA.vel, 1.0);
}
