#include "PerlinNoise2D.h"
#include <stdlib.h>
#include <cmath>
#include <iostream>
//#include <vector>

PerlinNoise2D::PerlinNoise2D(long long int seed, int gridWidth, double ratio) {
	this->seed = seed % INT_MAX;
	this->width = gridWidth;
	this->ratio = ratio;
}

double PerlinNoise2D::smoothStep(double x) { //sigmoid like function to produce a smooth interpolation between values
	return (3 * x * x - 2 * x * x * x);
}

double PerlinNoise2D::perlinIteration(double x, double z) {
	//double perlinX = (double)(x + 0.5) * ratio; //coordinates of center of block column relative to the perlin noise grid
	//double perlinZ = (double)(z + 0.5) * ratio;
	double dotProducts[4];
	int currX;
	int currZ;
	//double offX;
	//double offZ;
	//double gradX;
	//double gradZ;
	double angle;
	for (int i = 0; i < 4; i++) { //iterate through all 4 corners
		currX = (int)(x + corners[i][0]); //locate corner
		currZ = (int)(z + corners[i][1]);
#
		//find gradient vector at corner
		srand((seed + (long long int)((long long int)1000000000 * currX + (long long int)1000000000 * currZ * ((long long int)width + 1))) % INT_MAX);
		//rand();
		//std::cout << rand() << std::endl;
		angle = ((double)rand() / RAND_MAX) * 2 * PI;
		//std::cout << angle << std::endl;
		//angle = PI;
		//gradX = cos(angle);
		//gradZ = sin(angle);

		//find offset vector at corner
		//offX = currX - perlinX;
		//offZ = currZ - perlinZ;

		//dot product
		//dotProducts[i] = offX * gradX + offZ * gradZ;
		dotProducts[i] = cos(angle) * (currX - x) + sin(angle) * (currZ - z);
	}
	//return dotProducts[0];
	//interpolate between 4 dot products
	x -= (int)x; //decimal part only needed for interpolation
	z -= (int)z;
	angle = smoothStep(z); //reuse angle variable for smoothStep
	dotProducts[0] += angle * (dotProducts[1] - dotProducts[0]);
	dotProducts[2] += angle * (dotProducts[3] - dotProducts[2]);
	return (double)(dotProducts[0] + smoothStep(x) * (dotProducts[2] - dotProducts[0]));
}

/*void generateDotProducts(std::vector<int> &original, std::vector<int> coords, int index) { //original unchanged coordinates
	if (index == coords.size()) {
		long long int currSeed = seed;
		long long int mag = 1000;
		for (int i = 0; i < coords.size(); i++) {
			currSeed += coords[i] * mag;
			mag *= 1000;
		}
		srand(currSeed);
		double angle = ((double)rand() / RAND_MAX) * 2 * PI;
		dotProducts[ptr] = cos(angle) * (;
		ptr += 1;
	}
	else {
		generateDotProducts(original, coords, index + 1);
		coords[index] += 1;
		generateDotProducts(original, coords, index + 1);
	}
}

double newNoise(std::vector<double> coords) { //n dimensional
	//number of points = 2^coords.size()
	int rightPtr = pow(2, coords.size()) - 1;
	std::vector<int> dotProducts(16, 0); //replace this with attribute
	int ptr = 0; //make this an attribute
}*/

double PerlinNoise2D::getHeight(int x, int z, int iterations, double ratioFactor, double damping, double scale) {
	double factor = 1;
	double result = 0;
	double currRatio = this->ratio * scale;

	for (int i = 0; i < iterations; i++) {
		result += this->perlinIteration((x + 0.5) * currRatio, (z + 0.5) * currRatio) * factor;
		factor *= damping;
		currRatio *= ratioFactor;
	}
	return result;
}




double PerlinNoise2D::perlinIteration3D(double x, double y, double z) {
	double dotProducts[8];
	long long int currX, currY, currZ;
	//long long int currSeed;
	double angle, newZ;
	for (int i = 0; i < 8; i++) {
		currX = (int)x + corners[i][0];
		currY = (int)y + corners[i][1];
		currZ = (int)z + corners[i][2];
		srand((long long int)(seed + 1000 * (currX + currY * (width + 1) * (width + 1) + currZ * (width + 1))) % INT_MAX); //width of world - uniquely identifies each block
		//angle1 = ((double)rand() / RAND_MAX) * 2 * PI;
		//angle2 = ((double)rand() / RAND_MAX) * 2 * PI;
		newZ = (((double)rand() / RAND_MAX) * 2) - 1; //generates z coord between 1 and -1
		angle = ((double)rand() / RAND_MAX) * 2 * PI;
		dotProducts[i] = sqrt(1 - newZ * newZ) * (cos(angle) * (currX - x) + sin(angle) * (currZ - z)); //random z position - multiply by sqrt(1 - z^2) for consistency
	}
	x -= (int)x;
	y -= (int)y;
	z -= (int)z;
	for (int i = 0; i < 8; i += 2) {
		dotProducts[i] += smoothStep(y) * (dotProducts[i + 1] - dotProducts[i]);
	}
	for (int i = 0; i < 8; i += 4) {
		dotProducts[i] += smoothStep(x) * (dotProducts[i + 2] - dotProducts[i]);
	}
	return dotProducts[0] + smoothStep(z) * (dotProducts[4] - dotProducts[0]);
}

double PerlinNoise2D::iterativePerlin3D(int x, int y, int z, int iterations, double ratioFactor, double damping, double scale) {
	double factor = 1;
	double result = 0;
	double currRatio = this->ratio * scale;

	for (int i = 0; i < iterations; i++) {
		result += this->perlinIteration3D((x + 0.5) * currRatio, (y + 0.5) * currRatio, (z + 0.5) * currRatio) * factor;
		factor *= damping;
		currRatio *= ratioFactor;
	}
	return result;
}