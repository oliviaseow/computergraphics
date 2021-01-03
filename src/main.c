#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "array.h"
#include "display.h"
#include "vector.h"
#include "mesh.h"

// #define N_POINTS (9*9*9)

//const int N_POINTS = 9*9*9

///////////////////////////////////////////////////////////////////////////////
// Array of triangles that should be rendered frame by frame
///////////////////////////////////////////////////////////////////////////////

triangle_t* triangles_to_render = NULL;


///////////////////////////////////////////////////////////////////////////////
// Global variables for execution status and game loop
///////////////////////////////////////////////////////////////////////////////

bool is_running = false;
int previous_frame_time = 0;


vec3_t camera_position = { 0, 0, 0};
//vec3_t cube_rotation = { .x = 0, .y = 0, .z = 0};
float fov_factor = 640;//640;

// typedef struct {
// 	vec3_t position;
// 	vec3_t rotation;
// 	float fov_angle;
// } camera_t;

///////////////////////////////////////////////////////////////////////////////
// Setup function to initialize variables and game objects
///////////////////////////////////////////////////////////////////////////////

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

	// loads the hard coded cube values in the mesh data structure
	//load_cube_mesh_data();

	load_obj_file_data("./assets/cube2.obj");

	// vec3_t a = { 2.5,  6.4,  3.0};
	// vec3_t b = { -2.2, 1.4, -1.0};

	// float a_length = vec3_length(a);
	// float b_length = vec3_length(b);

	// vec3_t add_ab = vec3_add(a,b);
}

///////////////////////////////////////////////////////////////////////////////
// Poll system events and handle keyboard input
///////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////
// Function that receives a 3D vector and returns a projected 2D point
////////////////////////////////////////////////////////////////////////////////

vec2_t project(vec3_t point) {
	//convert 3D vector to projected 2D point
	vec2_t projected_point = {
		.x = (fov_factor * point.x) / point.z, //scaling
		.y = (fov_factor * point.y) / point.z
	};
	return projected_point;
}

///////////////////////////////////////////////////////////////////////////////
// Update function frame by frame with a fixed time step
///////////////////////////////////////////////////////////////////////////////

void update(void) {
	// while loops are processor instructions, part of the executor
	// process needs to share CPU with other tasks and not burn a lot of resources doing nothing
	// while loops are going to consume 100% of CPU core, don't want to have processor caught up here
	// while (!SDL_TICKS_PASSED(SDL_GetTicks(), previous_frame_time + FRAME_TARGET_TIME));

	// https://wiki.libsdl.org/SDL_Delay are using OS instructions to yield attention to other processes
	int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);

	if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) {
		SDL_Delay(time_to_wait);
	}

	previous_frame_time = SDL_GetTicks(); //how many ms since SDL_Init

	// Initialize the array of triangles to render
	triangles_to_render = NULL; //replace at every loop

	mesh.rotation.x += 0.01;
	mesh.rotation.y += 0.01;
	mesh.rotation.z += 0.01;

	// loop all triangle faces
	int num_faces = array_length(mesh.faces);

	for (int i = 0; i < num_faces; i ++) {
		face_t mesh_face = mesh.faces[i];
		vec3_t face_vertices[3];
		face_vertices[0] = mesh.vertices[mesh_face.a - 1];
		face_vertices[1] = mesh.vertices[mesh_face.b - 1];
		face_vertices[2] = mesh.vertices[mesh_face.c - 1];

		triangle_t projected_triangle;

		vec3_t transformed_vertices[3];

		// PERFORM TRANSFORMATION

		//loop all three vertices of this current face and apply transformations
		for (int j = 0; j < 3; j++) {
			vec3_t transformed_vertex = face_vertices[j];

			transformed_vertex = vec3_rotate_x(transformed_vertex, mesh.rotation.x);
			transformed_vertex = vec3_rotate_y(transformed_vertex, mesh.rotation.y);
			transformed_vertex = vec3_rotate_z(transformed_vertex, mesh.rotation.z);

			// translate the vertex away from the camera
			transformed_vertex.z += 5;

			// Save transformed vertex in the array of transformed vertices
			transformed_vertices[j] = transformed_vertex;

		}
		
		// CHECK BACKFACE CULLING
		vec3_t vector_a = transformed_vertices[0]; //   A
		vec3_t vector_b = transformed_vertices[1]; // /   \ //
		vec3_t vector_c = transformed_vertices[2]; // C---B

		// Get the vector subtraction of B-A and C-A
		vec3_t vector_ab = vec3_subtract(vector_b, vector_a);
		vec3_t vector_ac = vec3_subtract(vector_c, vector_a);
		vec3_normalize(&vector_ab);
		vec3_normalize(&vector_ac);

		// compute face normal using cross product to find perpendicular
		// because we're using left handed system handedness, z values grow inside monitor
		// for cross product to work properly
		vec3_t normal = vec3_cross(vector_ab, vector_ac);

		//normalize the face normal vector
		vec3_normalize(&normal); //exercise to pass a variable by reference

		// find vector bewteen point in the triangle and the camera origin
		vec3_t camera_ray = vec3_subtract(camera_position, vector_a);

		// calculate how aligned face normal is with camera ray using dot product
		float dot_normal_camera = vec3_dot(normal, camera_ray);

		// bypass triangles that are looking away from the camera
		if (dot_normal_camera < 0) {
			continue;
		}

		// PERFORM PROJECTION FROM 3D TO 2D FACES by looping all three vertices
		for (int j = 0; j < 3; j++) {	

			// project current vertex
			vec2_t projected_point = project(transformed_vertices[j]);

			// sclae and translate projected points to the middle of the screen
			projected_point.x += (window_width / 2);
			projected_point.y += (window_height / 2);

			projected_triangle.points[j] = projected_point;
		}

		//save projected triangle in array of triangles to render
		//triangles_to_render[i] = projected_triangle;
		array_push(triangles_to_render, projected_triangle);
	}
}

	/*

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
	*/

///////////////////////////////////////////////////////////////////////////////
// Render function to draw objects on the display
///////////////////////////////////////////////////////////////////////////////

void render(void) {
	// we don't need these anymore as we are using the color buffer
	// SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	// SDL_RenderClear(renderer);

	draw_grid();

	int num_triangles = array_length(triangles_to_render);

	// draw_pixel(50, 50, 0xFFFFFF00);
	// draw_rect(300, 200, 300, 150, 0xFFFF00FF);
	//draw_line(100, 200, 300, 50, 0xFF00FF00);

	// loop all projected triangles and render them
	// 	for (int i = 0; i < num_triangles; i++) {
	// 	//for (int i = 0; i < N_MESH_FACES; i++) {
	// 		triangle_t triangle = triangles_to_render[i];

	// 		//Draw vertex points
	// 		draw_rect(triangle.points[0].x, triangle.points[0].y, 3, 3, 0xFFFFFF00);
	// 		draw_rect(triangle.points[1].x, triangle.points[1].y, 3, 3, 0xFFFFFF00);
	// 		draw_rect(triangle.points[2].x, triangle.points[2].y, 3, 3, 0xFFFFFF00);

	// 		//Draw unfilled triangle faces
	// 		draw_triangle(
	// 			triangle.points[0].x, triangle.points[0].y, //vertex A
	// 			triangle.points[1].x, triangle.points[1].y, //vertex B
	// 			triangle.points[2].x, triangle.points[2].y, //vertex C
	// 			0xFF00FF00);
	// }

	//loop all projected triangles and render them
		for (int i = 0; i < num_triangles; i++) {
		//for (int i = 0; i < N_MESH_FACES; i++) {
			triangle_t triangle = triangles_to_render[i];

			//Draw vertex points
			draw_rect(triangle.points[0].x, triangle.points[0].y, 3, 3, 0xFFFFFF00);
			draw_rect(triangle.points[1].x, triangle.points[1].y, 3, 3, 0xFFFFFF00);
			draw_rect(triangle.points[2].x, triangle.points[2].y, 3, 3, 0xFFFFFF00);

			//Draw filled triangle faces
			draw_filled_triangle(
				triangle.points[0].x, triangle.points[0].y, //vertex A
				triangle.points[1].x, triangle.points[1].y, //vertex B
				triangle.points[2].x, triangle.points[2].y, //vertex C
				0xFF00FF00);

			//Draw unfilled triangle faces
			draw_triangle(
				triangle.points[0].x, triangle.points[0].y, //vertex A
				triangle.points[1].x, triangle.points[1].y, //vertex B
				triangle.points[2].x, triangle.points[2].y, //vertex C
				0x00000000);
	}

	//draw_filled_triangle(300, 100, 50, 400, 500, 700, 0xFF00FF00);

	// for (int i = 0; i < N_POINTS; i++) {
	// 	vec2_t projected_point = projected_points[i];
	// 	draw_rect(
	// 		projected_point.x + (window_width/2), //translation
	// 		projected_point.y + (window_height/2),
	// 		4,
	// 		4,
	// 		0xFFFFFF00);
	// }

	// Clear the array of triangles to render every frame loop
	array_free(triangles_to_render);
	
	render_color_buffer();

	clear_color_buffer(0xFF000000);

	SDL_RenderPresent(renderer);

}

///////////////////////////////////////////////////////////////////////////////
// Free the memory that was dynamically allocated by the program
///////////////////////////////////////////////////////////////////////////////


void free_resources(void) {
	free(color_buffer); //raw free call
	array_free(mesh.faces); //wrapper to free dynamic array
	array_free(mesh.vertices);
}

///////////////////////////////////////////////////////////////////////////////
// Main function
///////////////////////////////////////////////////////////////////////////////

int main(void) {
	
	is_running = initialize_window();

	setup();

	//vec3_t myvector = {2.0, 3.0, -4.0};

	// camera_t mycam = {
	// 	{ 0, 0, 0},
	// 	{ 0.3, -2.0, 0.0 },
	// 	0.78
	// };

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
	free_resources();

	return 0;
}