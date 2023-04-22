#ifndef INCLUDE_H
#define INCLUDE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.141592
#endif
#define RAD2DEG M_PI / 360.0 * 2.0
#define DEG2RAD M_PI / 180.0

typedef enum rrBlendMode {
	RR_MODE_BLEND = 0,
	RR_MODE_NONE,
} rrBlendMode;

typedef struct rrSurface {
	int width;
	int height;
	uint32_t* pixels;
	int bytesPerPixel;
	rrBlendMode blendMode;
} rrSurface;

typedef struct rrPoint {
	float x;
	float y;
} rrPoint;

typedef struct rrTri {
	rrPoint firstPoint;
	rrPoint secondPoint;
	rrPoint thirdPoint;
} rrTri;

typedef struct rrRect {
	float x;
	float y;
	float width;
	float height;
} rrRect;

#endif
