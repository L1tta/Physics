#pragma once
#include "glm\glm.hpp"

struct FluidCell
{
	float* Preasure;
	glm::vec2* Velocity;
	glm::vec3* Dye_colour;
};

class DIYFluid
{
public:

	DIYFluid(int _width, int _height, float _viscocity, float _cell_dist);
	~DIYFluid();
	void UpdateFluid(float dt);
	void RenderFluid(glm::mat4 view_proj);
	void Advect(float dt);
	void Diffuse(float dt);
	void Divergence(float dt);
	void UpdatePreassure(float dt);
	void ApplyPressure(float dt);
	void UpdateBoundary();
	void Swapcolors();
	void SwapVelocities();
	void SwapPreasures();
	float Viscocity;
	float Cell_dist;
	unsigned int program;

	FluidCell front_cells;
	FluidCell back_cells;

	float * divergence;

	int Width;
	int Height;
};