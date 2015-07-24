#include "glm\glm.hpp"
#include "DIYFluid.h"
#include "gl_core_4_4.h"
#include "Utilities.h"
void DIYFluid::Swapcolors()
{
	glm::vec3* temp = this->front_cells.Dye_colour;
	this->front_cells.Dye_colour = this->back_cells.Dye_colour;
	this->back_cells.Dye_colour = temp;
}
void DIYFluid::SwapVelocities()
{
	glm::vec2* temp = this->front_cells.Velocity;
	this->front_cells.Velocity = this->back_cells.Velocity;
	this->back_cells.Velocity = temp;
}
void DIYFluid::SwapPreasures()
{
	float* temp = this->front_cells.Preasure;
	this->front_cells.Preasure = this->back_cells.Preasure;
	this->back_cells.Preasure = temp;
}

DIYFluid::DIYFluid(int _width, int _height, float _viscocity, float _cell_dist)
{
	this->Width = _width;
	this->Height = _height;
	this->Viscocity = _viscocity;
	this->Cell_dist = _cell_dist;
	int Cell_count = _width * _height;
	this->front_cells.Velocity = new glm::vec2[Cell_count];
	this->front_cells.Dye_colour = new glm::vec3[Cell_count];
	this->front_cells.Preasure = new float[Cell_count];
	this->back_cells.Velocity = new glm::vec2[Cell_count];
	this->back_cells.Dye_colour = new glm::vec3[Cell_count];
	this->back_cells.Preasure = new float[Cell_count];
	this->divergence = new float[Cell_count];
	LoadShader("./simple_vertex.vs", 0, "./simple_texture.fs", &this->program);

	for (int i = 0; i < Cell_count; i++)
	{
		if (i < (Cell_count / 2))
		{
			this->front_cells.Dye_colour[i] = glm::vec3(255, 0, 0);
		}
		else
		{
			this->front_cells.Dye_colour[i] = glm::vec3(0, 255, 0);
		}
	}
}

DIYFluid::~DIYFluid()
{
	delete[] this->divergence;
	delete[] this->front_cells.Velocity;
	delete[] this->front_cells.Dye_colour;
	delete[] this->front_cells.Preasure;
	delete[] this->back_cells.Velocity;
	delete[] this->back_cells.Dye_colour;
	delete[] this->back_cells.Preasure;
}
void DIYFluid::UpdateFluid(float dt)
{
	Advect(dt);
	SwapVelocities();
	Swapcolors();

	for (int diffuse_step = 0; diffuse_step < 50; diffuse_step++)
	{
		Diffuse(dt);
		SwapVelocities();
	}


	Divergence(dt);

	for (int Preasure_step = 0; Preasure_step < 50; Preasure_step++)
	{
		UpdatePreassure(dt);
		SwapPreasures();
	}

	ApplyPressure(dt);


	UpdateBoundary();
}

void DIYFluid::Advect(float dt)
{
	for (int y = 0; y < this->Width; y++)
	{
		for (int x = 0; x < this->Height; x++)
		{
			int cell_index = x + y * this->Width;
			glm::vec2 velocity = this->front_cells.Velocity[cell_index] * dt;
			glm::vec2 sample_point = glm::vec2((float)x - velocity.x / this->Cell_dist, (float)y - velocity.y / this->Cell_dist);

			sample_point.x = glm::clamp(sample_point.x, 0.0f, (float)this->Width - 1.01f);
			sample_point.y = glm::clamp(sample_point.y, 0.0f, (float)this->Height - 1.01f);

			glm::vec2 bl = glm::vec2(floorf(sample_point.x), floorf(sample_point.y));
			glm::vec2 br = bl + glm::vec2(1, 0);
			glm::vec2 tl = bl + glm::vec2(0, 1);
			glm::vec2 tr = bl + glm::vec2(1, 1);

			int bli = (int)bl.x + this->Width * (int)bl.y;
			int bri = (int)br.x + this->Width * (int)br.y;
			int tli = (int)tl.x + this->Width * (int)tl.y;
			int tri = (int)tr.x + this->Width * (int)tr.y;

			glm::vec2 sample_fract = sample_point - bl;

			glm::vec3 dye_b = glm::mix(this->front_cells.Dye_colour[bli], this->front_cells.Dye_colour[bri], sample_fract.x);
			glm::vec3 dye_t = glm::mix(this->front_cells.Dye_colour[tli], this->front_cells.Dye_colour[tri], sample_fract.x);
			glm::vec3 new_dye = glm::mix(dye_b, dye_t, sample_fract.y);
			this->back_cells.Dye_colour[cell_index] = new_dye;

			glm::vec2 vel_b = glm::mix(this->front_cells.Velocity[bli], this->front_cells.Velocity[bri], sample_fract.x);
			glm::vec2 vel_t = glm::mix(this->front_cells.Velocity[tli], this->front_cells.Velocity[tri], sample_fract.x);
			glm::vec2 new_vel = glm::mix(vel_b, vel_t, sample_fract.y);
			this->back_cells.Velocity[cell_index] = new_vel;
		}
	}
}
void DIYFluid::Diffuse(float dt)
{
	float inv_vdt = 1.0f / (this->Viscocity * dt);
	for (int y = 0; y < this->Height; y++)
	{
		for (int x = 0; x < this->Width; x++)
		{
			int cell_index = x + y * this->Width;

			int xpl = glm::clamp(x + 1, 0, this->Width);
			int xml = glm::clamp(x - 1, 0, this->Width);
			int ypl = glm::clamp(y + 1, 0, this->Width);
			int yml = glm::clamp(y - 1, 0, this->Width);

			int up = x + ypl * this->Width;
			int down = x + yml * this->Width;
			int right = y + xpl * this->Width;
			int left = y + xml * this->Width;

			glm::vec2 vel_up = this->front_cells.Velocity[up];
			glm::vec2 vel_down = this->front_cells.Velocity[down];
			glm::vec2 vel_right = this->front_cells.Velocity[right];
			glm::vec2 vel_left = this->front_cells.Velocity[left];
			glm::vec2 vel_centre = this->front_cells.Velocity[cell_index];

			float denom = 1.0f / (4 + inv_vdt);
			glm::vec2 diffused_velocity = (vel_up + vel_down + vel_left + vel_right + vel_centre * inv_vdt) * denom;

			this->back_cells.Velocity[cell_index] = diffused_velocity;
		}
	}
}
void DIYFluid::Divergence(float dt)
{
	float inv_cell_dist = 1.0f / (2.0f * this->Cell_dist);

	for (int y = 0; y < this->Width; y++)
	{
		for (int x = 0; x < this->Height; x++)
		{
			int cell_index = x + y * this->Width;

			int xpl = glm::clamp(x + 1, 0, this->Width);
			int xml = glm::clamp(x - 1, 0, this->Width);
			int ypl = glm::clamp(y + 1, 0, this->Width);
			int yml = glm::clamp(y - 1, 0, this->Width);

			int up = x + ypl * this->Width;
			int down = x + yml * this->Width;
			int right = y + xpl * this->Width;
			int left = y + xml * this->Width;

			float vel_up = this->front_cells.Velocity[up].y;
			float vel_down = this->front_cells.Velocity[down].y;
			float vel_left = this->front_cells.Velocity[left].x;
			float vel_right = this->front_cells.Velocity[right].x;

			float diver = ((vel_right - vel_left) + (vel_up - vel_down)) * inv_cell_dist;

			this->divergence[cell_index] = diver;
		}
	}
}
void DIYFluid::UpdatePreassure(float dt)
{
	for (int y = 0; y < this->Width; y++)
	{
		for (int x = 0; x < this->Height; x++)
		{
			int cell_index = x + y * this->Width;

			int xpl = glm::clamp(x + 1, 0, this->Width);
			int xml = glm::clamp(x - 1, 0, this->Width);
			int ypl = glm::clamp(y + 1, 0, this->Width);
			int yml = glm::clamp(y - 1, 0, this->Width);

			int up = x + ypl * this->Width;
			int down = x + yml * this->Width;
			int right = y + xpl * this->Width;
			int left = y + xml * this->Width;

			float p_up = this->front_cells.Preasure[up];
			float p_down = this->front_cells.Preasure[down];
			float p_left = this->front_cells.Preasure[left];
			float p_right = this->front_cells.Preasure[right];

			float d = this->divergence[cell_index];

			float new_pressure = (p_up + p_down + p_left + p_right - d * this->Cell_dist * this->Cell_dist) * 0.25f;
		}
	}
}
void DIYFluid::ApplyPressure(float dt)
{
	float inv_cell_dist = 1.0f / (2.0f * this->Cell_dist);

	for (int y = 0; y < this->Width; y++)
	{
		for (int x = 0; x < this->Height; x++)
		{
			int cell_index = x + y * this->Width;

			int xpl = glm::clamp(x + 1, 0, this->Width);
			int xml = glm::clamp(x - 1, 0, this->Width);
			int ypl = glm::clamp(y + 1, 0, this->Width);
			int yml = glm::clamp(y - 1, 0, this->Width);

			int up = x + ypl * this->Width;
			int down = x + yml * this->Width;
			int right = y + xpl * this->Width;
			int left = y + xml * this->Width;

			float p_up = this->front_cells.Preasure[up];
			float p_down = this->front_cells.Preasure[down];
			float p_left = this->front_cells.Preasure[left];
			float p_right = this->front_cells.Preasure[right];

			glm::vec2 delta_v = -glm::vec2(p_right - p_left, p_up - p_down) * inv_cell_dist;

			this->back_cells.Velocity[cell_index] = this->front_cells.Velocity[cell_index] + delta_v;
		}
	}
}
void DIYFluid::UpdateBoundary()
{
	for (int x = 0; x < this->Width; x++)
	{
		int first_row_index = x;
		int second_row_index = x + this->Width;

		this->front_cells.Preasure[first_row_index] = this->front_cells.Preasure[second_row_index];
		this->front_cells.Velocity[first_row_index].x = this->front_cells.Velocity[second_row_index].x;
		this->front_cells.Velocity[first_row_index].y = -this->front_cells.Velocity[second_row_index].y;

		int bottom_index = x + (this->Height - 1) * this->Width;
		int second_last_row_index = x + (this->Height - 2) * this->Width;

		this->front_cells.Preasure[bottom_index] = this->front_cells.Preasure[second_last_row_index];
		this->front_cells.Velocity[bottom_index].x = this->front_cells.Velocity[second_last_row_index].x;
		this->front_cells.Velocity[bottom_index].y = -this->front_cells.Velocity[second_last_row_index].y;
	}

	for (int y = 0; y < this->Width; y++)
	{
		int first_col_index = 0 + y * this->Width;
		int second_col_index = 1 + y * this->Width;

		this->front_cells.Preasure[first_col_index] = this->front_cells.Preasure[second_col_index];
		this->front_cells.Velocity[first_col_index].x = -this->front_cells.Velocity[second_col_index].x;
		this->front_cells.Velocity[first_col_index].y = this->front_cells.Velocity[second_col_index].y;

		int last_col_index = (this->Width - 1) + y * this->Width;
		int second_last_col_index = (this->Width - 2) + y * this->Width;

		this->front_cells.Preasure[last_col_index] = this->front_cells.Preasure[second_last_col_index];
		this->front_cells.Velocity[last_col_index].x = -this->front_cells.Velocity[second_last_col_index].x;
		this->front_cells.Velocity[last_col_index].y = this->front_cells.Velocity[second_last_col_index].y;
	}
}

void DIYFluid::RenderFluid(glm::mat4 view_proj)
{
	unsigned char *tex_data = new unsigned char[this->Width * this->Height * 3];

	for (int i = 0; i < this->Width * this->Height; i++)
	{
		tex_data[i * 3 + 0] = this->front_cells.Dye_colour[i].r;
		tex_data[i * 3 + 1] = this->front_cells.Dye_colour[i].g;
		tex_data[i * 3 + 2] = this->front_cells.Dye_colour[i].b;
	}

	unsigned int texture_handle = CreateGLTextureBasic(tex_data, this->Width, this->Height, 3);
	unsigned int quad_vao = BuildQuadGLVAO(5.0f);

	RenderQuad(quad_vao, this->program, texture_handle, view_proj);

	glDeleteVertexArrays(1, &quad_vao);
	glDeleteTextures(1, &texture_handle);
	
}