#include "grrlibtext.h"
#include <stdarg.h>

//---------------------------------------------------------------------------------
void drawText(Alignment align, f32 ypos, GRRLIB_texImg *font, u32 colour, char *text, ...) {
//---------------------------------------------------------------------------------
	
	char temp[1024];		//variable to store our formatted text

	va_list args; 			//args variable
	va_start(args, text);	//initialize args variable, to get args in "text"
	vsnprintf(temp, sizeof(temp), text, args);	//create the string basically
	va_end(args);			//finishes up
	
	int tilew = font->tilew;	//The size of each character
	int strLength = strlen(temp);	//The length of the string inside temp

	switch(align) {
		case ALIGN_LEFT:
			GRRLIB_Printf(5, ypos, font, colour, 1, temp);
			break;
		case ALIGN_MIDDLE:
			GRRLIB_Printf(325, ypos, font, colour, 1, temp);
			break;
		case ALIGN_RIGHT:
			GRRLIB_Printf(635 - ((tilew * strLength)), ypos, font, colour, 1, temp);
			break;
		case ALIGN_CENTER:
			GRRLIB_Printf(320 - ((tilew * strLength) / 2), ypos, font, colour, 1, temp);
			break;
	}

}