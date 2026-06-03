#include <iostream>
#include <stdexcept>
#include "RasterSurface.h"
#include "XTime.h"
#include "Tiles.h"
#include "Fire_Animation.h"
#define RASTER_WIDTH 500
#define RASTER_HEIGHT 500
#define NUM_PIXELS (RASTER_WIDTH*RASTER_HEIGHT)
#define GRASS_LEFT   126
#define GRASS_TOP    285
#define GRASS_RIGHT  162
#define GRASS_BOTTOM 321
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
	RS_Initialize("Katherine Chen", RASTER_WIDTH, RASTER_HEIGHT);

	int total_trees = 10;
	unsigned int tree_positions[10][2];
	int placed = 0;

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
		if (!collision)
		{
			tree_positions[placed][0] = new_x;
			tree_positions[placed][1] = new_y;
			placed++;
		}
	}

	do {

		ColorClean(0xFF000000);

		Background_Tiles();

		for (int tree = 0; tree < total_trees; tree++)
		{
			Rectangle_KC src_rect = { Tree_LEFT, Tree_TOP, Tree_RIGHT, Tree_BOTTOM };
			BLIT(Tiles_pixels, SCREEN_ARRAY, Tiles_width, Tiles_height,
				RASTER_WIDTH, RASTER_HEIGHT, src_rect,
				tree_positions[tree][0], tree_positions[tree][1]);
		}

	} while (RS_Update(SCREEN_ARRAY, NUM_PIXELS));

	RS_Shutdown();
}

void ColorClean(Color pixel_color)
{
	for (int pixel_position = 0; pixel_position < NUM_PIXELS; pixel_position++)
	{
		SCREEN_ARRAY[pixel_position] = BGRAtoARGB(pixel_color);
	}
}

int D2_to_D1(int y, int x, int D2_width)
{
	return ((y * D2_width) + x);
}

void DrawPixel(int x, int y, Color pixel_color)
{
	int dimension = D2_to_D1(y, x, RASTER_WIDTH);

	if (x >= 0 && x < RASTER_WIDTH && y >= 0 && y < RASTER_HEIGHT)
	{
		SCREEN_ARRAY[dimension] = BGRAtoARGB(pixel_color);
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

			unsigned int src_index = D2_to_D1(src_y, src_x, Source_Width);
			unsigned int dst_index = D2_to_D1(dst_y, dst_x, Screen_Width);
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

	unsigned int FinalA = (unsigned int)(AlphaRatio * SrcA + (1.0f - AlphaRatio) * DstA);
	unsigned int FinalR = (unsigned int)(AlphaRatio * SrcR + (1.0f - AlphaRatio) * DstR);
	unsigned int FinalG = (unsigned int)(AlphaRatio * SrcG + (1.0f - AlphaRatio) * DstG);
	unsigned int FinalB = (unsigned int)(AlphaRatio * SrcB + (1.0f - AlphaRatio) * DstB);

	return (FinalA << 24) | (FinalR << 16) | (FinalG << 8) | FinalB;
}