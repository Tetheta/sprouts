/**********************************************************************
File Name: draw.h
Author: Chris Wills                                        Date: 6/5/07
File Purpose: Basic Drawing functions used by most classes
Description: Prototypes of the drawing functions
Problems: None that we know of
**********************************************************************/
#ifndef H_DRAW
#define H_DRAW

#include <string>
#include "SDL.h"
#include "SDL_image.h"

//Screen attributes
const int SCREEN_WIDTH  = 800;
const int SCREEN_HEIGHT = 600;
const int SCREEN_BPP    = 32;

struct Point
{
    int x;
    int y;
};

// Function to draw the surface
void apply_surface(int x, int y, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip);
SDL_Surface *load_image(std::string filename);

#endif
