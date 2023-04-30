#include "Roadrunner.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main() {
	rrSurface mainSurface;
	rrSurface sth;
	rrBitmap(&mainSurface, 256, 256);
	rrClear(0xff0000ff, &mainSurface);

	rrDrawCircleLines((rrPoint){(mainSurface.width / 2), (mainSurface.height / 2)}, (mainSurface.height / 2), 0xffffffff, &mainSurface);
	rrDrawCircle((rrPoint){(mainSurface.width / 2), (mainSurface.height / 2)}, 20 - 1, 0x7f7f7fff, &mainSurface);

	rrInitImage(&sth, "tojisnip.png");

	rrSetBlendMode(&mainSurface, RR_MODE_BLEND);
	rrDrawTriangle((rrTri){(rrPoint){40, 20}, (rrPoint){40, 30}, (rrPoint){80, 30}}, 0x00ff00ff, &mainSurface);
	rrDrawRectangleLines((rrRect){20, 20, 10, 10}, 0xffffffff, &mainSurface);
	rrDrawRectangle((rrRect){30, 30, 10, 10}, 0xffff00f0, &mainSurface);
	rrBlitScaled(&sth, &mainSurface, (rrRect){0, 0, 16, 32}, (rrRect){(mainSurface.height / 2) - (32 / 2), (mainSurface.height / 2) - (64 / 2), 32, 64}, (rrPoint){16 / 2, 32 / 2}, 0);
	rrSetBlendMode(&mainSurface, RR_MODE_NONE);

	rrChangeEndianness(&mainSurface);

	stbi_write_png("out.png", mainSurface.width, mainSurface.height, mainSurface.bytesPerPixel, mainSurface.pixels, mainSurface.width * mainSurface.bytesPerPixel);
	rrFreeSurface(&mainSurface);
	rrFreeSurface(&sth);
	return 0;
}
