#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "vector.h"

// Declare a new type to hold face information
typedef struct {
	int a, b, c;
} face_t;

typedef struct {
	vec2_t points[3];
} triangle_t;

#endif