#version 330 core

const float PI_F = 3.1415;
const float EPSILON = 0.000001;

uniform sampler2D texUnit1;
uniform int texSize;

in vec2 outTexCoord;
out vec4 out_Color;


// Temporary variables
struct Particle {
	float density;
	vec3 pos;
	vec3 vel;
	vec3 force;
};

int numParticles = 1000;

float particleMass = 10.0f; 
float particleRadius = 1.0f; 
float h = 5.0f;
float referenceDensity = 1.0f; 
float pressureConstant = 250.0f; 
float viscosity = 0.018f;

// Constants
float Poly6_const = 315.0f / (64.0f * PI_F * pow(h, 9));
float Spiky_const = -45.0f / (PI_F * pow(h, 6));
float deltaTime = 0.016f; // For 60fps
vec3 G = vec3(0, -9.8f, 0);

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

	pA.vel += deltaTime * ((pA.force + tempForce) / pA.density + G);
	pA.pos += deltaTime * pA.vel;
	pA.force = vec3(0.0);
}

void neighbouringParticles(inout Particle pA) {

	ivec2 zSquare = ivec2(floor(outTexCoord.x / 500) , floor(outTexCoord.y / 500));

	float xLO = (outTexCoord.x - ((7.0 / texSize) * zSquare.x * 500.0)) > 0.0 ? outTexCoord.x - (7.0 / texSize) : (500.0 / texSize) * zSquare.x;
	float xHI = (outTexCoord.x + ((7.0 / texSize) * zSquare.x * 500.0)) < 500.0 ? outTexCoord.x + (7.0 / texSize) : (500.0 / texSize) * (zSquare.x + 1);
	float yLO = (outTexCoord.y - ((7.0 / texSize) * zSquare.y * 500.0)) > 0.0 ? outTexCoord.y - (7.0 / texSize) : (500.0 / texSize) * zSquare.y;
	float yHI = (outTexCoord.y + ((7.0 / texSize) * zSquare.y * 500.0)) < 500.0 ? outTexCoord.y + (7.0 / texSize) : (500.0 / texSize) * (zSquare.y + 1);
	int zLO = (zSquare.x + zSquare.y - 7) > 0 ?  zSquare.x + zSquare.y - 7 : 0;
	int zHI = (zSquare.x + zSquare.y + 7) < 63 ? zSquare.x + zSquare.y + 7 : 63;

	for(float x = xLO; x < xHI; x + (1.0/texSize)) {
		for(float y = yLO; y < yHI; y + (1.0/texSize)){
			for(int z = zLO; z < zHI; z++) {
				vec4 currPos = texture(texUnit1, vec2(x, y));
				if(currPos.a > 0.8) {
					vec3 tempPos;
					TexToRel(vec2(x, y), tempPos);
					Particle pB = Particle(1.0, tempPos, vec3(currPos), vec3(0.0));
					calcForce(pA, pB);
					if(length(outTexCoord - vec2(x, y)) > EPSILON) {
						calcPressure(pA, pB);
						calcViscosity(pA, pB);
					}
				}
			}
		}
	}
}

void main(void){

	vec3 relPos;
	TexToRel(outTexCoord, relPos);

	Particle pA = Particle(1.0, relPos, vec3(texture(texUnit1, outTexCoord)), vec3(0.0));

	neighbouringParticles(pA);

	out_Color = texture(texUnit1, outTexCoord);
}