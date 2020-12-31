#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "display.h"
#include "vector.h"

#define N_POINTS (9*9*9)

//const int N_POINTS = 9*9*9
vec3_t cube_points[N_POINTS];
vec2_t projected_points[N_POINTS];

vec3_t camera_position = { .x = 0, .y = 0, .z = -5};
vec3_t cube_rotation = { .x = 0, .y = 9, .z = 0};

float fov_factor = 640;

bool is_running = false;

int previous_frame_time = 0;

typedef struct {
	vec3_t position;
	vec3_t rotation;
	float fov_angle;
} camera_t;

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

	int point_count = 0;

	// Start loading an array of vectors from -1 to 1 (in this 9 by 9 by 9 cube)
	for (float x = -1; x <= 1; x += 0.25) {
		for (float y = -1; y <= 1; y += 0.25) {
			for (float z = -1; z <= 1; z += 0.25) {
				vec3_t new_point = {.x = x, .y = y, .z = z};
				cube_points[point_count++] = new_point;
			}
		}
	}
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

vec2_t project(vec3_t point) {
	//convert 3D vector to projected 2D point
	vec2_t projected_point = {
		.x = (fov_factor * point.x) / point.z, //scaling
		.y = (fov_factor * point.y) / point.z
	};
	return projected_point;
}


void update(void) {

	while (!SDL_TICKS_PASSED(SDL_GetTicks(), previous_frame_time + FRAME_TARGET_TIME));

	previous_frame_time = SDL_GetTicks(); //how many ms since SDL_Init

	cube_rotation.x += 0.01;
	cube_rotation.y += 0.01;
	cube_rotation.z += 0.01;

	for (int i = 0; i < N_POINTS; i++) {
		vec3_t point = cube_points[i];

		vec3_t transformed_point = vec3_rotate_x(point, cube_rotation.x);
		transformed_point = vec3_rotate_y(transformed_point, cube_rotation.y);
		transformed_point = vec3_rotate_z(transformed_point, cube_rotation.z);

		//translate the points away from the camera
		transformed_point.z -= camera_position.z;

		//project the current point
		vec2_t projected_point = project(transformed_point);

		//save the projected 2D vector in the array of projected points
		projected_points[i] = projected_point;
	}
}

void render(void) {
	// we don't need these anymore as we are using the color buffer
	// SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	// SDL_RenderClear(renderer);

	draw_grid();

	// draw_pixel(50, 50, 0xFFFFFF00);
	// draw_rect(300, 200, 300, 150, 0xFFFF00FF);

	for (int i = 0; i < N_POINTS; i++) {
		vec2_t projected_point = projected_points[i];
		draw_rect(
			projected_point.x + (window_width/2), //translation
			projected_point.y + (window_height/2),
			4,
			4,
			0xFFFFFF00);
	}

	render_color_buffer();

	clear_color_buffer(0xFF000000);

	SDL_RenderPresent(renderer);
}

int main(void) {
	
	is_running = initialize_window();

	setup();

	//vec3_t myvector = {2.0, 3.0, -4.0};

	camera_t mycam = {
		{ 0, 0, 0},
		{ 0.3, -2.0, 0.0 },
		0.78
	};

	// naive implementation is tied to speed of processor clock
	// but we don't have different machines to have different speeds
	// instead, we need to think about a fixed fps

	// to fix, we added a while loop in update()
	while (is_running) {
		process_input();
		update();
		render();
	}

	destroy_window();

	return 0;
}