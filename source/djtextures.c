#include "djtextures.h"

//Background
GRRLIB_texImg *GFX_Background;
GRRLIB_texImg *GFX_Bar;

//Players
GRRLIB_texImg *GFX_Player_Left;
GRRLIB_texImg *GFX_Player_Right;
GRRLIB_texImg *GFX_Player_Left2;
GRRLIB_texImg *GFX_Player_Right2;

//Platforms
GRRLIB_texImg *GFX_Platform_Green;
GRRLIB_texImg *GFX_Platform_Blue;
GRRLIB_texImg *GFX_Platform_Brown;
GRRLIB_texImg *GFX_Platform_White;
GRRLIB_texImg *GFX_Platform_Spring;
GRRLIB_texImg *GFX_Platform_Gold;
GRRLIB_texImg *GFX_Platform_BlueH;

//Obstacles
GRRLIB_texImg *GFX_Obstacle_BlackHole;

//Fonts
GRRLIB_texImg *FONT_Doodle;
GRRLIB_texImg *FONT_Doodle_Bold;

//---------------------------------------------------------------------------------
void TEXTURES_Init() {
//---------------------------------------------------------------------------------

	//Background
	GFX_Background = GRRLIB_LoadTexture(background);
	GFX_Bar = GRRLIB_LoadTexture(topbar);

	//Players
	GFX_Player_Left = GRRLIB_LoadTexture(doodleL);
	GFX_Player_Right = GRRLIB_LoadTexture(doodleR);
	GFX_Player_Left2 = GRRLIB_LoadTexture(doodleL2);
	GFX_Player_Right2 = GRRLIB_LoadTexture(doodleR2);
	
	//Platforms
	GFX_Platform_Green = GRRLIB_LoadTexture(pgreen);
	GFX_Platform_Blue = GRRLIB_LoadTexture(pblue);
	GFX_Platform_BlueH = GRRLIB_LoadTexture(pbluevert);

	GFX_Platform_Brown = GRRLIB_LoadTexture(pbrown_all);
	GRRLIB_InitTileSet(GFX_Platform_Brown, 68, 20, 0);
	
	GFX_Platform_White = GRRLIB_LoadTexture(pwhite);
	
	GFX_Platform_Spring = GRRLIB_LoadTexture(pspring);
	GRRLIB_InitTileSet(GFX_Platform_Spring, 58, 36, 0);
	
	GFX_Platform_Gold = GRRLIB_LoadTexture(pgold);
	GRRLIB_InitTileSet(GFX_Platform_Gold, 64, 24, 0);
	
	//Obstacles
	GFX_Obstacle_BlackHole = GRRLIB_LoadTexture(blackhole);
	
	//Fonts
	FONT_Doodle = GRRLIB_LoadTexture(Al_seana_14);
	GRRLIB_InitTileSet(FONT_Doodle, 14, 22, 32);
	
	FONT_Doodle_Bold = GRRLIB_LoadTexture(Al_seana_16_Bold);
	GRRLIB_InitTileSet(FONT_Doodle_Bold, 17, 24, 32);
}

//---------------------------------------------------------------------------------
void TEXTURES_Exit() {
//---------------------------------------------------------------------------------

	//Background
	GRRLIB_FreeTexture(GFX_Background);
	GRRLIB_FreeTexture(GFX_Bar);

	//Players
	GRRLIB_FreeTexture(GFX_Player_Left);
	GRRLIB_FreeTexture(GFX_Player_Right);
	GRRLIB_FreeTexture(GFX_Player_Left2);
	GRRLIB_FreeTexture(GFX_Player_Right2);
	
	//Platforms
	GRRLIB_FreeTexture(GFX_Platform_Green);
	GRRLIB_FreeTexture(GFX_Platform_Blue);
	GRRLIB_FreeTexture(GFX_Platform_Brown);
	GRRLIB_FreeTexture(GFX_Platform_White);
	GRRLIB_FreeTexture(GFX_Platform_Spring);
	GRRLIB_FreeTexture(GFX_Platform_Gold);
	GRRLIB_FreeTexture(GFX_Platform_BlueH);
	
	//Obstacles
	GRRLIB_FreeTexture(GFX_Obstacle_BlackHole);
	
	//Fonts
	GRRLIB_FreeTexture(FONT_Doodle);
	GRRLIB_FreeTexture(FONT_Doodle_Bold);
}