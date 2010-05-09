/**********************************************************************
File Name: draw.cpp
Author: Chris Wills                                        Date: 6/5/07
File Purpose: Basic Drawing functions used by most classes
Description: Drawing Functions implemented
Problems: None that we know of
**********************************************************************/
#include "stdafx.h"
#include "include/draw.h"
#include <string>

//Takes a given surface and applies it to the other surface.
//Precondition: 2 valid surfaces.
//Postcondition: the second surface contains the first surface at the (x,y) coords given.
void apply_surface(int x, int y, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip = NULL)
{
    //Temporary rectangle to hold the offsets
    SDL_Rect offset;
    
    //Get the offsets
    offset.x = x;
    offset.y = y;
    
    //Blit the surface
    SDL_BlitSurface( source, clip, destination, &offset );
}

//to load a pre-existing image
SDL_Surface *load_image( std::string filename ) 
{ 
  //The image that's loaded 
  SDL_Surface* loadedImage = NULL; 
  //The optimized image that will be used 
  SDL_Surface* optimizedImage = NULL; 
  //Load the image using SDL_image 
  loadedImage = IMG_Load( filename.c_str() ); 
  //If the image loaded 
  if( loadedImage != NULL ) 
  { 
    //Create an optimized image 
    optimizedImage = SDL_DisplayFormat( loadedImage ); 
    //Free the old image 
    SDL_FreeSurface( loadedImage ); 
  } 
  //Return the optimized image 
  return optimizedImage; 
}