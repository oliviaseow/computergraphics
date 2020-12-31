#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "display.h"

bool is_running = false;

void setup(void) {
	// Allocate the required bytes in memory for the color buffer
	// Cast to uint32_t, which is type of color buffer
	color_buffer = (uint32_t*) malloc(sizeof(uint32_t) * window_width * window_height);

	// Create SDL texture that is used to display the color buffer
	// https://wiki.libsdl.org/SDL_PixelFormat
	color_buffer_texture = SDL_CreateTexture(
		renderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		window_width,
		window_height
	);

	// if (!color_buffer_texture) {
	// 	fprintf(stderr, "Error creating SDL texture.\n");
	// 	return false;
	// }
}

void process_input(void) {
	SDL_Event event;
	SDL_PollEvent(&event);

	switch (event.type) {
		case SDL_QUIT: //x button of window
			is_running = false;
			break;
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE)
				is_running = false;
			break;
	}
}


void update(void) {

}

void render(void) {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);

	draw_grid();

	draw_rect(300, 200, 300, 150, 0xFFFF00FF);

	render_color_buffer();
	clear_color_buffer(0xFF000000);

	SDL_RenderPresent(renderer);
}

int main(void) {
	
	is_running = initialize_window();

	setup();

	while (is_running) {
		process_input();
		update();
		render();
	}

	destroy_window();

	return 0;
}