/*---------------------------------------------------------------------------------

	Doodlejump
	
---------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <ogc/tpl.h>

#include <grrlib.h>

#include "gfx/doodleL.h"
#include "gfx/doodleR.h"
#include "gfx/background.h"
#include "gfx/pgreen.h"
#include "gfx/pblue.h"
#include "gfx/Arial_18.h"

#include <asndlib.h>
#include <mp3player.h>

//#include "doodle_tpl.h"
//#include "doodle.h"
//#include "mystery_mp3.h"
#include "fall_mp3.h"
#include "jump_mp3.h"
 
#define DEFAULT_FIFO_SIZE	(256*1024)

//Game constants
#define PLAYER_X_AXIS_SPEED 	6	//How quickly the character can go left/right by tilting
#define GRAVITY_CONSTANT		1	//How fast gravity is
#define NUM_PLATFORMS			10	//Number of platforms (TODO: Remove)
#define PLATFORM_JUMP_CONSTANT	5	//The amount of "bounce" a platform has //TODO: Lower this to accomodate for platforms moving downwards?
#define LINE_OF_MOVEMENT		140	//An invisible line, when crossed (above), it moves platforms downwards,
									//creating the illusion of travelling upwards
#define PLATFORM_MOVE_SPEED		1	//How quickly moving platforms (blue) move
#define PLATFORM_MOVE_DISTANCE	200	//How far a moving platform moves


//STRUCTURE DECLARATION -----------------------------------------------------------
//Player object
typedef struct {
	int x,y;			// screen co-ordinates 
	int dx, dy;			// velocity
	int direction; 		//direction: 0 = left, 1 = right
	int score;			// score of how high they've jumped
}Player;

//Platform object
typedef struct {
	int x,y;
	int moves;			//whether this is a moving platform: 0 = normal, 1 = moving	
	int dx;				//Used for moving platforms (unused for green platforms)
	int direction;		//Used for determining the direction of a moving platform: 0 = right, 1 = left
}Platform;
//---------------------------------------------------------------------------------

Player player;
Platform platformArr[NUM_PLATFORMS];

int cheats = 0;			//Number of times the player has pressed A or 2

int paused = 0; // 0 = good, 1 = paused

//METHOD DECLARATION ---------------------------------------------------------------
void drawDoodleJumper(int x, int y, int direction);
void drawPlatform(int x, int y, int moves);
int collidesWithPlatformFromAbove();
void drawBackground();
void drawPaused();
void printScore();
//---------------------------------------------------------------------------------

//Global textures for method access
GRRLIB_texImg *GFX_Background;
GRRLIB_texImg *GFX_Player_Left;
GRRLIB_texImg *GFX_Player_Right;
GRRLIB_texImg *GFX_Platform_Green;
GRRLIB_texImg *GFX_Platform_Blue;

GRRLIB_texImg *tex_BMfont4;


//---------------------------------------------------------------------------------
int main(int argc, char **argv){
//---------------------------------------------------------------------------------
	
	// Initialise the audio subsystem
	ASND_Init(NULL);
	MP3Player_Init();

	//Init GRRLIB
	GRRLIB_Init();
	
	//Load textures
	GFX_Background = GRRLIB_LoadTexture(background);
	GFX_Player_Left = GRRLIB_LoadTexture(doodleL);
	GFX_Player_Right = GRRLIB_LoadTexture(doodleR);
	GFX_Platform_Green = GRRLIB_LoadTexture(pgreen);
	GFX_Platform_Blue = GRRLIB_LoadTexture(pblue);
	
	tex_BMfont4 = GRRLIB_LoadTexture(Arial_18);
	GRRLIB_InitTileSet(tex_BMfont4, 24, 27, 32);

	//Initialise controllers
	WPAD_Init();
	
	//Allow access to gforce
	WPAD_SetDataFormat(WPAD_CHAN_0,WPAD_FMT_BTNS_ACC_IR);
	WPAD_SetVRes(0,640,480);

	srand(time(NULL));
	
	//Init player
	player.x = 320;	//center location
	player.y = 240;	//center
	player.dx = -1 * PLAYER_X_AXIS_SPEED; //DO NOT CHANGE THIS VALUE!!!
	player.dy = 0;// * GRAVITY_CONSTANT;
	player.direction = 0;
	player.score = 0;
	
	//Wii remote information
	WPAD_ScanPads();
	
	gforce_t gforce; //wiimote acceleration
	WPAD_GForce(0, &gforce); //get acceleration
	
	//Play music!
	//MP3Player_PlayBuffer(mystery_mp3, mystery_mp3_size, NULL);
	
	//Generate platforms all over the place
	int i;
	for(i = 1; i < NUM_PLATFORMS; i++) {
		platformArr[i].x = rand() % (640 - 64);  //This value takes into account the size of the platform
		platformArr[i].y = rand() % (480 - 16);	//TODO: Lower this value relative to other platforms! (So there aren't any "impossible" jumps)
														//Try y value of less than 11 << 8?
		platformArr[i].moves = rand() % 2;			//half are moving platforms (random number between 0 and 1)
		platformArr[i].dx = 0;
		platformArr[i].direction = 0;
	}
	
	//Generate a platform under the player
	platformArr[0].x = player.x;
	platformArr[0].y = player.y + 65;
	
	while(1) {

		//Get latest data from wiimote
		WPAD_ScanPads();

		//If home button, exit
		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME) {
			//Free up texture memory
			GRRLIB_FreeTexture(GFX_Background);
			GRRLIB_FreeTexture(GFX_Player_Left);
			GRRLIB_FreeTexture(GFX_Player_Right);
			GRRLIB_FreeTexture(GFX_Platform_Green);
			GRRLIB_FreeTexture(GFX_Platform_Blue);
			
			//Exit GRRLib
			GRRLIB_Exit();
			exit(0);
		}
		
		//Update acceleration
		WPAD_GForce(0, &gforce); 

		//Pressing A will put the player at the top of the screen (for testing purposes)
		if ( WPAD_ButtonsDown(0) & WPAD_BUTTON_A ){
			player.y = 10;		
			player.dy = 0;
			cheats = cheats + 1;
		}
		
		//Pressing 2 will simulate a player jump (for testing purposes)
		if ( WPAD_ButtonsDown(0) & WPAD_BUTTON_2 ){
			player.dy = -(PLATFORM_JUMP_CONSTANT);
			cheats = cheats + 1;
		}
		
		//Pause the game
		if ( WPAD_ButtonsDown(0) & WPAD_BUTTON_PLUS	){
			if(paused == 0) {
				paused = 1;
			} else if(paused == 1) {
				paused = 0;
			}
		}
		
		if(paused == 0) {
		
			//Player movement
			player.x += (int) (player.dx * gforce.y);		//gforce.y is the left/right tilt of wiimote when horizontal (2 button to the right)
			player.y += player.dy;
			
			player.dy += GRAVITY_CONSTANT;			//gravity?
			
			//player direction changes when going left/right
			if(gforce.y <= 0) {
				player.direction = 1;
			} else {
				player.direction = 0;
			}	
			
			//Makes the player loop if they go left/right off the screen
			if(player.x < 1) 
				player.x = 640-32;
			
			if(player.x > (640-32)) 
				player.x = 1;

			//Player touches the top of the screen
			if(player.y < 1) {
				player.dy = 0;
				player.y = 10;
			}  
			
			//Player touches the bottom of the screen
			if(player.y > (480-32)) {
				//player.dy = 0;
				//player.y = 10 << 8; //TEMPORARY				//TODO: game over 
				MP3Player_PlayBuffer(fall_mp3, fall_mp3_size, NULL);
				
				//Reset player
				player.x = 320;	//center location
				player.y = 240;	//center
				
				player.dy = 0;
				player.score = 0;
				cheats = 0;
				
				//Regenerate all platforms
				for(i = 1; i < NUM_PLATFORMS; i++) {
					platformArr[i].x = rand() % (640 - 64); //This value takes into account the size of the platform
					platformArr[i].y = rand() % (480 - 16);
				}
				
				//Generate a platform under the player
				platformArr[0].x = player.x;
				platformArr[0].y = player.y + 65;
			}
			
			//Player lands on a platform
			if(collidesWithPlatformFromAbove() == 1) {
				//Reset gravity, giving an upthrust of 
				player.dy = -(PLATFORM_JUMP_CONSTANT); //2 is our constant here - 2 << 8 = 512
				//MP3Player_PlayBuffer(jump_mp3, jump_mp3_size, NULL); //Jump sound doesn't always activate... why?
				MP3Player_PlayBuffer(jump_mp3, jump_mp3_size, NULL); //Jump sound doesn't always activate... why?
			}
			
			//Move platforms when the player is above the line of movement and the player is NOT falling
			//if(player.y < ((LINE_OF_MOVEMENT)) && player.dy < 0) { 
			//	player.score = player.score + 1;
			//	for(i = 0; i < NUM_PLATFORMS; i++) {
			//		platformArr[i].y = platformArr[i].y + (PLATFORM_JUMP_CONSTANT); //From the gravity code above
			//		
			//		//If the platform is off of the screen
			//		if(platformArr[i].y > (480)) {
			//			//Generate a new random platform
			//			platformArr[i].x = rand() % (640 - 64); //This value takes into account the size of the platform
			//			platformArr[i].y = rand() % (480 - 16);
			//		}
			//	}
			//}
		
		} 
		
		//rendering stuff goes here
						
		//Background
		drawBackground();
		
		//Drawing of platforms and player
		drawDoodleJumper( player.x, player.y, player.direction);
		
		//Drawing of platforms
		for(i = 0; i < NUM_PLATFORMS; i++) {
			if(platformArr[i].moves == 1) {
			
				//Changes direction value of platform
				if(platformArr[i].direction == 0) { //If it's going right
					
					if(platformArr[i].dx > PLATFORM_MOVE_DISTANCE) { //If it's gone as far as it can go
						platformArr[i].direction = 1; //Switch direction
					} else {
						platformArr[i].dx = platformArr[i].dx + PLATFORM_MOVE_SPEED;	//else, move it
					}
					
				} else if(platformArr[i].direction == 1) {	//Otherwise, if it's going left
					if(platformArr[i].dx < 0) {
						platformArr[i].direction = 0; //Switch direction
					} else {
						platformArr[i].dx = platformArr[i].dx - PLATFORM_MOVE_SPEED;
					}
				}
				
				drawPlatform(platformArr[i].x + platformArr[i].dx, platformArr[i].y, platformArr[i].moves);
			} else {
				drawPlatform(platformArr[i].x, platformArr[i].y, platformArr[i].moves);
			}
		}
		
		if(paused == 1) {
			GRRLIB_Printf(256, 208, tex_BMfont4, 0x800080FF, 1, "PAUSED");
			//drawPaused();
		}
		
		GRRLIB_Printf(5, 5, tex_BMfont4, 0x800080FF, 1, "cheats: %d", cheats);
		GRRLIB_Printf(5, 30, tex_BMfont4, 0x800080FF, 1, "gforce %f", gforce.y);
		
		GRRLIB_Render();  // Render the frame buffer to the TV	
		
	}
	return 0;
}

//---------------------------------------------------------------------------------
void printScore() {
	printf("\x1b[2;0H");
	if(cheats == 0) {
		printf("Score: %d", player.score);
	} else if(cheats) {
		printf("Score: %d Cheats used: %d", player.score, cheats);
	}
}
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
void drawDoodleJumper( int x, int y, int direction) {
//---------------------------------------------------------------------------------

	if(direction == 0) {
		GRRLIB_DrawImg(x, y, GFX_Player_Left, 0, 1, 1, RGBA(255, 255, 255, 255));
	} else if(direction == 1) {
		GRRLIB_DrawImg(x, y, GFX_Player_Right, 0, 1, 1, RGBA(255, 255, 255, 255));
	}

}

//---------------------------------------------------------------------------------
void drawPlatform(int x, int y, int moves) {
//---------------------------------------------------------------------------------

	if(moves == 0) {
		GRRLIB_DrawImg(x, y, GFX_Platform_Green, 0, 1, 1, RGBA(255, 255, 255, 255));
	} else if(moves == 1) {
		GRRLIB_DrawImg(x, y, GFX_Platform_Blue, 0, 1, 1, RGBA(255, 255, 255, 255));
	}
	
}

//---------------------------------------------------------------------------------
void drawBackground() {
//---------------------------------------------------------------------------------
	GRRLIB_DrawImg(0, 0, GFX_Background, 0, 1, 1, RGBA(255, 255, 255, 255));
}

//---------------------------------------------------------------------------------
void drawPaused() {
//---------------------------------------------------------------------------------

}

//---------------------------------------------------------------------------------
int collidesWithPlatformFromAbove() {
//---------------------------------------------------------------------------------
	int j;
	for(j = 0; j < NUM_PLATFORMS; j++) {
		int px = (player.x + 32); //Center x-coordinate of the player
		
		if(platformArr[j].moves == 1) {
		
			int platX = platformArr[j].x + (platformArr[j].dx); //Moving platform dx to determine dynamic location of platform
		
			if(px > platX && px < (platX + (64))) { //TODO take into account platforms which move
				
				int py = player.y + (64); //The foot of the character
				
				if(py <= (platformArr[j].y + (16))) {
					if(py >= (platformArr[j].y)) {
						if(player.dy > 0) //The player is falling
							return 1;	
					}
				}
			}
		} else {
			if(px > platformArr[j].x && px < (platformArr[j].x + (64))) { //TODO take into account platforms which move
				
				int py = player.y + (64); //The foot of the character
				
				if(py <= (platformArr[j].y + (16))) {
					if(py >= (platformArr[j].y)) {
						if(player.dy > 0) //The player is falling
							return 1;	
					}
				}
			}
		}
	}
	
	return 0;
}

