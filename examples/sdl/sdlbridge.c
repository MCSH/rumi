#include "SDL2/SDL.h"

SDL_PixelFormat *SDL_SurfaceGetFormat(SDL_Surface *surface){
  return surface->format;
}
