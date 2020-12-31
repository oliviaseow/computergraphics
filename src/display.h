#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

extern SDL_Window* window;
extern SDL_Renderer* renderer;

extern SDL_Texture* color_buffer_texture;
// Declare a pointer to an array of uint32 elements
extern uint32_t* color_buffer;

int window_width;
int window_height;

bool initialize_window(void);
void draw_grid(void);
void draw_rect(int x, int y, int width, int height, uint32_t color);
void render_color_buffer();
void clear_color_buffer(uint32_t color);
void destroy_window(void);

#endif