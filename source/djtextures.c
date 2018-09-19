#include "djtextures.h"

//Graphic files
//player
#include "gfx/doodleL.h"
#include "gfx/doodleR.h"

#include "gfx/doodleL2.h"
#include "gfx/doodleR2.h"

//backgrounds
#include "gfx/background.h"
#include "gfx/topbar.h"

//platforms
#include "gfx/pgreen.h"
#include "gfx/pblue.h"
#include "gfx/pbluevert.h" 
#include "gfx/pbrown_all.h" 
#include "gfx/pwhite.h" 
#include "gfx/pspring.h" 
#include "gfx/pgold.h" 

//obstacles
#include "gfx/blackhole.h" 

//fonts
#include "gfx/Arial_18.h"
#include "gfx/Al_seana_14.h"
#include "gfx/Al_seana_16_Bold.h"

//menu buttons
#include "gfx/singleplayerbtn.h"
#include "gfx/coopbtn.h"
#include "gfx/competitivebtn.h"
#include "gfx/selectedbtn.h"


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

//menu buttons
GRRLIB_texImg *GFX_Singleplayer_Button;
GRRLIB_texImg *GFX_Coop_Button;
GRRLIB_texImg *GFX_Competitive_Button;
GRRLIB_texImg *GFX_Selected_Button;

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
	
	//Menu
	GFX_Singleplayer_Button = GRRLIB_LoadTexture(singleplayerbtn);
	GFX_Coop_Button = GRRLIB_LoadTexture(coopbtn);
	GFX_Competitive_Button = GRRLIB_LoadTexture(competitivebtn);
	GFX_Selected_Button = GRRLIB_LoadTexture(selectedbtn);
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
	
	//Menu
	GRRLIB_FreeTexture(GFX_Singleplayer_Button);
	GRRLIB_FreeTexture(GFX_Coop_Button);
	GRRLIB_FreeTexture(GFX_Competitive_Button);
	GRRLIB_FreeTexture(GFX_Selected_Button);
	
}