#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

bool is_running = false;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

SDL_Texture* color_buffer_texture = NULL;
// Declare a pointer to an array of uint32 elements
uint32_t* color_buffer = NULL;

int window_width = 800;
int window_height = 600;

// Set the pixel at row 10 column 20 to the color red
//color_buffer[(window_width * 10) + 20] = 0xFFFF0000;

bool initialize_window(void) {
	if (SDL_Init(SDL_INIT_EVERYTHING) !=0 ) {
		fprintf(stderr, "Error initializing SDL.\n");
		return false;
	}

	//Set width and height of the SDL window to the max screen sze
	SDL_DisplayMode display_mode;
	SDL_GetCurrentDisplayMode(0, &display_mode);
	window_width = display_mode.w;
	window_height = display_mode.h;

	
	// Create SDL window
	window = SDL_CreateWindow(
		NULL,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		window_width,
		window_height,
		SDL_WINDOW_BORDERLESS
	);

	if (!window) {
		fprintf(stderr, "Error creating SDL window.\n");
		return false;
	}

	// Create SDL renderer
	renderer = SDL_CreateRenderer(
		window,
		-1,
		0);

	if (!renderer) {
		fprintf(stderr, "Error creating SDL renderer.\n");
		return false;
	}

	return true;
}


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

void draw_grid(void) {
	// for (int y = 0; y < window_height; y++) {
	// 	for (int x = 0; x < window_width; x++) {
	// 		if (x % 10 == 0 || y % 10 == 0) {
	// 			color_buffer[(window_width * y) + x] = 0xFF333333;
	// 		}
	// 	}
	// }

	//dot grid!
	for (int y = 0; y < window_height; y+=10) {
		for (int x = 0; x < window_width; x+=10) {
			color_buffer[(window_width * y) + x] = 0xFF333333;
		}
	}
}

void draw_rect(int x, int y, int width, int height, uint32_t color){
	for (int i=0; i < width; i++) {
		for (int j=0; j < height; j++) {
			int current_x = x + i;
			int current_y = y + j;
			color_buffer[(window_width * current_y) + current_x] = color;
		}
	}
}

void render_color_buffer() {
	//copy all content of color buffer and render it
	// https://wiki.libsdl.org/SDL_UpdateTexture

	//texture, sub-divisions, source, pitch (size of each row)
	SDL_UpdateTexture(
		color_buffer_texture,
		NULL,
		color_buffer,
		(int)(window_width * sizeof(uint32_t))
	);

	// https://wiki.libsdl.org/SDL_RenderCopy
	SDL_RenderCopy(renderer, color_buffer_texture, NULL, NULL);
}

void clear_color_buffer(uint32_t color) {
	for (int y = 0; y < window_height; y++) {
		for (int x = 0; x <window_width; x++) {
			color_buffer[(window_width * y) + x] = color;
		}
	}
	
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

void destroy_window(void) {
	free(color_buffer);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
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