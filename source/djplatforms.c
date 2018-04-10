#include "djplatforms.h"
#include "djtextures.h"

// 0 = solo (and coop?)
// 1 = pvp
void initPlatformArr(int mode) {
	
}

//---------------------------------------------------------------------------------
void drawPlatform(int x, int y, PlatformType type, int frame) {
//---------------------------------------------------------------------------------

	if(y <= 0) {
		return;	//Don't draw it if it's off screen!
	}
	
	switch(type) {
		case NORMAL:
			GRRLIB_DrawImg(x, y, GFX_Platform_Green, 0, 1, 1, RGBA(255, 255, 255, 255));
			break;
		case MOVING_HORIZ:
			GRRLIB_DrawImg(x, y, GFX_Platform_Blue, 0, 1, 1, RGBA(255, 255, 255, 255));
			break;
		case MOVING_VERT:
			GRRLIB_DrawImg(x, y, GFX_Platform_BlueH, 0, 1, 1, RGBA(255, 255, 255, 255));
			break;
		case BREAKING:
			GRRLIB_DrawTile(x, y, GFX_Platform_Brown, 0, 1, 1, RGBA(255, 255, 255, 255), frame);
			break;
		case GHOST:
			GRRLIB_DrawImg(x, y, GFX_Platform_White, 0, 1, 1, RGBA(255, 255, 255, 255));
			break;
		case SPRING:
			GRRLIB_DrawTile(x, y, GFX_Platform_Spring, 0, 1, 1, RGBA(255, 255, 255, 255), frame);
			break;
		case GOLD:
			GRRLIB_DrawTile(x, y, GFX_Platform_Gold, 0, 1, 1, RGBA(255, 255, 255, 255), frame);
			break;
		case NO_PLATFORM:
			break;
	}
	
}