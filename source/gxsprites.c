/*---------------------------------------------------------------------------------

	Doodlejump
	
	TODO: http://wiibrew.org/wiki/GRRLIB?

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

#include <string.h>
#include <malloc.h>
#include <math.h>
#include <ogcsys.h>

#include <asndlib.h>
#include <mp3player.h>

#include "doodle_tpl.h"
#include "doodle.h"
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


//---------------------------------------------------------------------------------
int main( int argc, char **argv ){
//---------------------------------------------------------------------------------
	

	// Initialise the audio subsystem
	ASND_Init(NULL);
	MP3Player_Init();

	GRRLIB_Init();

	//Initialise controllers
	WPAD_Init();
	//Allow access to gforce
	WPAD_SetDataFormat(0,WPAD_FMT_BTNS_ACC_IR);

	srand(time(NULL));
	
	//Init player
	player.x = 320 << 8;	//center location
	player.y = 240 << 8;	//center
	player.dx = -256 * PLAYER_X_AXIS_SPEED; //DO NOT CHANGE THIS VALUE!!!
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
		platformArr[i].x = rand() % (640 - 64) << 8;  //This value takes into account the size of the platform
		platformArr[i].y = rand() % (480 - 16) << 8;	//TODO: Lower this value relative to other platforms! (So there aren't any "impossible" jumps)
														//Try y value of less than 11 << 8?
		platformArr[i].moves = rand() % 2;			//half are moving platforms (random number between 0 and 1)
		platformArr[i].dx = 0;
		platformArr[i].direction = 0;
	}
	
	//Generate a platform under the player
	platformArr[0].x = player.x;
	platformArr[0].y = player.y + (65 << 8);
	
	while(1) {

		//Get latest data from wiimote
		WPAD_ScanPads();

		//If home button, exit
		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME) {
			GRRLIB_Exit();
			exit(0);
		}

		//Pressing A will put the player at the top of the screen (for testing purposes)
		if ( WPAD_ButtonsDown(0) & WPAD_BUTTON_A ){
			player.y = 10 << 8;		
			player.dy = 0;
			cheats = cheats + 1;
		}
		
		//Pressing 2 will simulate a player jump (for testing purposes)
		if ( WPAD_ButtonsDown(0) & WPAD_BUTTON_2 ){
			player.dy = -(PLATFORM_JUMP_CONSTANT << 8);
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
			
			player.dy += 32 * GRAVITY_CONSTANT;			//gravity?
			
			//player direction changes when going left/right
			if(gforce.y <= 0) {
				player.direction = 1;
			} else {
				player.direction = 0;
			}	
			
			//Makes the player loop if they go left/right off the screen
			if(player.x < (1<<8)) 
				player.x = ((640-32) << 8);
			
			if(player.x > ((640-32) << 8)) 
				player.x = (1<<8);

			//Player touches the top of the screen
			if(player.y < (1<<8)) {
				player.dy = 0;
				player.y = 10 << 8;
			}  
			
			//Player touches the bottom of the screen
			if(player.y > ((480-32) << 8)) {
				//player.dy = 0;
				//player.y = 10 << 8; //TEMPORARY				//TODO: game over 
				MP3Player_PlayBuffer(fall_mp3, fall_mp3_size, NULL);
				
				//Reset player
				player.x = 320 << 8;	//center location
				player.y = 240 << 8;	//center
				
				player.dy = 0;
				player.score = 0;
				cheats = 0;
				
				//Regenerate all platforms
				for(i = 1; i < NUM_PLATFORMS; i++) {
					platformArr[i].x = rand() % (640 - 64) << 8; //This value takes into account the size of the platform
					platformArr[i].y = rand() % (480 - 16) << 8;
				}
				
				//Generate a platform under the player
				platformArr[0].x = player.x;
				platformArr[0].y = player.y + (65 << 8);
			}
			
			//Player lands on a platform
			if(collidesWithPlatformFromAbove() == 1) {
				//Reset gravity, giving an upthrust of 
				player.dy = -(PLATFORM_JUMP_CONSTANT << 8); //2 is our constant here - 2 << 8 = 512
				//MP3Player_PlayBuffer(jump_mp3, jump_mp3_size, NULL); //Jump sound doesn't always activate... why?
				MP3Player_PlayBuffer(jump_mp3, jump_mp3_size, NULL); //Jump sound doesn't always activate... why?
			}
			
			//Move platforms when the player is above the line of movement and the player is NOT falling
			if(player.y < ((LINE_OF_MOVEMENT) << 8) && player.dy < 0) { 
				player.score = player.score + 1;
				for(i = 0; i < NUM_PLATFORMS; i++) {
					platformArr[i].y = platformArr[i].y + (PLATFORM_JUMP_CONSTANT << 8); //From the gravity code above
					
					//If the platform is off of the screen
					if(platformArr[i].y > (480 << 8)) {
						//Generate a new random platform
						platformArr[i].x = rand() % (640 - 64) << 8; //This value takes into account the size of the platform
						platformArr[i].y = rand() % (480 - 16) << 8;
					}
				}
			}
		
		} 
		
		//rendering stuff goes here
		
		GRRLIB_Render();  // Render the frame buffer to the TV	
		
		
				
		//Background
		drawBackground();
		
		//Drawing of platforms and player
		drawDoodleJumper( player.x >> 8, player.y >> 8, player.direction);
		
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
				
				drawPlatform(((platformArr[i].x + (platformArr[i].dx << 8)) >> 8), platformArr[i].y >> 8, platformArr[i].moves);
			} else {
				drawPlatform(platformArr[i].x >> 8, platformArr[i].y >> 8, platformArr[i].moves);
			}
		}
		
		if(paused == 1) {
			drawPaused();
		}
		
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

 
#define BOTTOM_ROW_CONST	0.8824f //0.88235f 


//---------------------------------------------------------------------------------
void drawDoodleJumper( int x, int y, int direction) {
//---------------------------------------------------------------------------------

	//Dimensions for the player
	int width = 64;
	int height = 64;

	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);			// Draw A Quad
	
	if(direction == 0) { //left facing doodler
	
		GX_Position2f32(x, y);					// Top Left
		GX_TexCoord2f32(0.0,BOTTOM_ROW_CONST);
		
		GX_Position2f32(x+width-1, y);			// Top Right
		GX_TexCoord2f32(0.1,BOTTOM_ROW_CONST);
		
		GX_Position2f32(x+width-1,y+height-1);	// Bottom Right
		GX_TexCoord2f32(0.1,1.0);
		
		GX_Position2f32(x,y+height-1);			// Bottom Left
		GX_TexCoord2f32(0.0,1.0);
	
	} else { //right facing doodler
	
		GX_Position2f32(x, y);					// Top Left
		GX_TexCoord2f32(0.1,BOTTOM_ROW_CONST);
		
		GX_Position2f32(x+width-1, y);			// Top Right
		GX_TexCoord2f32(0.2,BOTTOM_ROW_CONST);
		
		GX_Position2f32(x+width-1,y+height-1);	// Bottom Right
		GX_TexCoord2f32(0.2,1.0);
		
		GX_Position2f32(x,y+height-1);			// Bottom Left
		GX_TexCoord2f32(0.1,1.0);
	
	}
	
	GX_End();									// Done Drawing The Quad 

}

//---------------------------------------------------------------------------------
void drawPlatform(int x, int y, int moves) {
//---------------------------------------------------------------------------------
	
	//x = x - 32; //Center constant - By having this, we provide a value for x for the center of the platform
	//y = y - 8; 	//Center constant
	
	//Dimensions for the platform
	int width = 64;
	int height = 16;

	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);			// Draw A Quad
	
		if(moves == 0) {
			GX_Position2f32(x, y);					// Top Left
			GX_TexCoord2f32(0.2,BOTTOM_ROW_CONST);
			
			GX_Position2f32(x+width-1, y);			// Top Right
			GX_TexCoord2f32(0.3,BOTTOM_ROW_CONST);
			
			GX_Position2f32(x+width-1,y+height-1);	// Bottom Right
			GX_TexCoord2f32(0.3,0.91176);
			
			GX_Position2f32(x,y+height-1);			// Bottom Left
			GX_TexCoord2f32(0.2,0.91176);
		} else if(moves == 1) {
			GX_Position2f32(x, y);					// Top Left
			GX_TexCoord2f32(0.5,BOTTOM_ROW_CONST);
			
			GX_Position2f32(x+width-1, y);			// Top Right
			GX_TexCoord2f32(0.6,BOTTOM_ROW_CONST);
			
			GX_Position2f32(x+width-1,y+height-1);	// Bottom Right
			GX_TexCoord2f32(0.6,0.91176);
			
			GX_Position2f32(x,y+height-1);			// Bottom Left
			GX_TexCoord2f32(0.5,0.91176);
		}

	GX_End();									// Done Drawing The Quad 

}

//---------------------------------------------------------------------------------
void drawBackground() {
//---------------------------------------------------------------------------------
	
	int x = 0;
	int y = 0;
	
	//Dimensions for the background texture
	int width = 640;
	int height = 480;

	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);			// Draw A Quad
	
		GX_Position2f32(x, y);					// Top Left
		GX_TexCoord2f32(0.0,0.0);
		
		GX_Position2f32(x+width-1, y);			// Top Right
		GX_TexCoord2f32(1.0,0.0);
		
		GX_Position2f32(x+width-1,y+height-1);	// Bottom Right
		GX_TexCoord2f32(1.0,BOTTOM_ROW_CONST); //15/17
		
		GX_Position2f32(x,y+height-1);			// Bottom Left
		GX_TexCoord2f32(0.0,BOTTOM_ROW_CONST);

	GX_End();									// Done Drawing The Quad 

}

//---------------------------------------------------------------------------------
void drawPaused() {
//---------------------------------------------------------------------------------
	
	int x = 256;
	int y = 208;
	
	//Dimensions for the word "paused"
	int width = 128;
	int height = 64;

	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);			// Draw A Quad
	
		GX_Position2f32(x, y);					// Top Left
		GX_TexCoord2f32(0.3,BOTTOM_ROW_CONST);
		
		GX_Position2f32(x+width-1, y);			// Top Right
		GX_TexCoord2f32(0.5,BOTTOM_ROW_CONST);
		
		GX_Position2f32(x+width-1,y+height-1);	// Bottom Right
		GX_TexCoord2f32(0.5,1); //15/17
		
		GX_Position2f32(x,y+height-1);			// Bottom Left
		GX_TexCoord2f32(0.3,1);

	GX_End();									// Done Drawing The Quad 

}

//---------------------------------------------------------------------------------
int collidesWithPlatformFromAbove() {
//---------------------------------------------------------------------------------
	int j;
	for(j = 0; j < NUM_PLATFORMS; j++) {
		int px = (player.x + (32 << 8)); //Center x-coordinate of the player
		
		if(platformArr[j].moves == 1) {
		
			int platX = platformArr[j].x + (platformArr[j].dx << 8); //Moving platform dx to determine dynamic location of platform
		
			if(px > platX && px < (platX + (64 << 8))) { //TODO take into account platforms which move
				
				int py = player.y + (64 << 8); //The foot of the character
				
				if(py <= (platformArr[j].y + (16 << 8))) {
					if(py >= (platformArr[j].y)) {
						if(player.dy > 0) //The player is falling
							return 1;	
					}
				}
			}
		} else {
			if(px > platformArr[j].x && px < (platformArr[j].x + (64 << 8))) { //TODO take into account platforms which move
				
				int py = player.y + (64 << 8); //The foot of the character
				
				if(py <= (platformArr[j].y + (16 << 8))) {
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

