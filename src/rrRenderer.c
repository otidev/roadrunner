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
	memset(surf->pixels, 0, width * height * surf->bytesPerPixel);
	surf->blendMode = RR_MODE_NONE;
}

void rrBitmapImage(rrSurface* surf, char* filename) {
	surf->pixels = (uint32_t*)stbi_load(filename, &surf->width, &surf->height, &surf->bytesPerPixel, 0);

	// Reverse for big-endianness.
	rrChangeEndianness(surf);

	surf->blendMode = RR_MODE_NONE;
}

void rrBlit(rrSurface* srcSurf, rrSurface* dstSurf, rrPoint pos, float rotation) {
	uint32_t* screenPixel = (uint32_t*)dstSurf->pixels;
	uint32_t* imagePixel = (uint32_t*)srcSurf->pixels;

	// Each pixel on image is equal to screen.
	for (int y = 0; y < srcSurf->height; y++) {
		for (int x = 0; x < srcSurf->width; x++) {
			int dstY = (int)(sin(rotation * DEG2RAD) * x + cos(rotation * DEG2RAD) * y);
			int dstX = (int)(cos(rotation * DEG2RAD) * x - sin(rotation * DEG2RAD) * y);

			if ((dstY + (int)pos.y) < dstSurf->height && (dstY + (int)pos.y) >= 0 && (dstX + (int)pos.x) < dstSurf->width && (dstX + (int)pos.x) >= 0) {
				if (dstSurf->blendMode == RR_MODE_BLEND) {
					_rrGetBlended(
						&screenPixel[(dstY + (int)pos.y) * dstSurf->width + (dstX + (int)pos.x)],
						imagePixel[y * srcSurf->width + x]
					);
				} else {
					screenPixel[(dstY + (int)pos.y) * dstSurf->width + (dstX + (int)pos.x)] = imagePixel[y * srcSurf->width + x];
				}
			}
		}
	}
}

void rrBlitScaled(rrSurface* srcSurf, rrSurface* dstSurf, rrRect srcRect, rrRect dstRect, rrPoint center, float rotation) {
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
			int posY = (flipY ? (int)((dstRect.height - 1 - y) * yScale) : (int)(y * yScale));
			int posX = (flipX ? (int)((dstRect.width - 1 - x) * xScale) : (int)(x * xScale));
			int dstY = (int)(sin(rotation * DEG2RAD) * (x - (int)center.x) + cos(rotation * DEG2RAD) * (y - (int)center.y)) + (int)center.y;
			int dstX = (int)(cos(rotation * DEG2RAD) * (x - (int)center.x) - sin(rotation * DEG2RAD) * (y - (int)center.y)) + (int)center.x;

			if ((dstY + (int)dstRect.y) < dstSurf->height && (dstY + (int)dstRect.y) >= 0 && (dstX + (int)dstRect.x) < dstSurf->width && (dstX + (int)dstRect.x) >= 0) {
				if (dstSurf->blendMode == RR_MODE_BLEND) {
					_rrGetBlended(&screenPixel[dstY * dstSurf->width + dstX], imagePixel[posY * (int)srcSurf->width + posX]);
				} else {
					screenPixel[dstY * dstSurf->width + dstX] = imagePixel[posY * (int)srcSurf->width + posX];
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

static int cp1252[] = {
    0x20ac, 0xfffd, 0x201a, 0x0192, 0x201e, 0x2026, 0x2020, 0x2021, 0x02c6, 0x2030, 0x0160, 0x2039, 0x0152,
    0xfffd, 0x017d, 0xfffd, 0xfffd, 0x2018, 0x2019, 0x201c, 0x201d, 0x2022, 0x2013, 0x2014, 0x02dc, 0x2122,
    0x0161, 0x203a, 0x0153, 0xfffd, 0x017e, 0x0178,
};

void rrLoadFont(rrFont* font, char* fontFilename, int codepage) {
	font->codepage = codepage;
	if (codepage == 0) {
		font->numGlyphs = 128 - 32;
	} if (codepage == 1252) {
		font->numGlyphs = 256 - 32;
	}
	rrBitmapImage(&font->surface, fontFilename);

	font->glyphs = malloc(sizeof(rrGlyph) * font->numGlyphs);


	int x = 0, y = 1; // Start in the character
	for (int i = 0; i < font->numGlyphs; i++) {
		uint32_t origPixel = font->surface.pixels[0];
		int w = 0, h = 0;
		x += 1;

		while (font->surface.pixels[y * font->surface.width + (x + w)] != origPixel) {
			w++;
		}

		while (font->surface.pixels[(y + h) * font->surface.width + x] != origPixel) {
			h++;
		}

		if (w == 0) {
			y += font->glyphs[0].rect.height + 1;
			x = 0;
			i--;
		} else {
			if (i > 128 && i < 160) {
				font->glyphs[i].code = cp1252[i - 128];
			} else {
				font->glyphs[i].code = i;
			}

			font->glyphs[i] = (rrGlyph){(rrRect){x, y, w, h}};
			if (font->surface.pixels[y * font->surface.width + (x + w)] == origPixel) {
				x += w;
			}
		}
	}
}

// Taken from Tigr
static const char* DecodeUTF8(const char* text, int* cp) {
    unsigned char c = *text++;
    int extra = 0, min = 0;
    *cp = 0;
    if (c >= 0xf0) {
        *cp = c & 0x07;
        extra = 3;
        min = 0x10000;
    } else if (c >= 0xe0) {
        *cp = c & 0x0f;
        extra = 2;
        min = 0x800;
    } else if (c >= 0xc0) {
        *cp = c & 0x1f;
        extra = 1;
        min = 0x80;
    } else if (c >= 0x80) {
        *cp = 0xfffd;
    } else {
        *cp = c;
    }
    while (extra--) {
        c = *text++;
        if ((c & 0xc0) != 0x80) {
            *cp = 0xfffd;
            break;
        }
        (*cp) = ((*cp) << 6) | (c & 0x3f);
    }
    if (*cp < min) {
        *cp = 0xfffd;
    }
    return text;
}

int rrTextWidth(rrFont* font, char* text) {
	int x = 0, width = 0;
	const char* pointer = text;
	for (int i = 0; i < strlen(text); i++) {
		int letter = *(char*)(text + i);
		pointer = DecodeUTF8(pointer, &letter);

		if (letter == '\t') {
			x += rrTextWidth(font, " ") * 4; // deal with it.
			width = x > width ? x : width;
		} if (letter == '\n') {
			x = 0;
		} else {
			x += font->glyphs[letter - 32].rect.width;
			width = x > width ? x : width;
		}
	}

	return width;
}

int rrTextHeight(rrFont* font, char* text) {
	int y = font->glyphs[0].rect.height;
	const char* pointer = text;
	for (int i = 0; i < strlen(text); i++) {
		int letter = *(char*)(text + i);
		pointer = DecodeUTF8(pointer, &letter);

		if (letter == '\n') {
			y += font->glyphs[0].rect.height;
		}
	}

	return y;
}

void rrDrawText(rrFont* font, rrPoint pos, char* text, rrSurface* surf) {
	int x = pos.x, y = pos.y;
	const char* pointer = text;
	for (int i = 0; i < strlen(text); i++) {
		int letter = *(char*)(text + i);
		pointer = DecodeUTF8(pointer, &letter);

		if (letter == '\n') {
			x = pos.x;
			y += rrTextHeight(font, " ");
			goto done;
		}

		if (letter == '\t') {
			x += rrTextWidth(font, " ") * 4;
			goto done;
		}

		rrBlitScaled(&font->surface, surf, font->glyphs[letter - 32].rect, (rrRect){x, y, font->glyphs[letter - 32].rect.width, font->glyphs[letter - 32].rect.height}, (rrPoint){0}, 0);
		x += font->glyphs[letter - 32].rect.width;

		done:
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
