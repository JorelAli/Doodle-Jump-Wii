/*---------------------------------------------------------------------------------

	Doodlejump - Written by Jorel Ali
	
---------------------------------------------------------------------------------*/

//Standard libs
#include <stdlib.h>

//devkitPPC libs
#include <wiiuse/wpad.h>

//Graphics lib
#include <grrlib.h>

//Graphic files
#include "gfx/doodleL.h"
#include "gfx/doodleR.h"
#include "gfx/background.h"
#include "gfx/pgreen.h"
#include "gfx/pblue.h"
#include "gfx/pbrown.h" //TEMPORARY - TODO: USE TILES
#include "gfx/Arial_18.h"
#include "gfx/Al_seana_14.h"
#include "gfx/Al_seana_16_Bold.h"

//Music libs
#include <asndlib.h>
#include <mp3player.h>

//Sound files
#include "fall_mp3.h"
#include "jump_mp3.h"
 
//Game Constants ------------------------------------------------------------------
#define PLAYER_X_AXIS_SPEED 	6	//How quickly the character can go left/right by tilting
#define GRAVITY_CONSTANT		1	//How fast gravity is
#define NUM_PLATFORMS			10	//Number of platforms (TODO: Remove)
#define PLATFORM_JUMP_CONSTANT	5	//The amount of "bounce" a platform has
#define LINE_OF_MOVEMENT		140	//An invisible line, when crossed (above), it moves platforms downwards,
									//creating the illusion of travelling upwards
#define PLATFORM_MOVE_SPEED		1	//How quickly moving platforms (blue) move
#define PLATFORM_MOVE_DISTANCE	200	//How far a moving platform moves
#define GAME_TICK_SPEED			8	//How quickly the game runs (default is 8)

#define PLAYER_JUMP_HEIGHT		100	//A rough indication of how high a player can jump (this idea is not 100% confirmed)
//---------------------------------------------------------------------------------

//ENUM DECLARATION ----------------------------------------------------------------
typedef enum {NORMAL, MOVING, BREAKING} PlatformType;
//---------------------------------------------------------------------------------

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
	PlatformType type;
	int dx;				//Used for moving platforms (blue)
	int direction;		//Used for determining the direction of a moving platform: 0 = right, 1 = left
}Platform;
//---------------------------------------------------------------------------------

Player player;			//Global play object
Platform platformArr[NUM_PLATFORMS];

int cheats = 0;			//Number of times the player has pressed A or 2
int paused = 0; 		// 0 = playing, 1 = paused

//METHOD DECLARATION --------------------------------------------------------------
void drawDoodleJumper(int x, int y, int direction);		//Draws the player
void drawPlatform(int x, int y, PlatformType type);		//Draws a platform
int collidesWithPlatformFromAbove();					//Checks if the player bounces on a platform
void drawBackground();									//Draws the background
void drawPaused();										//Draws the pause screen
void createPlatform(int index);							//Creates a platform at index for platformArr[] 
void drawAllPlatforms();								//Draws all of the platforms from platformArr[]
//---------------------------------------------------------------------------------

//Global textures for method access -----------------------------------------------

//Textures
GRRLIB_texImg *GFX_Background;
GRRLIB_texImg *GFX_Player_Left;
GRRLIB_texImg *GFX_Player_Right;
GRRLIB_texImg *GFX_Platform_Green;
GRRLIB_texImg *GFX_Platform_Blue;
GRRLIB_texImg *GFX_Platform_Brown;	//TODO: USE TILES

//Fonts
GRRLIB_texImg *doodlefont;
GRRLIB_texImg *doodlefont_bold;

//---------------------------------------------------------------------------------

// RGBA Colors --------------------------------------------------------------------
#define GRRLIB_DOODLE  0xAA1F23FF //Reddish doodlejump colour
#define GRRLIB_BLACK   0x000000FF
#define GRRLIB_MAROON  0x800000FF
#define GRRLIB_GREEN   0x008000FF
#define GRRLIB_OLIVE   0x808000FF
#define GRRLIB_NAVY    0x000080FF
#define GRRLIB_PURPLE  0x800080FF
#define GRRLIB_TEAL    0x008080FF
#define GRRLIB_GRAY    0x808080FF
#define GRRLIB_SILVER  0xC0C0C0FF
#define GRRLIB_RED     0xFF0000FF
#define GRRLIB_LIME    0x00FF00FF
#define GRRLIB_YELLOW  0xFFFF00FF
#define GRRLIB_BLUE    0x0000FFFF
#define GRRLIB_FUCHSIA 0xFF00FFFF
#define GRRLIB_AQUA    0x00FFFFFF
#define GRRLIB_WHITE   0xFFFFFFFF
//---------------------------------------------------------------------------------


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
	GFX_Platform_Brown = GRRLIB_LoadTexture(pbrown);
	
	//Load fonts
	doodlefont = GRRLIB_LoadTexture(Al_seana_14);
	GRRLIB_InitTileSet(doodlefont, 14, 22, 32);
	
	doodlefont_bold = GRRLIB_LoadTexture(Al_seana_16_Bold);
	GRRLIB_InitTileSet(doodlefont_bold, 17, 24, 32);

	//Initialise controllers
	WPAD_Init();
	
	//Allow access to gforce (acceleration)
	WPAD_SetDataFormat(WPAD_CHAN_0,WPAD_FMT_BTNS_ACC_IR);

	//Setup random generator (this chooses a random seed for rand() generation)
	srand(time(NULL));
	
	//Init player 
	player.x = 320;	//center location
	player.y = 240;	//center
	player.dx = -1 * PLAYER_X_AXIS_SPEED; //DO NOT CHANGE THIS VALUE!!!
	player.dy = 0;
	player.direction = 0;
	player.score = 0;
	
	//Wii remote information
	WPAD_ScanPads();
	
	gforce_t gforce; //wiimote acceleration
	WPAD_GForce(0, &gforce); //get acceleration
	
	//Generate a platform under the player
	platformArr[0].x = player.x;
	platformArr[0].y = player.y + 65;
	
	//Generate platforms all over the place
	int i;
	for(i = 1; i < NUM_PLATFORMS; i++) {
		createPlatform(i);
	}
	
	//Game tick speed counter
	int gameTick = 0;
	
	//Main game loop
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
			
			GRRLIB_FreeTexture(doodlefont);
			GRRLIB_FreeTexture(doodlefont_bold);
			
			//Exit GRRLib
			GRRLIB_Exit();
			exit(0);
		}
		
		//Manage game tick speed looping
		gameTick = gameTick + 1;
		if(gameTick > GAME_TICK_SPEED) {
			gameTick = 0;
		}
		
		//Update acceleration
		WPAD_GForce(0, &gforce); 

		//Pressing A will put the player at the top of the screen (for testing purposes)
		if (WPAD_ButtonsHeld(0) & WPAD_BUTTON_A){		
			player.dy = 0;
		}
		
		//Pressing 2 will simulate a player jump (for testing purposes)
		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_2 ){
			player.dy = -(PLATFORM_JUMP_CONSTANT);
			cheats++;
		}
		
		//Pause the game
		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_PLUS){
			paused ^= 1;
		}
		
		int rY = player.y;
		
		//If not paused
		if(paused == 0) {
		
			if(gameTick == 0) {								//Only update gravity on the gametick (makes it smooth and easy to control) 
				player.dy += GRAVITY_CONSTANT;
			}
			
			//Player lands on a platform
			if(collidesWithPlatformFromAbove()) {
				//Jump
				player.dy = -(PLATFORM_JUMP_CONSTANT);
				
				MP3Player_PlayBuffer(jump_mp3, jump_mp3_size, NULL); 
			}
		
			//Player movement
			player.x += (int) (player.dx * gforce.y);		//gforce.y is the left/right tilt of wiimote when horizontal (2 button to the right)
			player.y += player.dy;		
			
			
			
			//Move platforms when the player is above the line of movement and the player is NOT falling
			if(player.y <= ((LINE_OF_MOVEMENT)) && player.dy <= 0) { 
				rY = LINE_OF_MOVEMENT; //TODO: Just set dy = 0 using a rdY variable - this prevents gravity, therefore y never changes, but dy will (because rdY)
				player.y += PLATFORM_JUMP_CONSTANT;
				player.score++;
				
				for(i = 0; i < NUM_PLATFORMS; i++) {
					platformArr[i].y += (PLATFORM_JUMP_CONSTANT); //From the gravity code above
					
					//If the platform is off of the screen
					if(platformArr[i].y > (480)) {
						createPlatform(i);
					}
				}
			} else {
				rY = player.y;
			}
			
			
			//player direction changes when going left/right
			if(gforce.y <= 0) {
				player.direction = 1;
			} else {
				player.direction = 0;
			}	
			
			//Makes the player loop if they go left/right off the screen
			if(player.x < 1) 
				player.x = 640-64;
			
			if(player.x > (640-64)) 
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
					createPlatform(i);
				}
				
				//Generate a platform under the player
				platformArr[0].x = player.x;
				platformArr[0].y = player.y + 65;
			}
		} 
		
		//---------------------------------------------------------------------------------
		// VIDEO RENDERING
		//---------------------------------------------------------------------------------
						
		//Background
		drawBackground();
		
		//Drawing of platforms and player
		drawDoodleJumper( player.x, rY, player.direction);
		
		//Drawing of platforms
		drawAllPlatforms();
		
		//Draw paused screen
		if(paused) {
			drawPaused();
		}
		
		if(cheats == 0)
			GRRLIB_Printf(5, 5, doodlefont, GRRLIB_BLACK, 1, "Score: %d", player.score);
		else
			GRRLIB_Printf(5, 5, doodlefont, GRRLIB_BLACK, 1, "Score: %d (Cheats: %d)", player.score, cheats);
		
		GRRLIB_Line(0, LINE_OF_MOVEMENT, 640, LINE_OF_MOVEMENT, GRRLIB_BLACK);
		
		GRRLIB_Printf(5, 30, doodlefont_bold, GRRLIB_BLACK, 1, "dy: %d", player.dy);
		GRRLIB_Printf(5, 60, doodlefont_bold, GRRLIB_BLACK, 1, "c: (%d, %d)", player.x, player.y);
		GRRLIB_Printf(5, 90, doodlefont_bold, GRRLIB_BLACK, 1, "rY:      %d", rY);
		GRRLIB_Printf(5, 120, doodlefont_bold, GRRLIB_BLACK, 1, "gT: %d", gameTick);
		
		
		GRRLIB_Render();  // Render the frame buffer to the TV	
		
		//---------------------------------------------------------------------------------
		
		//Take a screenshot :)
		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_1){
			char buf[12];
			sprintf(buf, "sc%d.png", rand() % 20); //generate a random sc2.png file (for example)
			GRRLIB_ScrShot(buf);
		}
		
	}
	return 0;
}

//---------------------------------------------------------------------------------
void drawDoodleJumper( int x, int y, int direction) {
//---------------------------------------------------------------------------------

	if(direction)
		GRRLIB_DrawImg(x, y, GFX_Player_Right, 0, 1, 1, RGBA(255, 255, 255, 255));
	else
		GRRLIB_DrawImg(x, y, GFX_Player_Left, 0, 1, 1, RGBA(255, 255, 255, 255));

}

//---------------------------------------------------------------------------------
void drawAllPlatforms() {
//---------------------------------------------------------------------------------

	int i;
	for(i = 0; i < NUM_PLATFORMS; i++) {
		if(platformArr[i].type == MOVING) {
		
			if(paused == 0) {
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
			}
			
			drawPlatform(platformArr[i].x + platformArr[i].dx, platformArr[i].y, platformArr[i].type);
		} else {
			drawPlatform(platformArr[i].x, platformArr[i].y, platformArr[i].type);
		}
	}
	
}

//---------------------------------------------------------------------------------
void drawPlatform(int x, int y, PlatformType type) {
//---------------------------------------------------------------------------------

	switch(type) {
		case NORMAL:
			GRRLIB_DrawImg(x, y, GFX_Platform_Green, 0, 1, 1, RGBA(255, 255, 255, 255));
			break;
		case MOVING:
			GRRLIB_DrawImg(x, y, GFX_Platform_Blue, 0, 1, 1, RGBA(255, 255, 255, 255));
			break;
		case BREAKING:
			//TODO USE TILES.
			GRRLIB_DrawImg(x, y, GFX_Platform_Blue, 0, 1, 1, RGBA(255, 255, 255, 255));
			break;
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
	GRRLIB_Printf(266, 208, doodlefont_bold, GRRLIB_DOODLE, 1, "PAUSED");
	GRRLIB_Printf(200, 238, doodlefont_bold, GRRLIB_DOODLE, 1, "Press HOME to exit");
}

//---------------------------------------------------------------------------------
void createPlatform(int index) {
//---------------------------------------------------------------------------------

	//Scoring system:
	//Score > 1000:
	//	Moving platforms appear
	//Score > 2000:
	//	Brown platform appear
	
	platformArr[index].type = NORMAL;
	
	if(player.score > 1000) {
		// 1/2 probability
		if(rand() % 5 == 0) {
			platformArr[index].type = MOVING;
		}
	}
	
	if(player.score > 2000) {
		if(platformArr[index].type != MOVING) {
			// 1/5 probability OUT OF non-moving platforms
			if(rand() % 5 == 0) {
				platformArr[index].type = BREAKING;
			}
		}
	}
		
	if(platformArr[index].type == MOVING) {
		platformArr[index].x = rand() % (640 - 64 - PLATFORM_MOVE_DISTANCE);  //This value takes into account the size of the platform
	} else {
		platformArr[index].x = rand() % (640 - 64);  //This value takes into account the size of the platform
	}
	
	int minY = 480;
	
	int i;
	for(i = 0; i < NUM_PLATFORMS; i++) {
		//Ignore this index, we're writing to this index
		if(i == index) {
			continue;
		}
		//If platform is null, ignore it
		if(platformArr[i].y == 0) {
			continue;
		}
		//Get min value of y.
		if(platformArr[i].y < minY) {
			minY = platformArr[i].y;
		}
	}
	
	if(minY - PLAYER_JUMP_HEIGHT <= 0) {
		platformArr[index].y = rand() % (480 - 16);
	} else {
		//Can't have breaking platforms - this MUST be normal/moving
		if(platformArr[index].type == BREAKING) {
			platformArr[index].type = NORMAL;
		}
		platformArr[index].y = minY - PLAYER_JUMP_HEIGHT;
	}

	platformArr[index].dx = 0;
	platformArr[index].direction = 0;
}



//---------------------------------------------------------------------------------
int collidesWithPlatformFromAbove() {
//---------------------------------------------------------------------------------
	int j;
	for(j = 0; j < NUM_PLATFORMS; j++) {
		int px = (player.x + 32); //Center x-coordinate of the player
		
		switch(platformArr[j].type) {
			case NORMAL:
				if(px > platformArr[j].x && px < (platformArr[j].x + (64))) { //TODO take into account platforms which move
				
					int py = player.y + (64); //The foot of the character
					
					if(py <= (platformArr[j].y + (16))) {
						if(py >= (platformArr[j].y)) {
							if(player.dy >= 0) //The player is falling
								return 1;	
						}
					}
				}
				break;
			case MOVING:
				//(platformArr[j].x + platformArr[j].dx) is the location of the moving platform
				if(px > (platformArr[j].x + platformArr[j].dx) && px < ((platformArr[j].x + platformArr[j].dx) + 64)) { 
					
					int py = player.y + (64); //The foot of the character
					
					if(py <= (platformArr[j].y + (16))) {
						if(py >= (platformArr[j].y)) {
							if(player.dy >= 0) //The player is falling
								return 1;	
						}
					}
				}
				break;
			case BREAKING:
				//TODO USE TILES.
				//ACTIVATE BREAKING ANIMATION
				break;
		}
	}
	
	return 0;
}

