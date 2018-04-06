#include "grrlibtext.h"

//---------------------------------------------------------------------------------
void drawText(Alignment align, f32 ypos, GRRLIB_texImg *font, u32 colour, char *text) {
//---------------------------------------------------------------------------------
	
	int tilew = font->tilew;
	int strLength = strlen(text);
	switch(align) {
		case LEFT:
			GRRLIB_Printf(5, ypos, font, colour, 1, text);
			break;
		case RIGHT:
			GRRLIB_Printf(635 - ((tilew * strLength)), ypos, font, colour, 1, text);
			break;
		case CENTER:
			GRRLIB_Printf(320 - ((tilew * strLength) / 2), ypos, font, colour, 1, text);
			break;
	}

}