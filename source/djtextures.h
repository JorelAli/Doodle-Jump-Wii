#include <grrlib.h>

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

#ifndef DOODLEJUMP_TEXTURES
#define DOODLEJUMP_TEXTURES
/* ^^ these are the include guards */

/* Variables */

//Background
extern GRRLIB_texImg *GFX_Background;
extern GRRLIB_texImg *GFX_Bar;

//Players
extern GRRLIB_texImg *GFX_Player_Left;
extern GRRLIB_texImg *GFX_Player_Right;
extern GRRLIB_texImg *GFX_Player_Left2;
extern GRRLIB_texImg *GFX_Player_Right2;

//Platforms
extern GRRLIB_texImg *GFX_Platform_Green;
extern GRRLIB_texImg *GFX_Platform_Blue;
extern GRRLIB_texImg *GFX_Platform_Brown;
extern GRRLIB_texImg *GFX_Platform_White;
extern GRRLIB_texImg *GFX_Platform_Spring;
extern GRRLIB_texImg *GFX_Platform_Gold;
extern GRRLIB_texImg *GFX_Platform_BlueH;

//Obstacles
extern GRRLIB_texImg *GFX_Obstacle_BlackHole;

//Fonts
extern GRRLIB_texImg *FONT_Doodle;
extern GRRLIB_texImg *FONT_Doodle_Bold;

/* Prototypes for the functions */

void TEXTURES_Init();
void TEXTURES_Exit();

#endif