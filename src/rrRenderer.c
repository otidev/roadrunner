#include "rrInclude.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "rrRenderer.h"

static void _rrGetBlended(uint32_t* screenPixel, uint32_t imagePixel) {
	// Thank you, Dainel Peñalba (https://stackoverflow.com/a/36089930) and SDL
	
	// Separating the colour values
	uint8_t dstR = (*screenPixel & 0xff000000) >> 24;
	uint8_t dstG = (*screenPixel & 0x00ff0000) >> 16;
	uint8_t dstB = (*screenPixel & 0x0000ff00) >> 8;
	
	uint8_t srcR = (imagePixel & 0xff000000) >> 24;
	uint8_t srcG = (imagePixel & 0x00ff0000) >> 16;
	uint8_t srcB = (imagePixel & 0x0000ff00) >> 8;

	uint8_t dstAlpha = *screenPixel & 0x000000ff;
	uint8_t srcAlpha = imagePixel & 0x000000ff;

	// Do equation
	dstR = (uint8_t)((srcR * srcAlpha / 0xff) + (dstR * (1 - srcAlpha / 0xff)));
	dstG = (uint8_t)((srcG * srcAlpha / 0xff) + (dstG * (1 - srcAlpha / 0xff)));
	dstB = (uint8_t)((srcB * srcAlpha / 0xff) + (dstB * (1 - srcAlpha / 0xff)));

	dstAlpha = srcAlpha + (dstAlpha * (1 - (srcAlpha / 0xff)));

	// Put them back together, then blit
	uint32_t dstRGB = (dstR << 24) | (dstG << 16) | (dstB << 8) | (dstAlpha);
	*screenPixel = dstRGB;
}

void rrChangeEndianness(rrSurface* surf) {
	for (int x = 0; x < surf->width; x++) {
		for (int y = 0; y < surf->height; y++) {
			uint32_t srcPixel = surf->pixels[y * surf->width + x];
			surf->pixels[y * surf->width + x] = (srcPixel << 24) | (((srcPixel >> 16) << 24) >> 16) | (((srcPixel << 16) >> 24) << 16) | (srcPixel >> 24);
		}
	}
}

void rrBitmap(rrSurface* surf, int width, int height) {
	surf->width = width;
	surf->height = height;
	surf->bytesPerPixel = 4; // Forcing this for now, can change later
	surf->pixels = malloc(width * height * surf->bytesPerPixel);
	surf->blendMode = RR_MODE_NONE;
}

void rrInitImage(rrSurface* surf, char* filename) {
	surf->pixels = (uint32_t*)stbi_load(filename, &surf->width, &surf->height, &surf->bytesPerPixel, 0);

	// Reverse for big-endianness.
	rrChangeEndianness(surf);

	surf->blendMode = RR_MODE_NONE;
}

void rrBlit(rrSurface* srcSurf, rrSurface* dstSurf, rrPoint pos) {
	uint32_t* screenPixel = (uint32_t*)dstSurf->pixels;
	uint32_t* imagePixel = (uint32_t*)srcSurf->pixels;

	// Each pixel on image is equal to screen.
	for (int y = 0; y < srcSurf->height; y++) {
		for (int x = 0; x < srcSurf->width; x++) {
			if ((y + (int)pos.y) < dstSurf->height && (y + (int)pos.y) >= 0 && (x + (int)pos.x) < dstSurf->width && (x + (int)pos.x) >= 0) {
				if (dstSurf->blendMode == RR_MODE_BLEND) {
					_rrGetBlended(&screenPixel[(y + (int)pos.y) * dstSurf->width + (x + (int)pos.x)], imagePixel[y * srcSurf->width + x]);
				} else {
					screenPixel[(y + (int)pos.y) * dstSurf->width + (x + (int)pos.x)] = imagePixel[y * srcSurf->width + x];
					printf("%d\n", (y + (int)pos.y));
				}
			}
		}
	}
}

void rrBlitScaled(rrSurface* srcSurf, rrSurface* dstSurf, rrRect srcRect, rrRect dstRect) {
	uint32_t* screenPixel = (uint32_t*)dstSurf->pixels + (((int)dstRect.y * dstSurf->width) + (int)dstRect.x);
	uint32_t* imagePixel = (uint32_t*)srcSurf->pixels + (((int)srcRect.y * srcSurf->width) + (int)srcRect.x);

	float width, height;
	bool flipX = false, flipY = false;
	if (srcRect.width >= 0) {
		width = srcRect.width;
	} else {
		width = -srcRect.width;
		flipX = true;
	}
	
	if (srcRect.height >= 0) {
		height = srcRect.height;
	} else {
		height = -srcRect.height;
		flipY = true;
	}

	// X / Y scale
	float xScale = width / dstRect.width;
	float yScale = height / dstRect.height;

	for (int y = 0; y < (int)dstRect.height; y++) {
		for (int x = 0; x < (int)dstRect.width; x++) {
			if ((y + (int)dstRect.y) < dstSurf->height && (y + (int)dstRect.y) >= 0 && (x + (int)dstRect.x) < dstSurf->width && (x + (int)dstRect.x) >= 0) {
				if (dstSurf->blendMode == RR_MODE_BLEND) {
					_rrGetBlended(&screenPixel[y * dstSurf->width + x], imagePixel[(flipY ? (int)((dstRect.height - 1 - y) * yScale) : (int)(y * yScale)) * (int)srcSurf->width + (flipX ? (int)((dstRect.width - 1 - x) * xScale) : (int)(x * xScale))]);
				} else {
					screenPixel[y * dstSurf->width + x] = imagePixel[(flipY ? (int)((dstRect.height - 1 - y) * yScale) : (int)(y * yScale)) * (int)srcSurf->width + (flipX ? (int)((dstRect.width - 1 - x) * xScale) : (int)(x * xScale))];
				}
			}
		}
	}
}

void rrDrawPixel(rrPoint pos, uint32_t colour, rrSurface* surf) {
	uint32_t* pixel = (uint32_t*)surf->pixels;
	if (pos.y < surf->height && pos.y >= 0 && pos.x < surf->width && pos.x >= 0) {
		if (surf->blendMode == RR_MODE_BLEND) {
			_rrGetBlended(&pixel[(int)pos.y * surf->width + (int)pos.x], colour);
		} else {
			pixel[(int)pos.y * surf->width + (int)pos.x] = colour;
		}
	}
}

void rrClear(uint32_t colour, rrSurface* surf) {
	uint32_t* pixel = (uint32_t*)surf->pixels;
	for (int i = 0; i < surf->width * surf->height; i++) {
		if (surf->blendMode == RR_MODE_BLEND) {
			_rrGetBlended(&pixel[i], colour);
		} else {
			pixel[i] = colour;
		}
	}
}

void rrDrawRectangle(rrRect rect, uint32_t colour, rrSurface* surf) {
	uint32_t* pixel = (uint32_t*)surf->pixels;
	pixel += ((int)rect.y * surf->width) + (int)rect.x;
	
	for (int y = 0; y < (int)rect.height; y++) {
		for (int x = 0; x < (int)rect.width; x++) {
			if ((y + (int)rect.y) < surf->height && (y + (int)rect.y) >= 0 && (x + (int)rect.x) < surf->width && (x + (int)rect.x) >= 0) {
				if (surf->blendMode == RR_MODE_BLEND) {
					_rrGetBlended(&pixel[y * surf->width + x], colour);
				} else {
					pixel[y * surf->width + x] = colour;
				}
			}
		}
	}
}

void rrDrawRectangleLines(rrRect rect, uint32_t colour, rrSurface* surf) {
	uint32_t* pixel = (uint32_t*)surf->pixels;
	pixel += ((int)rect.y * surf->width) + (int)rect.x;
	
	for (int y = 0; y < (int)rect.height; y++) {
		for (int x = 0; x < (int)rect.width; x++) {
			if ((y + (int)rect.y) < surf->height && (y + (int)rect.y) >= 0 && (x + (int)rect.x) < surf->width && (x + (int)rect.x) >= 0) {
				if ((y == 0 || y == rect.height - 1) || (x == 0 || x == rect.width - 1)) {
					if (surf->blendMode == RR_MODE_BLEND) {
						_rrGetBlended(&pixel[y * surf->width + x], colour);
					} else {
						pixel[y * surf->width + x] = colour;
					}
				}
			}
		}
	}
}

void rrDrawTriangleLines(rrTri triangle, uint32_t colour, rrSurface* surf) {
	rrDrawLine(triangle.firstPoint, triangle.secondPoint, colour, surf);
	rrDrawLine(triangle.secondPoint, triangle.thirdPoint, colour, surf);
	rrDrawLine(triangle.thirdPoint, triangle.firstPoint, colour, surf);
}

void rrDrawTriangle(rrTri triangle, uint32_t colour, rrSurface* surf) {
	if (triangle.secondPoint.y < triangle.firstPoint.y) {
		rrPoint tmp = triangle.firstPoint;
		triangle.firstPoint = triangle.secondPoint;
		triangle.secondPoint = tmp;
	} if (triangle.thirdPoint.y < triangle.firstPoint.y) {
		rrPoint tmp = triangle.firstPoint;
		triangle.firstPoint = triangle.thirdPoint;
		triangle.thirdPoint = tmp;
	} if (triangle.thirdPoint.y < triangle.secondPoint.y) {
		rrPoint tmp = triangle.secondPoint;
		triangle.secondPoint = triangle.thirdPoint;
		triangle.thirdPoint = tmp;
	}

	// Thanks to this thread for the following code (https://stackoverflow.com/questions/34923406/filling-a-triangle-algorithm)
	
	double dx1 = (triangle.thirdPoint.x - triangle.firstPoint.x) / (triangle.thirdPoint.y - triangle.firstPoint.y);
	double dx2 = (triangle.secondPoint.x - triangle.firstPoint.x) / (triangle.secondPoint.y - triangle.firstPoint.y);
	double dx3 = (triangle.thirdPoint.x - triangle.secondPoint.x) / (triangle.thirdPoint.y - triangle.secondPoint.y);
	double x1 = triangle.firstPoint.x;
	double x2 = triangle.firstPoint.x;

	// loop through coordinates
	for (int y = triangle.firstPoint.y; y < triangle.secondPoint.y; y++) {
		rrDrawLine((rrPoint){x1, y}, (rrPoint){x2, y}, colour, surf);
		x1 += dx1;
		x2 += dx2;
	}
	
	x2 = triangle.secondPoint.x;

	// loop through coordinates
	for (int y = triangle.secondPoint.y; y < triangle.thirdPoint.y; y++) {
		rrDrawLine((rrPoint){x1, y}, (rrPoint){x2, y}, colour, surf);
		x1 += dx1;
		x2 += dx3;
	}
}

void rrDrawCircle(rrPoint center, int radius, uint32_t colour, rrSurface* surf) {
	// Bresenham, borrowed from Tigr.
	if (radius <= 0) {
		return;
	}

	int e = 1 - radius;
	int dstX = 0;
	int dstY = radius * -2;
	int srcX = 0;
	int srcY = radius;

	rrDrawLine((rrPoint){center.x - radius + 1, center.y}, (rrPoint){center.x + radius, center.y}, colour, surf);

	while (srcX < srcY - 1) {
		srcX++;

		if (e >= 0) {
			srcY--;
			dstY += 2;
			e += dstY;
			rrDrawLine((rrPoint){center.x - srcX + 1, center.y + srcY}, (rrPoint){center.x + srcX, center.y + srcY}, colour, surf);
			rrDrawLine((rrPoint){center.x - srcX + 1, center.y - srcY}, (rrPoint){center.x + srcX, center.y - srcY}, colour, surf);
		}

		dstX += 2;
		e += dstX + 1;

		if (dstX != dstY) {
			rrDrawLine((rrPoint){center.x - srcY + 1, center.y + srcX}, (rrPoint){center.x + srcY, center.y + srcX}, colour, surf);
			rrDrawLine((rrPoint){center.x - srcY + 1, center.y - srcX}, (rrPoint){center.x + srcY, center.y - srcX}, colour, surf);
		}
	}
}

void rrDrawCircleLines(rrPoint center, int radius, uint32_t colour, rrSurface* surf) {
	// Bresenham, also borrowed from Tigr.
	if (radius <= 0) {
		return;
	}

	int e = 1 - radius;
	int dstX = 0;
	int dstY = radius * -2;
	int srcX = 0;
	int srcY = radius;

	rrDrawPixel((rrPoint){center.x, center.y + radius}, colour, surf);
	rrDrawPixel((rrPoint){center.x, center.y - radius}, colour, surf);
	rrDrawPixel((rrPoint){center.x + radius, center.y}, colour, surf);
	rrDrawPixel((rrPoint){center.x - radius, center.y}, colour, surf);


	while (srcX < srcY - 1) {
		srcX++;

		if (e >= 0) {
			srcY--;
			dstY += 2;
			e += dstY;
		}

			rrDrawPixel((rrPoint){center.x + srcX, center.y + srcY}, colour, surf);
			rrDrawPixel((rrPoint){center.x - srcX, center.y + srcY}, colour, surf);
			rrDrawPixel((rrPoint){center.x + srcX, center.y - srcY}, colour, surf);
			rrDrawPixel((rrPoint){center.x - srcX, center.y - srcY}, colour, surf);

		dstX += 2;
		e += dstX + 1;

		if (dstX != dstY) {
			rrDrawPixel((rrPoint){center.x + srcY, center.y + srcX}, colour, surf);
			rrDrawPixel((rrPoint){center.x - srcY, center.y + srcX}, colour, surf);
			rrDrawPixel((rrPoint){center.x + srcY, center.y - srcX}, colour, surf);
			rrDrawPixel((rrPoint){center.x - srcY, center.y - srcX}, colour, surf);
		}
	}
}

void rrDrawLine(rrPoint startPos, rrPoint endPos, uint32_t colour, rrSurface* s) {
	// Bresenham, i think
	int dstX = abs((int)endPos.x - (int)startPos.x);
	int dstY = -abs((int)endPos.y - (int)startPos.y);
	int srcX = (int)startPos.x < (int)endPos.x ? 1 : -1;
	int srcY = (int)startPos.y < (int)endPos.y ? 1 : -1;
	int error = dstX + dstY;
	int e2;


	while (true) {
		rrDrawPixel((rrPoint){startPos.x, startPos.y}, colour, s);
		if ((int)startPos.x == (int)endPos.x && (int)startPos.y == (int)endPos.y)
			break;

		e2 = error << 1;
		if (e2 >= dstY) {
			error += dstY;
			startPos.x += srcX;
		}


		if (e2 <= dstX) {
			error += dstX;
			startPos.y += srcY;
		}
	}
	
}

void rrCopySurface(rrSurface* surf, void* dstPixels) {
	memcpy(dstPixels, surf->pixels, surf->width * surf->height * surf->bytesPerPixel);
}

void rrFreeSurface(rrSurface* surf) {
	free(surf->pixels);
	surf->pixels = NULL;
}

void rrSetBlendMode(rrSurface* surf, rrBlendMode blend) {
	surf->blendMode = blend;
}
