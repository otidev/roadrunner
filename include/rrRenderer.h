#ifndef RR_RENDERER_H
#define RR_RENDERER_H

#include "rrInclude.h"

// Changes the endianness of a surface
// surf - The surface to change endianness
void rrChangeEndianness(rrSurface* surf);

// Creates an empty surface.
// surf - Destination surface.
// width - Width in pixels for the surface.
// height - Height in pixels for the surface.
void rrBitmap(rrSurface* surf, int width, int height);

// Opens an image into a surface using stbi.
// surf - Destination surface.
// filename - The filename of the image.
void rrBitmapImage(rrSurface* surf, char* filename);

// Blits a surface onto another surface.
// srcSurf - Source surface.
// dstSurf - Destination surface.
// pos - Position to render srcSurf in dstSurf.
// rotation - How many degrees srcSurf should be spun.
// NOTE: As of now, the alpha formula is Asrc = Adst.
void rrBlit(rrSurface* srcSurf, rrSurface* dstSurf, rrPoint pos, float rotation);

// Blits a surface onto another surface, with scaling.
// srcSurf - Source surface.
// dstSurf - Destination surface.
// srcRect - Rectangle in srcSurf where we should blit.
// dstRect - Rectangle in dstSurf where we should blit.
// center - Center of rotaiton (NOTE: offsets the bitmap)
// rotation - How many degrees srcSurf should be spun.
// NOTE: As of now, the alpha formula is Asrc = Adst.
void rrBlitScaled(rrSurface* srcSurf, rrSurface* dstSurf, rrRect srcRect, rrRect dstRect, rrPoint center, float rotation);

// Draws a pixel on a surface.
// pos - Pixel position.
// colour - The colour of the pixel.
// surf - Destination surface.
void rrDrawPixel(rrPoint pos, uint32_t colour, rrSurface* surf);

// Clears a surface.
// colour - The colour to clear the surface in.
// surf - Destination surface.
void rrClear(uint32_t colour, rrSurface* surf);

// Draws a filled rectangle.
// rect - Rectangle to draw.
// colour - The colour to draw the rectangle in.
// surf - Destination surface.
void rrDrawRectangle(rrRect rect, uint32_t colour, rrSurface* surf);

// Draws the lines of a rectangle.
// rect - Rectangle to draw.
// colour - The colour to draw the rectangle in.
// surf - Destination surface.
void rrDrawRectangleLines(rrRect rect, uint32_t colour, rrSurface* surf);

// Draws the lines of a triangle.
// rect - Rectangle to draw.
// colour - The colour to draw the triangle in.
// surf - Destination surface.
void rrDrawTriangleLines(rrTri triangle, uint32_t colour, rrSurface* surf);

// Draws a triangle.
// rect - Rectangle to draw.
// colour - The colour to draw the triangle in.
// surf - Destination surface.
void rrDrawTriangle(rrTri triangle, uint32_t colour, rrSurface* surf);

// Draws the lines of a circle.
// center - Center of the circle.
// radius - Radius of the circle.
// colour - The colour to draw the circle in.
// surf - Destination surface.
void rrDrawCircle(rrPoint center, int radius, uint32_t colour, rrSurface* surf);

// Draws the lines of a circle.
// center - Center of the circle.
// radius - Radius of the circle.
// colour - The colour to draw the circle in.
// surf - Destination surface.
void rrDrawCircleLines(rrPoint center, int radius, uint32_t colour, rrSurface* surf);

// Draws a line using Bresenham's.
// startPos - The start position of the line.
// endPos - The end position of the line.
// colour - The colour to draw the circle in.
// surf - Destination surface.
void rrDrawLine(rrPoint startPos, rrPoint endPos, uint32_t colour, rrSurface* surf);

// Loads a font.
// font - The font to load into.
// fontFilename - The filename of the font to load.
// codepage - The codepage of the font.
void rrLoadFont(rrFont* font, char* fontFilename, int codepage);

// Returns the width of text.
// font - The font to calculate with.
// text - The text to calculate.
int rrTextWidth(rrFont* font, char* text);

// Returns the height of text.
// font - The font to calculate with.
// text - The text to calculate.
int rrTextHeight(rrFont* font, char* text);

// Draws text.
// font - The font to use.
// pos - The point to start rendering.
// text - The text to render.
// surf - Destination surface.
void rrDrawText(rrFont* font, rrPoint pos, char* text, rrSurface* surf);

// Copies a whole surface to a whole array of pixels.
// surf - Source surface.
// dstPixels - Destination array of pixels not used by roadrunner.
void rrCopySurface(rrSurface* surf, void* dstPixels);

// Frees a surface.
// surf - Surface to free.
// NOTE: This does not free a surface, only its *pixels*. It then NULLs the surface.
void rrFreeSurface(rrSurface* surf);

// Sets the blend mode of a surface.
// surf - Destination surface.
// blend - Blend mode to use.
void rrSetBlendMode(rrSurface* surf, rrBlendMode blend);

#ifdef RR_SHORTNAMES
    #define ChangeEndianness rrChangeEndianness
    #define Bitmap rrBitmap
    #define InitImage rrInitImage
    #define Blit rrBlit
    #define BlitScaled rrBlitScaled
    #define DrawPixel rrDrawPixel
    #define Clear rrClear
    #define DrawRectangle rrDrawRectangle
    #define DrawRectangleLines rrDrawRectangleLines
    #define DrawTriangleLines rrDrawTriangleLines
    #define DrawTriangle rrDrawTriangle
    #define DrawCircle rrDrawCircle
    #define DrawCircleLines rrDrawCircleLines
    #define DrawLine rrDrawLine
    #define LoadFont rrLoadFont
    #define TextWidth rrTextWidth
    #define TextHeight rrTextHeight
    #define DrawText rrDrawText
    #define CopySurface rrCopySurface
    #define FreeSurface rrFreeSurface
    #define SetBlendMode rrSetBlendMode
#endif

#endif
