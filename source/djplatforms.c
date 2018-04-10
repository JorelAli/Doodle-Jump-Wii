#include "djplatforms.h"
#include "djtextures.h"

#include <stdio.h> //This is required for NULL
#include <stdlib.h> //This is required for malloc/free etc.

Platform *platformArray;

// 0 = solo (and coop?)
// 1 = pvp
void initPlatformArr(int mode) {
	if(mode == 0) {
		platformArray = realloc(platformArray, NUM_PLATFORMS * sizeof(Platform));
		
		if(platformArray == NULL) {
			//realloc failed...
		}
		
	} else if(mode == 1) {
		platformArray = realloc(platformArray, NUM_PLATFORMS_PVP * sizeof(Platform));
		
		if(platformArray == NULL) {
			//realloc failed...
		}
	}
}

//Frees the platform array memory. Use this before re-initialising the platform array
void destroyPlatformArr() {
	free(platformArray);
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