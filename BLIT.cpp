#include <iostream>
#include <stdexcept>
#include <ctime>
#include "RasterSurface.h"
#include "XTime.h"
#include "Tiles.h"
#include "Fire_Animation.h"
#define RASTER_WIDTH 500
#define RASTER_HEIGHT 500
#define NUM_PIXELS (RASTER_WIDTH*RASTER_HEIGHT)
#define GRASS_LEFT   285
#define GRASS_TOP    125
#define GRASS_RIGHT  322
#define GRASS_BOTTOM 162
#define grass_width  (GRASS_RIGHT  - GRASS_LEFT)
#define grass_height (GRASS_BOTTOM - GRASS_TOP)
#define masc1 0xFF000000
#define masc2 0x00FF0000
#define masc3 0x0000FF00
#define masc4 0x000000FF
#define Tree_LEFT   319
#define Tree_TOP    17
#define Tree_RIGHT  384
#define Tree_BOTTOM 95
#define Tree_width  (Tree_RIGHT  - Tree_LEFT)
#define Tree_height (Tree_BOTTOM - Tree_TOP)
#define CellWidth (Fire_Animation_width/8)
#define CellHeight (Fire_Animation_height/8)
#define Sign_LEFT   175
#define Sign_TOP    177
#define Sign_RIGHT  225
#define Sign_BOTTOM 192
#define Sign_width  (Sign_RIGHT  - Sign_LEFT)*2
#define Sign_height (Sign_BOTTOM - Sign_TOP)*2
#define House_LEFT   127
#define House_TOP    31
#define House_RIGHT  191
#define House_BOTTOM 94
#define House_width  (House_RIGHT  - House_LEFT)
#define House_height (House_BOTTOM - House_TOP)

unsigned int SCREEN_ARRAY[NUM_PIXELS];
typedef unsigned int Color;

struct Rectangle_KC
{
	unsigned int left, top, right, bottom;
};

void DrawPixel(int x, int y, Color pixel_color);
void ColorClean(Color pixel_color);
void BLIT(const unsigned int* Source_Array, unsigned int* Screen_Array, unsigned int Source_Width, unsigned int Source_Height, unsigned int Screen_Width, unsigned int Screen_Height, Rectangle_KC Source_Rect, unsigned int Destination_X, unsigned int Destination_Y);
void Background_Tiles();
unsigned BGRAtoARGB(unsigned C);
unsigned AlphaBlending(unsigned int DestinationColor, unsigned int SourceColor);

int main()
{
	RS_Initialize("Katherine Chen, Lab 1", RASTER_WIDTH, RASTER_HEIGHT);

	srand((unsigned int)time(NULL));

	int total_trees = 10;
	unsigned int tree_positions[10][2];
	int placed = 0;
	unsigned int CurrentX_Fire = 0;
	unsigned int CurrentY_Fire = 0;
	Rectangle_KC src_animation = {};
	XTime time;
	double accumulator = 0;

	while (placed < total_trees)
	{
		unsigned int new_x = rand() % (RASTER_WIDTH - Tree_width);
		unsigned int new_y = rand() % (RASTER_HEIGHT - Tree_height);

		bool collision = false;
		for (int i = 0; i < placed; i++)
		{
			int dx = (int)new_x - (int)tree_positions[i][0];
			int dy = (int)new_y - (int)tree_positions[i][1];
			if (dx < 0) dx = -dx;
			if (dy < 0) dy = -dy;
			if (dx < (int)Tree_width && dy < (int)Tree_height)
			{
				collision = true;
				break;
			}
		}

		int dx_fire = (int)new_x - (int)(RASTER_WIDTH / 2);
		int dy_fire = (int)new_y - (int)(RASTER_HEIGHT / 2);
		if (dx_fire < 0) dx_fire = -dx_fire;
		if (dy_fire < 0) dy_fire = -dy_fire;
		if (dx_fire < (int)CellWidth && dy_fire < (int)CellHeight)
		{
			collision = true;
		}

		int dx_sign = (int)new_x - (int)(20);
		int dy_sign = (int)new_y - (int)(450);
		if (dx_sign < 0) dx_sign = -dx_sign;
		if (dy_sign < 0) dy_sign = -dy_sign;
		if (dx_sign < (int)Sign_width && dy_sign < (int)Sign_height)
		{
			collision = true;
		}

		int dx_House = (int)new_x - (int)(420);
		int dy_House = (int)new_y - (int)(10);
		if (dx_House < 0) dx_House = -dx_House;
		if (dy_House < 0) dy_House = -dy_House;
		if (dx_House < (int)House_width && dy_House < (int)House_height)
		{
			collision = true;
		}

		if (!collision)
		{
			tree_positions[placed][0] = new_x;
			tree_positions[placed][1] = new_y;
			placed++;
		}
	}



	do {

		time.Signal();

		ColorClean(0xFF000000);

		//Grass
		Background_Tiles();

		//Tree
		for (int tree = 0; tree < total_trees; tree++)
		{
			Rectangle_KC src_rect = { Tree_LEFT, Tree_TOP, Tree_RIGHT, Tree_BOTTOM };
			BLIT(Tiles_pixels, SCREEN_ARRAY, Tiles_width, Tiles_height,
				RASTER_WIDTH, RASTER_HEIGHT, src_rect,
				tree_positions[tree][0], tree_positions[tree][1]);
		}

		//Fire
		accumulator += time.SmoothDelta();

		if (accumulator >= (1.0 / 30.0)) {

			accumulator -= (1.0 / 30.0);

			CurrentX_Fire += CellWidth;
			if (CurrentX_Fire >= Fire_Animation_width)
			{

				CurrentX_Fire = 0;
				CurrentY_Fire += CellHeight;

				if (CurrentY_Fire >= Fire_Animation_height)
				{
					CurrentX_Fire = 0;
					CurrentY_Fire = 0;
				}
			}

			src_animation = { CurrentX_Fire,  CurrentY_Fire,  CurrentX_Fire + CellWidth,  CurrentY_Fire + CellHeight };

		}


		BLIT(Fire_Animation_pixels, SCREEN_ARRAY, Fire_Animation_width, Fire_Animation_height,
			RASTER_WIDTH, RASTER_HEIGHT, src_animation,
			RASTER_WIDTH / 2, RASTER_HEIGHT / 2);

		//Signs
		Rectangle_KC src_signs = { Sign_LEFT, Sign_TOP, Sign_RIGHT, Sign_BOTTOM };
		BLIT(Tiles_pixels, SCREEN_ARRAY, Tiles_width, Tiles_height,
			RASTER_WIDTH, RASTER_HEIGHT, src_signs,
			20, 450);

		//House
		Rectangle_KC src_house = { House_LEFT, House_TOP, House_RIGHT, House_BOTTOM };
		BLIT(Tiles_pixels, SCREEN_ARRAY, Tiles_width, Tiles_height,
			RASTER_WIDTH, RASTER_HEIGHT, src_house,
			420, 10);


	} while (RS_Update(SCREEN_ARRAY, NUM_PIXELS));


	RS_Shutdown();
}

void ColorClean(Color pixel_color)
{
	for (int pixel_position = 0; pixel_position < NUM_PIXELS; pixel_position++)
	{
		SCREEN_ARRAY[pixel_position] = pixel_color;
	}
}

int D2_to_D1(int x, int y, int D2_width)
{
	return ((y * D2_width) + x);
}

void DrawPixel(int x, int y, Color pixel_color)
{
	int dimension = D2_to_D1(y, x, RASTER_WIDTH);

	if (x >= 0 && x < RASTER_WIDTH && y >= 0 && y < RASTER_HEIGHT)
	{
		SCREEN_ARRAY[dimension] = pixel_color;
	}
	else
	{
		throw std::invalid_argument("Out of bounds.");
	}
}

void BLIT(const unsigned int* Source_Array, unsigned int* Screen_Array, unsigned int Source_Width, unsigned int Source_Height, unsigned int Screen_Width, unsigned int Screen_Height, Rectangle_KC Source_Rect, unsigned int Destination_X, unsigned int Destination_Y)
{
	unsigned int copy_width = Source_Rect.right - Source_Rect.left;
	unsigned int copy_height = Source_Rect.bottom - Source_Rect.top;

	if (Source_Rect.left >= Source_Rect.right ||
		Source_Rect.top >= Source_Rect.bottom)
	{
		return;
	}

	for (unsigned int y = 0; y < copy_height; y++)
	{
		for (unsigned int x = 0; x < copy_width; x++)
		{
			unsigned int src_x = Source_Rect.left + x;
			unsigned int src_y = Source_Rect.top + y;

			unsigned int dst_x = Destination_X + x;
			unsigned int dst_y = Destination_Y + y;

			if (dst_x >= Screen_Width || dst_y >= Screen_Height)
				continue;

			if (src_x >= Source_Width || src_y >= Source_Height)
			{
				std::cout << "OUT OF BOUNDS\n";
				continue;
			}

			unsigned int src_index = D2_to_D1(src_x, src_y, Source_Width);
			unsigned int dst_index = D2_to_D1(dst_x, dst_y, Screen_Width);
			unsigned int converted = BGRAtoARGB(Source_Array[src_index]);
			Screen_Array[dst_index] = AlphaBlending(Screen_Array[dst_index], converted);
		}
	}
}

void Background_Tiles()
{
	unsigned int cols = RASTER_WIDTH / grass_width;
	unsigned int rows = RASTER_HEIGHT / grass_height;
	unsigned int missing_x = RASTER_WIDTH - (cols * grass_width);
	unsigned int missing_y = RASTER_HEIGHT - (rows * grass_height);

	for (unsigned int row = 0; row < rows; row++)
	{
		for (unsigned int col = 0; col < cols; col++)
		{
			Rectangle_KC src_rect = { GRASS_LEFT, GRASS_TOP, GRASS_RIGHT, GRASS_BOTTOM };
			BLIT(Tiles_pixels, SCREEN_ARRAY, Tiles_width, Tiles_height,
				RASTER_WIDTH, RASTER_HEIGHT, src_rect,
				col * grass_width, row * grass_height);
		}
	}

	for (unsigned int x = 0; x < rows; x++)
	{
		Rectangle_KC src_rect = { GRASS_LEFT, GRASS_TOP, GRASS_LEFT + missing_x, GRASS_BOTTOM };
		BLIT(Tiles_pixels, SCREEN_ARRAY, Tiles_width, Tiles_height,
			RASTER_WIDTH, RASTER_HEIGHT, src_rect,
			cols * grass_width, x * grass_height);
	}

	for (unsigned int y = 0; y < cols; y++)
	{
		Rectangle_KC src_rect = { GRASS_LEFT, GRASS_TOP, GRASS_RIGHT, GRASS_TOP + missing_y };
		BLIT(Tiles_pixels, SCREEN_ARRAY, Tiles_width, Tiles_height,
			RASTER_WIDTH, RASTER_HEIGHT, src_rect,
			y * grass_width, rows * grass_height);
	}

	Rectangle_KC src_rect = { GRASS_LEFT, GRASS_TOP, GRASS_LEFT + missing_x, GRASS_TOP + missing_y };
	BLIT(Tiles_pixels, SCREEN_ARRAY, Tiles_width, Tiles_height,
		RASTER_WIDTH, RASTER_HEIGHT, src_rect,
		cols * grass_width, rows * grass_height);
}

unsigned BGRAtoARGB(unsigned Color)
{
	unsigned old_blue = (Color & masc1) >> 24;
	unsigned old_green = (Color & masc2) >> 8;
	unsigned old_red = (Color & masc3) << 8;
	unsigned old_alpha = (Color & masc4) << 24;

	return old_blue | old_green | old_red | old_alpha;
}

unsigned AlphaBlending(unsigned int DestinationColor, unsigned int SourceColor)
{
	unsigned int SrcA = (SourceColor >> 24) & 0xFF;
	unsigned int SrcR = (SourceColor >> 16) & 0xFF;
	unsigned int SrcG = (SourceColor >> 8) & 0xFF;
	unsigned int SrcB = (SourceColor) & 0xFF;

	unsigned int DstA = (DestinationColor >> 24) & 0xFF;
	unsigned int DstR = (DestinationColor >> 16) & 0xFF;
	unsigned int DstG = (DestinationColor >> 8) & 0xFF;
	unsigned int DstB = (DestinationColor) & 0xFF;

	float AlphaRatio = SrcA / 255.0f;

	unsigned int FinalA = (unsigned int)(SrcA + DstA * (1.0f - AlphaRatio));
	unsigned int FinalR = (unsigned int)(AlphaRatio * SrcR + (1.0f - AlphaRatio) * DstR);
	unsigned int FinalG = (unsigned int)(AlphaRatio * SrcG + (1.0f - AlphaRatio) * DstG);
	unsigned int FinalB = (unsigned int)(AlphaRatio * SrcB + (1.0f - AlphaRatio) * DstB);

	return (FinalA << 24) | (FinalR << 16) | (FinalG << 8) | FinalB;
}