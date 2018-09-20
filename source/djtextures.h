#ifndef __DOODLEJUMP_TEXTURES__
#define __DOODLEJUMP_TEXTURES__
/* ^^ these are the include guards */

#include <grrlib.h>

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

//Menu buttons
extern GRRLIB_texImg *GFX_Singleplayer_Button;
extern GRRLIB_texImg *GFX_Coop_Button;
extern GRRLIB_texImg *GFX_Competitive_Button;
extern GRRLIB_texImg *GFX_Selected_Button;

extern GRRLIB_texImg *GFX_Quit_Button;
extern GRRLIB_texImg *GFX_Resume_Button;
/* Prototypes for the functions */

void TEXTURES_Init();
void TEXTURES_Exit();

#endif