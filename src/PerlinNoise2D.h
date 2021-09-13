#pragma once

class PerlinNoise2D {
	private:
		const double PI = 3.1415926535897932384626433832795028841971;
		const double halfPI = (double)PI / 2;
		//const bool corners[4][2] = {{0, 0}, {0, 1}, {1, 0}, {1, 1}}; //4 corners of grid - bl, tl, br, tr
		const bool corners[8][3] = {{0, 0, 0}, {0, 1, 0}, {1, 0, 0}, {1, 1, 0}, {0, 0, 1}, {0, 1, 1}, {1, 0, 1}, {1, 1, 1}}; //4 corners of grid - bl, tl, br, tr

		long long int seed;
		int width;
		double ratio; //multiply block coordinate by this (gridWidth / worldWidth) to get coordinate on perlin noise grid

		double smoothStep(double x);

	public:
		PerlinNoise2D(long long int seed, int gridWidth, double ratio);
		double perlinIteration(double x, double z);
		double getHeight(int x, int z, int iterations, double ratioFactor, double damping, double scale = 1);
		double perlinIteration3D(double x, double y, double z);
		double iterativePerlin3D(int x, int y, int z, int iterations, double ratioFactor, double damping, double scale = 1);
		//getSeed, setSeed
		~PerlinNoise2D() {};
};