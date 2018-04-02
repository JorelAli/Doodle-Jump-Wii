/*---------------------------------------------------------------------------------

	Doodlejump
	
	TODO: http://wiibrew.org/wiki/GRRLIB

---------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <ogc/tpl.h>

#include <asndlib.h>
#include <mp3player.h>

#include "doodle_tpl.h"
#include "doodle.h"
#include "mystery_mp3.h"
 
#define DEFAULT_FIFO_SIZE	(256*1024)

//Game speed constants
#define PLAYER_X_AXIS_SPEED 	2
//#define GRAVITY_CONSTANT		1
#define NUM_PLATFORMS			5

static void *frameBuffer[2] = { NULL, NULL};

static GXRModeObj *rmode;

//simple sprite struct
typedef struct {
	int x,y;			// screen co-ordinates 
	int dx, dy;			// velocity
	int direction; 		//direction: 0 = left, 1 = right
}Player;

//Platform object
typedef struct {
	int x,y;
}Platform;

Player player;
Platform platformArr[NUM_PLATFORMS];

GXTexObj texObj;

void drawDoodleJumper(int x, int y, int direction);
void drawPlatform(int x, int y);

//---------------------------------------------------------------------------------
int main( int argc, char **argv ){
//---------------------------------------------------------------------------------
	u32	fb; 	// initial framebuffer index
	u32 first_frame;
	f32 yscale;
	u32 xfbHeight;
	Mtx44 perspective;
	Mtx GXmodelView2D;
	void *gp_fifo = NULL;
	
	//Background colour :)
	GXColor background = {0, 255, 128, 0};

	// Initialise the audio subsystem
	ASND_Init(NULL);
	MP3Player_Init();

	//Initialise video system
	VIDEO_Init();	
 
	rmode = VIDEO_GetPreferredMode(NULL);
	
	fb = 0;
	first_frame = 1;
	// allocate 2 framebuffers for double buffering
	frameBuffer[0] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));
	frameBuffer[1] = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

	VIDEO_Configure(rmode);
	VIDEO_SetNextFramebuffer(frameBuffer[fb]);
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(rmode->viTVMode&VI_NON_INTERLACE) VIDEO_WaitVSync();

	fb ^= 1;

	// setup the fifo and then init the flipper
	gp_fifo = memalign(32,DEFAULT_FIFO_SIZE);
	memset(gp_fifo,0,DEFAULT_FIFO_SIZE);
 
	GX_Init(gp_fifo,DEFAULT_FIFO_SIZE);
 
	// clears the bg to color and clears the z buffer
	GX_SetCopyClear(background, 0x00ffffff);
 
	// other gx setup
	GX_SetViewport(0,0,rmode->fbWidth,rmode->efbHeight,0,1);
	yscale = GX_GetYScaleFactor(rmode->efbHeight,rmode->xfbHeight);
	xfbHeight = GX_SetDispCopyYScale(yscale);
	GX_SetScissor(0,0,rmode->fbWidth,rmode->efbHeight);
	GX_SetDispCopySrc(0,0,rmode->fbWidth,rmode->efbHeight);
	GX_SetDispCopyDst(rmode->fbWidth,xfbHeight);
	GX_SetCopyFilter(rmode->aa,rmode->sample_pattern,GX_TRUE,rmode->vfilter);
	GX_SetFieldMode(rmode->field_rendering,((rmode->viHeight==2*rmode->xfbHeight)?GX_ENABLE:GX_DISABLE));

	if (rmode->aa)
		GX_SetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR);
	else
		GX_SetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR);


	GX_SetCullMode(GX_CULL_NONE);
	GX_CopyDisp(frameBuffer[fb],GX_TRUE);
	GX_SetDispCopyGamma(GX_GM_1_0);

	// setup the vertex descriptor
	// tells the flipper to expect direct data
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	

	GX_SetNumChans(1);
	GX_SetNumTexGens(1);
	GX_SetTevOp(GX_TEVSTAGE0, GX_REPLACE);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);


	GX_InvalidateTexAll();

	TPLFile spriteTPL;
	TPL_OpenTPLFromMemory(&spriteTPL, (void *)doodle_tpl,doodle_tpl_size);
	TPL_GetTexture(&spriteTPL,doodlepic,&texObj);
	GX_LoadTexObj(&texObj, GX_TEXMAP0);

	guOrtho(perspective,0,479,0,639,0,300);
	GX_LoadProjectionMtx(perspective, GX_ORTHOGRAPHIC);

	//Initialise controllers
	WPAD_Init();
	//Allow access to gforce
	WPAD_SetDataFormat(0,WPAD_FMT_BTNS_ACC_IR);

	srand(time(NULL));
	
	//Init player
	player.x = 320 << 8;	//center location
	player.y = 240 << 8;	//center
	player.dx = -256 * PLAYER_X_AXIS_SPEED;
	player.dy = 256;// * GRAVITY_CONSTANT;
	player.direction = 0;
	
	//Wii remote information
	WPAD_ScanPads();
	
	gforce_t gforce; //wiimote acceleration
	WPAD_GForce(0, &gforce); //get acceleration
	
	//Play music!
	MP3Player_PlayBuffer(mystery_mp3, mystery_mp3_size, NULL);
	
	//Generate random platforms <<-- THIS CRASHES AT THE MOMENT!!! - GX not initialised -.-
	//int i;
	//for(i = 0; i < 5; i++) {
	//	drawPlatform(rand() * 640, rand() * 480);
	//}
	
	int i;
	for(i = 0; i < NUM_PLATFORMS; i++) {
		platformArr[i].x = rand() % (640 - 32) << 8; //This value takes into account the size of the platform
		platformArr[i].y = rand() % (480 - 32) << 8;
	}
	
	while(1) {

		//Get latest data from wiimote
		WPAD_ScanPads();

		//If home button, exit
		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME) exit(0);
		
		//Update acceleration
		WPAD_GForce(0, &gforce); 
		
		//GX Setup
		GX_SetViewport(0,0,rmode->fbWidth,rmode->efbHeight,0,1);
		GX_InvVtxCache();
		GX_InvalidateTexAll();

		GX_ClearVtxDesc();
		GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
		GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);

		guMtxIdentity(GXmodelView2D);
		guMtxTransApply (GXmodelView2D, GXmodelView2D, 0.0F, 0.0F, -5.0F);
		GX_LoadPosMtxImm(GXmodelView2D,GX_PNMTX0);
		
		//Player movement
		player.x += (int) (player.dx * gforce.y);		//gforce.y is the left/right tilt of wiimote when horizontal (2 button to the right)
		player.y += player.dy;
		
		//player.dy += 128 * GRAVITY_CONSTANT;			//gravity?
		
		//player direction changes when going left/right
		if(gforce.y <= 0) {
			player.direction = 1;
		} else {
			player.direction = 0;
		}	
		if(player.x < (1<<8)) 
			player.x = ((640-32) << 8);
		
		if(player.x > ((640-32) << 8)) 
			player.x = (1<<8);

		//TODO gravity
		if(player.y < (1<<8) || player.y > ((480-32) << 8))
			player.dy = -player.dy;
		
		drawDoodleJumper( player.x >> 8, player.y >> 8, player.direction);
		
		for(i = 0; i < NUM_PLATFORMS; i++) {
			drawPlatform(platformArr[i].x >> 8, platformArr[i].y >> 8);
		}
		
		
		

		//Finish drawing - clean up :)
		GX_DrawDone();
		
		GX_SetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
		GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
		GX_SetAlphaUpdate(GX_TRUE);
		GX_SetColorUpdate(GX_TRUE);
		GX_CopyDisp(frameBuffer[fb],GX_TRUE);
		
		VIDEO_SetNextFramebuffer(frameBuffer[fb]);
		if(first_frame) {
			VIDEO_SetBlack(FALSE);
			first_frame = 0;
		}
		
		VIDEO_Flush();
		VIDEO_WaitVSync();
		fb ^= 1;		// flip framebuffer
	}
	return 0;
}


//---------------------------------------------------------------------------------
void drawDoodleJumper( int x, int y, int direction) {
//---------------------------------------------------------------------------------

	//Dimensions for the player
	int width = 64;
	int height = 64;

	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);			// Draw A Quad
	
	if(direction == 0) { //left facing doodler
	
		GX_Position2f32(x, y);					// Top Left
		GX_TexCoord2f32(0.0,0.0);
		
		GX_Position2f32(x+width-1, y);			// Top Right
		GX_TexCoord2f32(0.5,0.0);
		
		GX_Position2f32(x+width-1,y+height-1);	// Bottom Right
		GX_TexCoord2f32(0.5,0.5);
		
		GX_Position2f32(x,y+height-1);			// Bottom Left
		GX_TexCoord2f32(0.0,0.5);
	
	} else { //right facing doodler
	
		GX_Position2f32(x, y);					// Top Left
		GX_TexCoord2f32(0.5,0.0);
		
		GX_Position2f32(x+width-1, y);			// Top Right
		GX_TexCoord2f32(1.0,0.0);
		
		GX_Position2f32(x+width-1,y+height-1);	// Bottom Right
		GX_TexCoord2f32(1.0,0.5);
		
		GX_Position2f32(x,y+height-1);			// Bottom Left
		GX_TexCoord2f32(0.5,0.5);
	
	}
	
	GX_End();									// Done Drawing The Quad 

}

//---------------------------------------------------------------------------------
void drawPlatform(int x, int y) {
//---------------------------------------------------------------------------------
	
	//x = x - 32; //Center constant - By having this, we provide a value for x for the center of the platform
	//y = y - 8; 	//Center constant
	
	//Dimensions for the player
	int width = 64;
	int height = 16;

	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);			// Draw A Quad
	
		GX_Position2f32(x, y);					// Top Left
		GX_TexCoord2f32(0.0,0.5);
		
		GX_Position2f32(x+width-1, y);			// Top Right
		GX_TexCoord2f32(0.5,0.5);
		
		GX_Position2f32(x+width-1,y+height-1);	// Bottom Right
		GX_TexCoord2f32(0.5,0.625);
		
		GX_Position2f32(x,y+height-1);			// Bottom Left
		GX_TexCoord2f32(0.0,0.625);

	GX_End();									// Done Drawing The Quad 

}

