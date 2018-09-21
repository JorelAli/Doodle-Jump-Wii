/*---------------------------------------------------------------------------------

	Doodlejump - Written by Jorel Ali
	
	gamemodes branch outcomes:
	- Add a title screen. Lets you select 2 or 3 "modes":
		- Solo mode: full screen, regular doodle jump (what we already have)
		- Multiplayer mode coop: full screen, try to get as high as possible together. (Make sure there are two platforms per platform so ghost platforms don't ruin everything)
		- Multiplayer mode competitive: split screen, first to die loses
	
		- Options menu:
			- Lets you reset the highscore
	
	Known bugs:
	- When a white platform and a brown platform are basically on top of each other, the white platform has no effect and the player goes through it.
	- Coop mode with ghost platforms is currently impossible
	- Coop mode highscore doesn't exist
	- Single player doesn't work
	
	gm_refactoring branch outcomes:
	- Refactor code into multiple files
	- Make project easier to manage
	- Implement pointers
	- Improve memory usage (the latest gamemodes branch build didn't load properly - assume memory management issues)

	menu-gui branch outcomes:
	- Implement and complete home menu screen. (NOT pause menu)
	
---------------------------------------------------------------------------------*/

//Header files --------------------------------------------------------------------
//Standard libs
#include <stdlib.h>
#include <stdio.h>

//devkitPPC libs
#include <wiiuse/wpad.h>

//Graphics lib
#include <grrlib.h>

//Music libs
#include <asndlib.h>
#include <mp3player.h>

//Sound files
#include "fall_mp3.h"
#include "jump_mp3.h"
#include "break_mp3.h"
#include "ghost_mp3.h"
#include "spring_mp3.h"
#include "win_mp3.h"

//Other
#include "grrlibtext.h"
#include "djtextures.h"
#include "djplatforms.h"

//---------------------------------------------------------------------------------
 
//Game Constants ------------------------------------------------------------------
//Stuff required to keep the game running normally (DO NOT CHANGE THESE)
#define GRAVITY_CONSTANT			32	//How fast gravity is
#define LINE_OF_MOVEMENT			140	//An invisible line, when crossed (above), it moves platforms downwards, creating the illusion of travelling upwards

//The player
#define PLAYER_JUMP_HEIGHT			100	//The minimum height between player jumps:
										//The actual height of a jump is precisely 120, but we can't exceed that (so 100 is good :D)
#define PLAYER_X_AXIS_SPEED 		8	//How quickly the character can go left/right by tilting

#define PLAYER_START_X		 		100	//Starting location for the player (x-axis)
#define PLAYER_START_Y		 		300	//Starting location for the player (y-axis)

#define PLAYER2_START_X		 		450	//Starting location for the player2 (x-axis)

//Game content
#define GAME_STATE_CHANGE_FREQ		500	//How many points the player must earn to change the state of the game

//Misc
#define DEBUG_MODE					1	//Debug mode (0 = off, 1 = on)
#define CHEAT_MODE					1	//Cheat mode (0 = off, 1 = on)
//---------------------------------------------------------------------------------

//ENUM DECLARATION ----------------------------------------------------------------

//Obstacles (monsters etc.)
typedef enum {
	BLACK_HOLE
} Obstacles;

//Gamestate (what the platform structure looks like)
typedef enum {
	STATE_NORMAL, 		//Regular green platforms. A few springs
	STATE_NORMAL_MOV,	//Green platforms and moving platforms
	STATE_NORMAL_BR,	//Green platforms, moving platforms and breaking platforms
	STATE_GHOST,		//Only ghost and breaking platforms
	STATE_COUNT_VAR		//Used to get the number of states in this enum (see http://www.cplusplus.com/forum/beginner/161968/)
} GameState;

//Program state (what the program is doing)
typedef enum {	
	MENU,				//Starting menu
	OPTIONS_MENU,		//Options menu (second screen of starting menu)
	SOLO,				//Solo mode (current doodlejump)
	MULTIPLAYER_COOP,	//Multiplayer coop
	MULTIPLAYER_PVP		//Multiplayer competitive
} ProgramState;
//---------------------------------------------------------------------------------

//STRUCTURE DECLARATION -----------------------------------------------------------
//Player object
typedef struct {
	int x,y;			// screen co-ordinates 
	int bitShiftDy;		// velocity. This value is bitshifted to the left by 8 bits:
	/*
		Whenever you read the value of dy, you use (bitShiftDy >> 8) to convert it back to 480 height
		Whenever you modify the value of dy, you use (bitShiftDy += (VALUE << 8)) to convert it to
			the shifted dy value.
			
		This bitshifting adds a level of precision for gravity, so movement isn't too fast and isn't
		too jagged. (By having a larger number, it can be "scaled" better. For example, you can't
		scale 1 to 0.125 for an integer (this equals 0)). This then automatically makes certain frames
		equal (but appear smooth for humans).
	*/
	int direction; 		//direction: 0 = left, 1 = right
}Player;

//---------------------------------------------------------------------------------

int score = 0;
int score2 = 0;			//Score for pvp
int highscore = 0;

Player player;			//Global player objects
Player player2;

Platform platformArr[NUM_PLATFORMS];
Platform platformArrPvp[NUM_PLATFORMS_PVP]; //Used for pvp mode

//what the platforms looks like
int gamestateScore = 0;
GameState currentGameState = NORMAL;

ProgramState currentProgramState = MENU;

int cheats = 0;			//Number of times the player has pressed A or 2
int paused = 0; 		// 0 = playing, 1 = paused
int gameover = 0;		// 0 = playing normally, 1 = gameover state

int paused_menu_selection = 0;

//METHOD DECLARATION --------------------------------------------------------------
void drawDoodleJumper(int x, int y, int direction, int player);	//Draws the player
void drawPlatform(int x, int y, PlatformType type, int frame);		//Draws a platform
PlatformType touchesPlatform(Player player);						//Checks if the player bounces on a platform
void drawBackground();												//Draws the background
void drawPaused();													//Draws the pause screen
void createPlatform(int index);										//Creates a platform at index for platformArr[] 
void drawAllPlatforms();											//Draws all of the platforms from platformArr[]
void writeHighScore();												//Stores the highscore to a file
void loadHighScore();												//Loads the highscore from a file
void drawBar();														//Draws the bar at the top (where the score is shown)
void drawGameover();												//Draws the game over screen
void preGameOver();													//Saves the highscore. If the player presses HOME when they die, highscore is now saved
void init();														//Initialises the program

void initMain();
int menuTouch();													//Same as touchesPlatform, but just for the menu (1 platform)

void initSolo();
void doSolo();
void gameOver();													//Resets the player, score and platforms

void initCoop();
void doCoop();
void gameOverCoop();

void initPvp();
void doPvp();
void gameOverPvp();

void createPlatformPvp(int index);
void drawAllPlatformsPvp();

//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
void initMain() {
	//Init player with base values, but different starting position 
	player.x = 430;	//center location
	player.y = 220;	//center
	player.bitShiftDy = 0;
	player.direction = 0;
	
	//Generate a platform under the player
	platformArr[0].x = player.x;
	platformArr[0].y = player.y + 65;
}
//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
int menuTouch() {
//---------------------------------------------------------------------------------
	
	int px = (player.x + 32); //Center x-coordinate of the player
	int py = player.y + (64); //The foot of the character
	
	if(py <= (platformArr[0].y + 16) && py >= (platformArr[0].y) && (player.bitShiftDy >> 8) >= 0) {
		if(px >= (platformArr[0].x) && px <= ((platformArr[0].x) + 64)) { 
			MP3Player_PlayBuffer(jump_mp3, jump_mp3_size, NULL); 
			return 1;
		}
	}	
	return 0;
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv){
//---------------------------------------------------------------------------------
	
	//Initialise program
	init();
	
	// MENU VARIABLES
	currentProgramState = MENU;
	int menu_selected = 0;
	// END OF MENU
		
	//Wii remote information
	WPAD_ScanPads();
	
	//Dummy player setup for main menu
	initMain();
	
	int debugvar = 0;

	//Main game loop
	while(1) {
		
		//Literally the most important code
		
		//Get latest data from wiimote
		WPAD_ScanPads();

		//If home button, exit
		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_HOME) {
		
			//Free up texture memory
			TEXTURES_Exit();
			
			//Exit GRRLib
			GRRLIB_Exit();
			exit(0);
		}
		
		//Main game code
		switch(currentProgramState) {
			case MENU:
				
				//down
				if(WPAD_ButtonsDown(0) & WPAD_BUTTON_LEFT) {
					menu_selected += 1;
					if(menu_selected == 4) {
						menu_selected = 0;
					}
				}
				
				//select up
				if(WPAD_ButtonsDown(0) & WPAD_BUTTON_RIGHT) {
					menu_selected -= 1;
					if(menu_selected == -1) {
						menu_selected = 3;
					}
				}
				
				drawBackground();
				
				drawText(ALIGN_CENTER, 65, FONT_Doodle_Bold, GRRLIB_DOODLE, "-- Doodlejump --");
				
				//Drawing GUI menu
				GRRLIB_DrawImg(70, 150, GFX_Singleplayer_Button, 0, 1, 1, RGBA(255, 255, 255, 255));
				GRRLIB_DrawImg(70, 230, GFX_Coop_Button, 0, 1, 1, RGBA(255, 255, 255, 255));
				GRRLIB_DrawImg(70, 310, GFX_Competitive_Button, 0, 1, 1, RGBA(255, 255, 255, 255));
				GRRLIB_DrawImg(70, 390, GFX_Options_Button, 0, 1, 1, RGBA(255, 255, 255, 255));

				switch(menu_selected) {
					case 0:
						GRRLIB_DrawImg(65, 140, GFX_Selected_Button, 0, 1, 1, RGBA(255, 255, 255, 255));
						break;
					case 1:
						GRRLIB_DrawImg(65, 220, GFX_Selected_Button, 0, 1, 1, RGBA(255, 255, 255, 255));
						break;
					case 2:
						GRRLIB_DrawImg(65, 300, GFX_Selected_Button, 0, 1, 1, RGBA(255, 255, 255, 255));
						break;
					case 3:
						GRRLIB_DrawImg(65, 380, GFX_Selected_Button, 0, 1, 1, RGBA(255, 255, 255, 255));
						break;
				}
				
				/******* DUMMY PLAYER ANIMATION ************/
						
				//Apply gravity
				player.bitShiftDy += GRAVITY_CONSTANT; // 32 = 1 << 5
				
				//Player landing on a platform
				if(menuTouch()) {
					player.bitShiftDy = -(PLATFORM_JUMP_CONSTANT << 8);
					debugvar = 1;
				}
					
				//Update player.y location
				player.y += (player.bitShiftDy >> 8);	
				
				//Draw the player
				drawDoodleJumper(player.x, player.y, player.direction, 0);
				
				//Drawing of platforms
				drawAllPlatforms();

				//Mode selection
				
				if(WPAD_ButtonsDown(0) & WPAD_BUTTON_2) {
					switch(menu_selected) {
						case 0:
							currentProgramState = SOLO;
							initSolo();
							//initPlatformArr(0);
							break;
						case 1:
							currentProgramState = MULTIPLAYER_COOP;
							initCoop();
							break;
						case 2:
							currentProgramState = MULTIPLAYER_PVP;
							initPvp();
							break;
						case 3:
							//Options menu hasn't been implemented yet
							break;
					}	
				}
				
				//Debugging
				if(DEBUG_MODE == 1) {
					GRRLIB_Line(0, LINE_OF_MOVEMENT, 640, LINE_OF_MOVEMENT, GRRLIB_BLACK);
					int heightConst = 50;
					GRRLIB_Printf(5, heightConst - 30, FONT_Doodle_Bold, GRRLIB_BLACK, 1, "debugvar: %d", debugvar);
					GRRLIB_Printf(5, heightConst, FONT_Doodle_Bold, GRRLIB_BLACK, 1, "dy: %d (%d)", (player.bitShiftDy >> 8), player.bitShiftDy);
					GRRLIB_Printf(5, heightConst + 30, FONT_Doodle_Bold, GRRLIB_BLACK, 1, "c: (%d, %d)", player.x, player.y);
					GRRLIB_Printf(5, heightConst + 60, FONT_Doodle_Bold, GRRLIB_BLACK, 1, "rY:      %d", rY);
				}

				GRRLIB_Render();
				break;
			case OPTIONS_MENU:
				break;
			case SOLO:
				doSolo();
				break;
			case MULTIPLAYER_COOP:
				doCoop();
				break;
			case MULTIPLAYER_PVP:
				doPvp();
				break;
		}
		
	}
	return 0;
}


//---------------------------------------------------------------------------------
void init() {
//---------------------------------------------------------------------------------

	// Initialise the audio subsystem
	ASND_Init(NULL);
	MP3Player_Init();

	//Init GRRLIB
	GRRLIB_Init();
	
	//Init textures
	TEXTURES_Init();

	//Initialise controllers
	WPAD_Init();
	
	//Allow access to gforce (acceleration)
	WPAD_SetDataFormat(WPAD_CHAN_ALL,WPAD_FMT_BTNS_ACC); //Access all channels (wiimotes) and don't require access to IR, so don't use it.

	//Setup random generator (this chooses a random seed for rand() generation)
	srand(time(NULL));
}



void initSolo() {

	//Init player 
	player.x = PLAYER_START_X;	//center location
	player.y = PLAYER_START_Y;	//center
	player.bitShiftDy = 0;
	player.direction = 0;

	platformArray[0].x = player.x;
	platformArray[0].y = player.y + 65;
	
	//Generate a platform under the player
	//platformArr[0].x = player.x;
	//platformArr[0].y = player.y + 65;
	
	//Generate platforms all over the place
	int i;
	for(i = 1; i < NUM_PLATFORMS; i++) {
		createPlatform(i); //TODO: Fix this
	}
	
	//Load high score from file
	loadHighScore();

}

void doSolo() {

	int i;

	gforce_t gforce; //wiimote acceleration

	//Update acceleration
	WPAD_GForce(0, &gforce); 
	
	//If they press + whilst the game is running
	if ((WPAD_ButtonsDown(0) & WPAD_BUTTON_PLUS) && (!paused)) {
		paused = 1; //Pause the game
	}
	
	if(CHEAT_MODE) {
		//Pressing B will increase score by 250.
		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_B){		
			score += 250;
		}
		
		//Pressing 2 will simulate a player jump (for testing purposes)
		if (WPAD_ButtonsDown(0) & WPAD_BUTTON_2 ){
			player.bitShiftDy = -(PLATFORM_JUMP_CONSTANT << 8);
			cheats++;
		}
	}
	
	
	
	//Restart the game
	if ((WPAD_ButtonsDown(0) & WPAD_BUTTON_A) && (gameover)){
		gameOver();
	}
	
	//Variable to manage the LINE OF MOVEMENT
	int rY = player.y;
	
	//If not paused, or the player hasn't lost
	if(!paused && !gameover) {
	
		//Apply gravity
		player.bitShiftDy += GRAVITY_CONSTANT; // 32 = 1 << 5
		
		//Player landing on a platform
		switch(touchesPlatform(player)) {
			case SPRING:
				player.bitShiftDy = -(PLATFORM_SPRING_CONSTANT << 8);
				break;
			case NO_PLATFORM:
				break;
			default:
				player.bitShiftDy = -(PLATFORM_JUMP_CONSTANT << 8);
				break;
		}
	
		//Player movement
		player.x += (int) (-1 * PLAYER_X_AXIS_SPEED * gforce.y);	//	gforce.y is the left/right tilt of wiimote when horizontal (2 button to the right)
		player.y += player.bitShiftDy >> 8;		
					
		//Move platforms when the player is above the line of movement and the player is NOT falling
		if(player.y <= ((LINE_OF_MOVEMENT)) && (player.bitShiftDy >> 8) <= 0) { 
			rY = LINE_OF_MOVEMENT;// TODO: Just set dy = 0 using a rdY variable - this prevents gravity, therefore y never changes, but dy will (because rdY)
			player.y += PLATFORM_JUMP_CONSTANT;
			score++;
			
			for(i = 0; i < NUM_PLATFORMS; i++) {
				platformArray[i].y += (PLATFORM_JUMP_CONSTANT);// From the gravity code above
				
				//If the platform is off of the screen
				if(platformArray[i].y > (480)) {
					createPlatform(i);
				}
			}
		} else {
			rY = player.y;
		}
		
		//Modify gamestate
		if(score >= gamestateScore + GAME_STATE_CHANGE_FREQ) {
			gamestateScore = score;
			currentGameState = rand() % STATE_COUNT_VAR;
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
		
		//Player dies by falling
		if(player.y > (480-32)) {
			MP3Player_PlayBuffer(fall_mp3, fall_mp3_size, NULL);
			gameover = 1;
		}
	} 
	
	//---------------------------------------------------------------------------------
	// VIDEO RENDERING
	//---------------------------------------------------------------------------------
					
	//Background
	drawBackground();
	
	if(!gameover) {
		//Drawing of platforms and player
		drawDoodleJumper( player.x, rY, player.direction, 0);
		
		//Drawing of platforms
		drawAllPlatforms();
		
		//Draw paused screen
		if(paused) {
			//Paused menu handler
			//handle directional inputs for menu
			
			//lovely dark(er) background
			GRRLIB_Rectangle(0, 0, 640, 480, RGBA(0, 0, 0, 100), 1);
			
			
			//selection. There are only two options (resume and quit), so we can toggle them
			if((WPAD_ButtonsDown(0) & WPAD_BUTTON_UP) || (WPAD_ButtonsDown(0) & WPAD_BUTTON_DOWN)) {
				paused_menu_selection ^= 1;
			}
			
						
			drawText(ALIGN_CENTER, 170, FONT_Doodle_Bold, GRRLIB_DOODLE, "-- Paused --");
			
			//Drawing GUI menu
			GRRLIB_DrawImg(122, 254, GFX_Resume_Button, 0, 1, 1, RGBA(255, 255, 255, 255));
			GRRLIB_DrawImg(340, 254, GFX_Quit_Button, 0, 1, 1, RGBA(255, 255, 255, 255));
			
			switch(paused_menu_selection) {
				//resume button
				case 0:
					GRRLIB_DrawImg(117, 249, GFX_Selected_Button, 0, 1, 1, RGBA(255, 255, 255, 255));
					break;
				//quit button
				case 1:
					GRRLIB_DrawImg(335, 249, GFX_Selected_Button, 0, 1, 1, RGBA(255, 255, 255, 255));
					break;
			}
			
			if(WPAD_ButtonsDown(0) & WPAD_BUTTON_2) {
				if(paused_menu_selection == 0) {
					paused = 0; //unpause
				} else if(paused_menu_selection == 1) {
					paused = 0;	//prevent starting a new game as paused
					paused_menu_selection = 0; //reset menu selection
					initMain();
					currentProgramState = MENU;
				} 
					
			  //unpause regardless if they press the + button
			}			
		}
	} else {
		preGameOver();	//Saves highscore!
		drawGameover();
	}
	
	//Draw the bar - this has to be overlaying the platforms, but before the score
	drawBar();
	
	//Draw the score
	if(cheats == 0) {
		drawText(ALIGN_LEFT, 10, FONT_Doodle_Bold, GRRLIB_BLACK, "Score: %d", score);
	} else {
		drawText(ALIGN_LEFT, 10, FONT_Doodle_Bold, GRRLIB_BLACK, "Score: (%d)", score);
	}
	
	if(highscore != 0) {
		drawText(ALIGN_RIGHT, 10, FONT_Doodle_Bold, GRRLIB_BLACK, "Highscore: %d", highscore);
	}
	
	//Debugging
	if(DEBUG_MODE == 1) {
		GRRLIB_Line(0, LINE_OF_MOVEMENT, 640, LINE_OF_MOVEMENT, GRRLIB_BLACK);
		int heightConst = 50;
		GRRLIB_Printf(5, heightConst, FONT_Doodle_Bold, GRRLIB_BLACK, 1, "dy: %d (%d)", (player.bitShiftDy >> 8), player.bitShiftDy);
		GRRLIB_Printf(5, heightConst + 30, FONT_Doodle_Bold, GRRLIB_BLACK, 1, "c: (%d, %d)", player.x, player.y);
		GRRLIB_Printf(5, heightConst + 60, FONT_Doodle_Bold, GRRLIB_BLACK, 1, "rY:      %d", rY);
	}
	
	GRRLIB_Render();  // Render the frame buffer to the TV	
	
	//---------------------------------------------------------------------------------
	
	//Take a screenshot :)
	if (WPAD_ButtonsDown(0) & WPAD_BUTTON_1){
		char buf[12];
		sprintf(buf, "sc%d.png", rand() % 20);// generate a random sc2.png file (for example)
		GRRLIB_ScrShot(buf);
	}

}

void initCoop() {
	
	//Init player 
	player.x = PLAYER_START_X;	//center location
	player.y = PLAYER_START_Y;	//center
	player.bitShiftDy = 0;
	player.direction = 0;
	
	player2.x = PLAYER2_START_X;	//center location
	player2.y = PLAYER_START_Y;	//center
	player2.bitShiftDy = 0;
	player2.direction = 0;

	//Generate a platform under the player
	platformArr[0].x = player.x;
	platformArr[0].y = player.y + 65;
	
	//Generate a platform under the player2
	platformArr[1].x = player2.x;
	platformArr[1].y = player2.y + 65;
	
	//Generate platforms all over the place
	int i;
	for(i = 2; i < NUM_PLATFORMS; i++) {
		createPlatform(i);
	}

}
void doCoop() {

	int i;

	gforce_t gforce1; //wiimote acceleration player 1
	gforce_t gforce2; //wiimote acceleration player 2

	//Update acceleration
	WPAD_GForce(WPAD_CHAN_0, &gforce1); 
	WPAD_GForce(WPAD_CHAN_1, &gforce2); 
	
	//Pause the game
	if (WPAD_ButtonsDown(0) & WPAD_BUTTON_PLUS){
		paused ^= 1;
	}
	
	//Restart the game
	if ((WPAD_ButtonsDown(0) & WPAD_BUTTON_A) && (gameover)){
		gameOverCoop();
	}
	
	int rY = player.y;
	int rY2 = player2.y;
	
	//If not paused, or the player hasn't lost
	if(!paused && !gameover) {
	
		//Apply gravity
		player.bitShiftDy += GRAVITY_CONSTANT; // 32 = 1 << 5
		player2.bitShiftDy += GRAVITY_CONSTANT; // 32 = 1 << 5
		
		//Player landing on a platform
		switch(touchesPlatform(player)) {
			case SPRING:
				player.bitShiftDy = -(PLATFORM_SPRING_CONSTANT << 8);
				break;
			case NO_PLATFORM:
				break;
			default:
				player.bitShiftDy = -(PLATFORM_JUMP_CONSTANT << 8);
				break;
		}
		
		//Player landing on a platform
		switch(touchesPlatform(player2)) {
			case SPRING:
				player2.bitShiftDy = -(PLATFORM_SPRING_CONSTANT << 8);
				break;
			case NO_PLATFORM:
				break;
			default:
				player2.bitShiftDy = -(PLATFORM_JUMP_CONSTANT << 8);
				break;
		}
	
		//Player movement
		player.x += (int) (-1 * PLAYER_X_AXIS_SPEED * gforce1.y);	//	gforce.y is the left/right tilt of wiimote when horizontal (2 button to the right)
		player.y += player.bitShiftDy >> 8;		
		
		player2.x += (int) (-1 * PLAYER_X_AXIS_SPEED * gforce2.y);	//	gforce.y is the left/right tilt of wiimote when horizontal (2 button to the right)
		player2.y += player2.bitShiftDy >> 8;		
					
		//Move platforms when the player is above the line of movement and the player is NOT falling, OR PLAYER 2
		if((player.y <= ((LINE_OF_MOVEMENT)) && (player.bitShiftDy >> 8) <= 0) || (player2.y <= ((LINE_OF_MOVEMENT)) && (player2.bitShiftDy >> 8) <= 0)) { 
			
			if(player.y <= ((LINE_OF_MOVEMENT)) && (player.bitShiftDy >> 8) <= 0) {
				rY = LINE_OF_MOVEMENT;
			}
				
			if(player2.y <= ((LINE_OF_MOVEMENT)) && (player2.bitShiftDy >> 8) <= 0) {
				rY2 = LINE_OF_MOVEMENT;
			}
			
			player.y += PLATFORM_JUMP_CONSTANT;
			player2.y += PLATFORM_JUMP_CONSTANT;	

				
			score++;
			
			for(i = 0; i < NUM_PLATFORMS; i++) {
				platformArr[i].y += (PLATFORM_JUMP_CONSTANT);// From the gravity code above
				
				//If the platform is off of the screen
				if(platformArr[i].y > (480)) {
					createPlatform(i);
				}
			}
		} else {
			rY = player.y;
			rY2 = player2.y;
		}
		
		//Modify gamestate
		if(score >= gamestateScore + GAME_STATE_CHANGE_FREQ) {
			gamestateScore = score;
			currentGameState = rand() % STATE_COUNT_VAR;
		}			
		
		//player direction changes when going left/right
		if(gforce1.y <= 0) {
			player.direction = 1;
		} else {
			player.direction = 0;
		}	
		
		//player direction changes when going left/right
		if(gforce2.y <= 0) {
			player2.direction = 1;
		} else {
			player2.direction = 0;
		}	
		
		//Makes the player loop if they go left/right off the screen
		if(player.x < 1) 
			player.x = 640-64;
		
		if(player.x > (640-64)) 
			player.x = 1;
		
		//Player dies by falling
		if(player.y > (480-32)) {
			MP3Player_PlayBuffer(fall_mp3, fall_mp3_size, NULL);
			gameover = 1;
		}
		
		//Makes the player loop if they go left/right off the screen
		if(player2.x < 1) 
			player2.x = 640-64;
		
		if(player2.x > (640-64)) 
			player2.x = 1;
		
		//Player dies by falling
		if(player2.y > (480-32)) {
			MP3Player_PlayBuffer(fall_mp3, fall_mp3_size, NULL);
			gameover = 1;
		}
	} 
	
	//---------------------------------------------------------------------------------
	// VIDEO RENDERING
	//---------------------------------------------------------------------------------
					
	//Background
	drawBackground();
	
	if(!gameover) {
		//Drawing of platforms and player
		drawDoodleJumper(player.x, rY, player.direction, 0);
		drawDoodleJumper(player2.x, rY2, player2.direction, 1);
		
		//Drawing of platforms
		drawAllPlatforms();
		
		//Draw paused screen
		if(paused) {
			drawPaused();
		}
	} else {
		preGameOver();	//Saves highscore!
		drawGameover();
	}
	
	//Draw the bar - this has to be overlaying the platforms, but before the score
	drawBar();
	
	drawText(ALIGN_LEFT, 10, FONT_Doodle_Bold, GRRLIB_BLACK, "Score: %d", score);
	
	//Debugging
	if(DEBUG_MODE == 1) {
		GRRLIB_Line(0, LINE_OF_MOVEMENT, 640, LINE_OF_MOVEMENT, GRRLIB_BLACK);
		int heightConst = 50;
		GRRLIB_Printf(5, heightConst, FONT_Doodle_Bold, GRRLIB_BLACK, 1, "dy: %d (%d)", (player.bitShiftDy >> 8), player.bitShiftDy);
		GRRLIB_Printf(5, heightConst + 30, FONT_Doodle_Bold, GRRLIB_BLACK, 1, "c: (%d, %d)", player.x, player.y);
		GRRLIB_Printf(5, heightConst + 60, FONT_Doodle_Bold, GRRLIB_BLACK, 1, "rY:      %d", rY);
	}

	GRRLIB_Render();  // Render the frame buffer to the TV	
	
}

void initPvp() {

	//Init player 
	player.x = PLAYER_START_X;
	player.y = PLAYER_START_Y;
	player.bitShiftDy = 0;
	player.direction = 0;
	
	player2.x = PLAYER_START_X + 320;	
	player2.y = PLAYER_START_Y;	
	player2.bitShiftDy = 0;
	player2.direction = 0;

	score = 0;
	score2 = 0;

	//For PVP, all ODD indicies are for player 2, all EVEN indicies are for player 1.

	//Generate a platform under the player
	platformArr[0].x = player.x;
	platformArr[0].y = player.y + 65;
	
	//Generate a platform under the player2
	platformArr[1].x = player2.x;
	platformArr[1].y = player2.y + 65;
	
	//Generate platforms all over the place
	int i;
	for(i = 2; i < NUM_PLATFORMS_PVP; i+=2) {
		createPlatformPvp(i);
	}

}
void doPvp() {

	int i;

	gforce_t gforce1; //wiimote acceleration player 1
	gforce_t gforce2; //wiimote acceleration player 2

	//Update acceleration
	WPAD_GForce(WPAD_CHAN_0, &gforce1); 
	WPAD_GForce(WPAD_CHAN_1, &gforce2); 
	
	//Pause the game
	if (WPAD_ButtonsDown(0) & WPAD_BUTTON_PLUS){
		paused ^= 1;
	}
	
	//Restart the game
	if ((WPAD_ButtonsDown(0) & WPAD_BUTTON_A) && (gameover)){
		gameOverPvp(); 
	}
	
	int rY = player.y;
	int rY2 = player2.y;
	
	//If not paused, or the player hasn't lost
	if(!paused && !gameover) {
	
		//Apply gravity
		player.bitShiftDy += GRAVITY_CONSTANT; // 32 = 1 << 5
		player2.bitShiftDy += GRAVITY_CONSTANT; // 32 = 1 << 5
		
		//Player landing on a platform
		switch(touchesPlatform(player)) {
			case SPRING:
				player.bitShiftDy = -(PLATFORM_SPRING_CONSTANT << 8);
				break;
			case NO_PLATFORM:
				break;
			default:
				player.bitShiftDy = -(PLATFORM_JUMP_CONSTANT << 8);
				break;
		}
		
		//Player landing on a platform
		switch(touchesPlatform(player2)) {
			case SPRING:
				player2.bitShiftDy = -(PLATFORM_SPRING_CONSTANT << 8);
				break;
			case NO_PLATFORM:
				break;
			default:
				player2.bitShiftDy = -(PLATFORM_JUMP_CONSTANT << 8);
				break;
		}
	
		//Player movement
		player.x += (int) (-1 * PLAYER_X_AXIS_SPEED * gforce1.y);	//	gforce.y is the left/right tilt of wiimote when horizontal (2 button to the right)
		player.y += player.bitShiftDy >> 8;		
		
		player2.x += (int) (-1 * PLAYER_X_AXIS_SPEED * gforce2.y);	//	gforce.y is the left/right tilt of wiimote when horizontal (2 button to the right)
		player2.y += player2.bitShiftDy >> 8;		
					
		//Player 1 reached L_O_M
		if(player.y <= ((LINE_OF_MOVEMENT)) && (player.bitShiftDy >> 8) <= 0) { 
			
			if(player.y <= ((LINE_OF_MOVEMENT)) && (player.bitShiftDy >> 8) <= 0) {
				rY = LINE_OF_MOVEMENT;
			}
			
			player.y += PLATFORM_JUMP_CONSTANT;
				
			score++;
			
			for(i = 0; i < NUM_PLATFORMS_PVP; i+=2) {
				platformArrPvp[i].y += (PLATFORM_JUMP_CONSTANT);// From the gravity code above
				
				//If the platform is off of the screen
				if(platformArrPvp[i].y > (480)) {
					createPlatformPvp(i);
				}
			}
		} else {
			rY = player.y;
			rY2 = player2.y;
		}
		
		if(player2.y <= ((LINE_OF_MOVEMENT)) && (player2.bitShiftDy >> 8) <= 0) { 
				
			if(player2.y <= ((LINE_OF_MOVEMENT)) && (player2.bitShiftDy >> 8) <= 0) {
				rY2 = LINE_OF_MOVEMENT;
			}
			
			player2.y += PLATFORM_JUMP_CONSTANT;	
				
			score2++;
			
			for(i = 1; i < NUM_PLATFORMS_PVP; i+= 2) {
				platformArrPvp[i].y += (PLATFORM_JUMP_CONSTANT);// From the gravity code above
				
				//If the platform is off of the screen
				if(platformArrPvp[i].y > (480)) {
					createPlatformPvp(i - 1); //Only regenerate from even index
				}
			}
		}
		
		//Modify gamestate
		if(score >= gamestateScore + GAME_STATE_CHANGE_FREQ) {
			gamestateScore = score;
			currentGameState = rand() % STATE_COUNT_VAR;
		}			
		
		//player direction changes when going left/right
		if(gforce1.y <= 0) {
			player.direction = 1;
		} else {
			player.direction = 0;
		}	
		
		//player direction changes when going left/right
		if(gforce2.y <= 0) {
			player2.direction = 1;
		} else {
			player2.direction = 0;
		}	
		
		//Makes the player loop if they go left/right off the screen
		if(player.x < 1) 
			player.x = 320-64;
		
		if(player.x > (320-64)) 
			player.x = 1;
		
		//Player dies by falling
		if(player.y > (480-32)) {
			MP3Player_PlayBuffer(fall_mp3, fall_mp3_size, NULL);
			gameover = 1;
		}
		
		//Makes the player loop if they go left/right off the screen
		if(player2.x < 320) 
			player2.x = 640-64;
		
		if(player2.x > (640-64)) 
			player2.x = 320;
		
		//Player dies by falling
		if(player2.y > (480-32)) {
			MP3Player_PlayBuffer(fall_mp3, fall_mp3_size, NULL);
			gameover = 1;
		}
	} 
	
	//---------------------------------------------------------------------------------
	// VIDEO RENDERING
	//---------------------------------------------------------------------------------
					
	//Background
	drawBackground();
	
	if(!gameover) {
		//Drawing of platforms and player
		drawDoodleJumper(player.x, rY, player.direction, 0);
		drawDoodleJumper(player2.x, rY2, player2.direction, 1);
		
		//Drawing of platforms
		drawAllPlatformsPvp();
		
		//Draw paused screen
		if(paused) {
			drawPaused();
		}
	} else {
		preGameOver();	//Saves highscore!
		drawGameover();
	}
	
	//Draw the bar - this has to be overlaying the platforms, but before the score
	drawBar();
	
	drawText(ALIGN_LEFT, 10, FONT_Doodle_Bold, GRRLIB_BLACK, "Score (P1): %d", score);
	drawText(ALIGN_MIDDLE, 10, FONT_Doodle_Bold, GRRLIB_BLACK, "Score (P2): %d", score2);
	
	//Draw center line
	GRRLIB_Line(320, 0, 320, 480, GRRLIB_BLACK);
	
	GRRLIB_Render();

}



//---------------------------------------------------------------------------------
void preGameOver() {
//---------------------------------------------------------------------------------
	//update highscore
	if(score > highscore && cheats == 0) {
		highscore = score;
		writeHighScore();
	}
	
	//reset gamestate
	gamestateScore = 0;
	currentGameState = NORMAL;
}

//---------------------------------------------------------------------------------
void gameOver() {
//---------------------------------------------------------------------------------

	gameover = 0;

	//Reset player
	player.x = PLAYER_START_X;	//center location
	player.y = PLAYER_START_Y;	//center
	player.bitShiftDy = 0;
	
	//reset scores
	score = 0;
	cheats = 0;
	
	//Generate a platform under the player
	platformArr[0].x = player.x;
	platformArr[0].y = player.y + 65;
	platformArr[0].type = NORMAL;
	platformArr[0].dy = 0;
	platformArr[0].dx = 0;
	
	//Regenerate all platforms
	int i;
	for(i = 1; i < NUM_PLATFORMS; i++) {
		platformArr[i].y = 480;	
	}
	
	for(i = 1; i < NUM_PLATFORMS; i++) {
		createPlatform(i);
	}

}

//---------------------------------------------------------------------------------
void gameOverCoop() {
//---------------------------------------------------------------------------------

	gameover = 0;

	//Reset players
	player.x = PLAYER_START_X;	//center location
	player.y = PLAYER_START_Y;	//center
	player.bitShiftDy = 0;
	
	//Reset players
	player2.x = PLAYER2_START_X;	//center location
	player2.y = PLAYER_START_Y;	//center
	player2.bitShiftDy = 0;
	
	//reset scores
	score = 0;
	cheats = 0;
	
	//Generate a platform under the player
	platformArr[0].x = player.x;
	platformArr[0].y = player.y + 65;
	platformArr[0].type = NORMAL;
	platformArr[0].dy = 0;
	platformArr[0].dx = 0;
	
	//Generate a platform under the player
	platformArr[1].x = player2.x;
	platformArr[1].y = player2.y + 65;
	platformArr[1].type = NORMAL;
	platformArr[1].dy = 0;
	platformArr[1].dx = 0;
	
	//Regenerate all platforms
	int i;
	for(i = 2; i < NUM_PLATFORMS; i++) {
		platformArr[i].y = 480;	
	}
	
	for(i = 2; i < NUM_PLATFORMS; i++) {
		createPlatform(i);
	}

}

//---------------------------------------------------------------------------------
void gameOverPvp() {
//---------------------------------------------------------------------------------

	gameover = 0;

	//Init player 
	player.x = PLAYER_START_X;
	player.y = PLAYER_START_Y;
	player.bitShiftDy = 0;
	player.direction = 0;
	
	player2.x = PLAYER_START_X + 320;	
	player2.y = PLAYER_START_Y;	
	player2.bitShiftDy = 0;
	player2.direction = 0;

	score = 0;
	score2 = 0;

	//For PVP, all ODD indicies are for player 2, all EVEN indicies are for player 1.

	//Generate a platform under the player
	platformArr[0].x = player.x;
	platformArr[0].y = player.y + 65;
	
	//Generate a platform under the player2
	platformArr[1].x = player2.x;
	platformArr[1].y = player2.y + 65;
	
	//reset all platforms
	int i;
	for(i = 2; i < NUM_PLATFORMS; i++) {
		platformArr[i].y = 481;	
	}
	
	//Generate platforms all over the place
	for(i = 2; i < NUM_PLATFORMS_PVP; i+=2) {
		createPlatformPvp(i);
	}

}

//---------------------------------------------------------------------------------
void drawDoodleJumper(int x, int y, int direction, int player) {
//---------------------------------------------------------------------------------
	
	if(player == 0) {
		if(direction)
			GRRLIB_DrawImg(x, y, GFX_Player_Right, 0, 1, 1, RGBA(255, 255, 255, 255));
		else
			GRRLIB_DrawImg(x, y, GFX_Player_Left, 0, 1, 1, RGBA(255, 255, 255, 255));
	} else if(player == 1) {
		if(direction)
			GRRLIB_DrawImg(x, y, GFX_Player_Right2, 0, 1, 1, RGBA(255, 255, 255, 255));
		else
			GRRLIB_DrawImg(x, y, GFX_Player_Left2, 0, 1, 1, RGBA(255, 255, 255, 255));
	}

}

//---------------------------------------------------------------------------------
void drawAllPlatformsPvp() {
//---------------------------------------------------------------------------------

	int i;
	for(i = 0; i < NUM_PLATFORMS_PVP; i++) {
	
		switch(platformArrPvp[i].type) {
			case MOVING_HORIZ:
				if(!paused) {
					//Changes direction value of platform
					if(platformArrPvp[i].direction == 0) { //If it's going right
						
						if(platformArrPvp[i].dx > PLATFORM_MOVE_DISTANCE) { //If it's gone as far as it can go
							platformArrPvp[i].direction = 1; //Switch direction
						} else {
							platformArrPvp[i].dx = platformArrPvp[i].dx + platformArrPvp[i].speed;	//else, move it
						}
						
					} else if(platformArrPvp[i].direction == 1) {	//Otherwise, if it's going left
						if(platformArrPvp[i].dx < 0) {
							platformArrPvp[i].direction = 0; //Switch direction
						} else {
							platformArrPvp[i].dx = platformArrPvp[i].dx - platformArrPvp[i].speed;
						}
					}
				}
				drawPlatform(platformArrPvp[i].x + platformArrPvp[i].dx, platformArrPvp[i].y, platformArrPvp[i].type, 0);
				break;
			case MOVING_VERT:
				if(!paused) {
					
					//Changes direction value of platform
					if(platformArrPvp[i].direction == 0) { //If it's going up
						
						if(platformArrPvp[i].dy < 0) { //If it's gone as far as it can go upwards
							platformArrPvp[i].direction = 1; //Switch direction
						} else {
							platformArrPvp[i].dy = platformArrPvp[i].dy - platformArrPvp[i].speed;	//else, move it
						}
						
					} else if(platformArrPvp[i].direction == 1) {	//Otherwise, if it's going down
						if(platformArrPvp[i].dy > PLATFORM_MOVE_DISTANCE_VERT) {
							platformArrPvp[i].direction = 0; //Switch direction
						} else {
							platformArrPvp[i].dy = platformArrPvp[i].dy + platformArrPvp[i].speed;
						}
					}
				}
				drawPlatform(platformArrPvp[i].x, platformArrPvp[i].y + platformArrPvp[i].dy, platformArrPvp[i].type, 0);
				break;
			case BREAKING:
				if(platformArrPvp[i].animation > 0) {
					drawPlatform(platformArrPvp[i].x, platformArrPvp[i].y, platformArrPvp[i].type, platformArrPvp[i].animation++);
				} else {
					drawPlatform(platformArrPvp[i].x, platformArrPvp[i].y, platformArrPvp[i].type, 0);
				}
				
				if(platformArrPvp[i].animation == 5) {
					createPlatformPvp(i); //Assumes that this will be an even index, if odd, it'll still be replaced?
				}
				break;
			case NORMAL:
				drawPlatform(platformArrPvp[i].x, platformArrPvp[i].y, platformArrPvp[i].type, 0);
				break;
			case GHOST:
				drawPlatform(platformArrPvp[i].x, platformArrPvp[i].y, platformArrPvp[i].type, 0);
				break;
			case GOLD:
				platformArrPvp[i].animation++;
				
				//Loop animation
				if(platformArrPvp[i].animation == 6) {
					platformArrPvp[i].animation = 0;
				}
				drawPlatform(platformArrPvp[i].x, platformArrPvp[i].y, platformArrPvp[i].type, platformArrPvp[i].animation);
				break;
			case SPRING:
				if(platformArrPvp[i].animation == 1) {
					drawPlatform(platformArrPvp[i].x, platformArrPvp[i].y, platformArrPvp[i].type, 1);
				} else {
					drawPlatform(platformArrPvp[i].x, platformArrPvp[i].y, platformArrPvp[i].type, 0);
				}
				break;
			case NO_PLATFORM:
				break;
		}
	}
	
}

//---------------------------------------------------------------------------------
void drawAllPlatforms() {
//---------------------------------------------------------------------------------

	int i;
	for(i = 0; i < NUM_PLATFORMS; i++) {
	
		switch(platformArr[i].type) {
			case MOVING_HORIZ:
				if(!paused) {
					//Changes direction value of platform
					if(platformArr[i].direction == 0) { //If it's going right
						
						if(platformArr[i].dx > PLATFORM_MOVE_DISTANCE) { //If it's gone as far as it can go
							platformArr[i].direction = 1; //Switch direction
						} else {
							platformArr[i].dx = platformArr[i].dx + platformArr[i].speed;	//else, move it
						}
						
					} else if(platformArr[i].direction == 1) {	//Otherwise, if it's going left
						if(platformArr[i].dx < 0) {
							platformArr[i].direction = 0; //Switch direction
						} else {
							platformArr[i].dx = platformArr[i].dx - platformArr[i].speed;
						}
					}
				}
				drawPlatform(platformArr[i].x + platformArr[i].dx, platformArr[i].y, platformArr[i].type, 0);
				if(DEBUG_MODE) {
					GRRLIB_Line(platformArr[i].x, platformArr[i].y, platformArr[i].x + PLATFORM_MOVE_DISTANCE, platformArr[i].y, GRRLIB_BLACK);
				}
				break;
			case MOVING_VERT:
				if(!paused) {
					
					//Changes direction value of platform
					if(platformArr[i].direction == 0) { //If it's going up
						
						if(platformArr[i].dy < 0) { //If it's gone as far as it can go upwards
							platformArr[i].direction = 1; //Switch direction
						} else {
							platformArr[i].dy = platformArr[i].dy - platformArr[i].speed;	//else, move it
						}
						
					} else if(platformArr[i].direction == 1) {	//Otherwise, if it's going down
						if(platformArr[i].dy > PLATFORM_MOVE_DISTANCE_VERT) {
							platformArr[i].direction = 0; //Switch direction
						} else {
							platformArr[i].dy = platformArr[i].dy + platformArr[i].speed;
						}
					}
				}
				drawPlatform(platformArr[i].x, platformArr[i].y + platformArr[i].dy, platformArr[i].type, 0);
				if(DEBUG_MODE) {
					GRRLIB_Line(platformArr[i].x + 28, platformArr[i].y, platformArr[i].x + 28, platformArr[i].y + PLATFORM_MOVE_DISTANCE_VERT, GRRLIB_BLACK);
					GRRLIB_Line(platformArr[i].x + 20, platformArr[i].y, platformArr[i].x + 36, platformArr[i].y, GRRLIB_BLACK);
				}
				break;
			case BREAKING:
				if(platformArr[i].animation > 0) {
					drawPlatform(platformArr[i].x, platformArr[i].y, platformArr[i].type, platformArr[i].animation++);
				} else {
					drawPlatform(platformArr[i].x, platformArr[i].y, platformArr[i].type, 0);
				}
				
				if(platformArr[i].animation == 5) {
					createPlatform(i);
				}
				break;
			case NORMAL:
				drawPlatform(platformArr[i].x, platformArr[i].y, platformArr[i].type, 0);
				break;
			case GHOST:
				drawPlatform(platformArr[i].x, platformArr[i].y, platformArr[i].type, 0);
				break;
			case GOLD:
				platformArr[i].animation++;
				
				//Loop animation
				if(platformArr[i].animation == 6) {
					platformArr[i].animation = 0;
				}
				drawPlatform(platformArr[i].x, platformArr[i].y, platformArr[i].type, platformArr[i].animation);
				break;
			case SPRING:
				if(platformArr[i].animation == 1) {
					drawPlatform(platformArr[i].x, platformArr[i].y, platformArr[i].type, 1);
				} else {
					drawPlatform(platformArr[i].x, platformArr[i].y, platformArr[i].type, 0);
				}
				break;
			case NO_PLATFORM:
				break;
		}
	}
	
}

////---------------------------------------------------------------------------------
//void drawPlatform(int x, int y, PlatformType type, int frame) {
////---------------------------------------------------------------------------------
//
//	if(y <= 0) {
//		return;	//Don't draw it if it's off screen!
//	}
//	
//	switch(type) {
//		case NORMAL:
//			GRRLIB_DrawImg(x, y, GFX_Platform_Green, 0, 1, 1, RGBA(255, 255, 255, 255));
//			break;
//		case MOVING_HORIZ:
//			GRRLIB_DrawImg(x, y, GFX_Platform_Blue, 0, 1, 1, RGBA(255, 255, 255, 255));
//			break;
//		case MOVING_VERT:
//			GRRLIB_DrawImg(x, y, GFX_Platform_BlueH, 0, 1, 1, RGBA(255, 255, 255, 255));
//			break;
//		case BREAKING:
//			GRRLIB_DrawTile(x, y, GFX_Platform_Brown, 0, 1, 1, RGBA(255, 255, 255, 255), frame);
//			break;
//		case GHOST:
//			GRRLIB_DrawImg(x, y, GFX_Platform_White, 0, 1, 1, RGBA(255, 255, 255, 255));
//			break;
//		case SPRING:
//			GRRLIB_DrawTile(x, y, GFX_Platform_Spring, 0, 1, 1, RGBA(255, 255, 255, 255), frame);
//			break;
//		case GOLD:
//			GRRLIB_DrawTile(x, y, GFX_Platform_Gold, 0, 1, 1, RGBA(255, 255, 255, 255), frame);
//			break;
//		case NO_PLATFORM:
//			break;
//	}
//	
//}

//---------------------------------------------------------------------------------
void drawBackground() {
//---------------------------------------------------------------------------------
	GRRLIB_DrawImg(0, 0, GFX_Background, 0, 1, 1, RGBA(255, 255, 255, 255));
}

//---------------------------------------------------------------------------------
void drawBar() {
//---------------------------------------------------------------------------------
	GRRLIB_DrawImg(0, 0, GFX_Bar, 0, 1, 1, RGBA(255, 255, 255, 255));
}

//---------------------------------------------------------------------------------
void drawPaused() {
//---------------------------------------------------------------------------------
	
	//Draw a dark overlay over the main game
	GRRLIB_Rectangle(0, 0, 640, 480, RGBA(0, 0, 0, 100), 1);

	drawText(ALIGN_CENTER, 208, FONT_Doodle_Bold, GRRLIB_DOODLE, "PAUSED");
	drawText(ALIGN_CENTER, 238, FONT_Doodle_Bold, GRRLIB_DOODLE, "Press HOME to exit");
	drawText(ALIGN_CENTER, 268, FONT_Doodle_Bold, GRRLIB_DOODLE, "Other test stuff :)");
}

//---------------------------------------------------------------------------------
void drawGameover() {
//---------------------------------------------------------------------------------
	drawText(ALIGN_CENTER, 208, FONT_Doodle_Bold, GRRLIB_DOODLE, "GAME OVER");
	drawText(ALIGN_CENTER, 238, FONT_Doodle_Bold, GRRLIB_DOODLE, "Your final score is %d", score);
	if(score > highscore) {
		drawText(ALIGN_CENTER, 268, FONT_Doodle_Bold, GRRLIB_DOODLE, "You got a new highscore!");
		drawText(ALIGN_CENTER, 298, FONT_Doodle_Bold, GRRLIB_DOODLE, "Press A to restart, or HOME to exit");
	} else {
		drawText(ALIGN_CENTER, 268, FONT_Doodle_Bold, GRRLIB_DOODLE, "Press A to restart, or HOME to exit");
	}
	
}

//---------------------------------------------------------------------------------
void createPlatform(int index) {
//---------------------------------------------------------------------------------
	
	platformArr[index].type = NORMAL;
	platformArr[index].animation = 0;
	platformArr[index].dy = 0; 	//reset dy
	platformArr[index].dx = 0;	//reset dx	
	
	platformArr[index].speed = (rand() % (PLATFORM_MOVE_SPEED_MAX - PLATFORM_MOVE_SPEED_MIN)) + PLATFORM_MOVE_SPEED_MIN;
	
	
	switch(currentGameState) {
		case STATE_NORMAL:
			if(rand() % 3 == 0) {
				platformArr[index].type = SPRING;
			}
			break;	
		case STATE_NORMAL_MOV:
			// 1/2 probability
			if(rand() % 2 == 0) {
				if(rand() % 5 == 0) {
					platformArr[index].type = MOVING_VERT;
				} else {
					platformArr[index].type = MOVING_HORIZ;
				}
			}
			break;
		case STATE_NORMAL_BR:
			if(platformArr[index].type != MOVING_HORIZ) {
				// 1/2 probability OUT OF non-moving platforms
				if(rand() % 2 == 0) {
					platformArr[index].type = BREAKING;
				}
			}
			break;
		case STATE_GHOST:
			//if(score > 3000)
			if(rand() % 2 == 0) {
				platformArr[index].type = GHOST;
			} else {
				platformArr[index].type = BREAKING;
			}
			break;
		case STATE_COUNT_VAR:
			break;
	}
	
	if(rand() % PLATFORM_GOLD_RARITY == 0) {
		platformArr[index].type = GOLD;
	}
	
	//dx and dy are only changed for moving platforms
	if(platformArr[index].type == MOVING_HORIZ) {
		platformArr[index].x = rand() % (640 - 64 - PLATFORM_MOVE_DISTANCE);  	//This value takes into account the size of the platform
		platformArr[index].dx = rand() % PLATFORM_MOVE_DISTANCE;				//Gives platform a random x value (so all generated platforms don't look the same)
		platformArr[index].direction = rand() % 2;								//Gives platform a random direction
	} else if(platformArr[index].type == MOVING_VERT) {
		platformArr[index].x = rand() % (640 - 64);  //This value takes into account the size of the platform
		platformArr[index].dy = rand() % PLATFORM_MOVE_DISTANCE_VERT;
		platformArr[index].direction = rand() % 2;
	} else {
		platformArr[index].x = rand() % (640 - 64);  //This value takes into account the size of the platform
	}
	
	int minY = 480;
	
	int i;
	int breaking = 0;
	int moving_vert = 0;
	
	//Determins y value for this platform to be generated (the type has already been determined here)
	for(i = 0; i < NUM_PLATFORMS; i++) {
		//Ignore this index, we're writing to this index
		if(i == index) {
			continue;
		}
		//If platform is null?, ignore it TODO: Remove this
		if(platformArr[i].y == 0) {
			continue;
		}
		
		//Ignore breaking platforms, these "don't exist"
		if(platformArr[i].type == BREAKING) {
			breaking++;
			continue;
		}
		
		if(platformArr[i].type == MOVING_VERT) {
			moving_vert++;
		}		
	
		//Get min value of y.
		if(platformArr[i].y < minY) {
			minY = platformArr[i].y;
		}
	}
	
	//Re-generate this platform, don't have more than 1 moving vertical one per NUM_PLATFORMS
	//(Can cause levels to be impossible)
	if(moving_vert == 1 && platformArr[index].type == MOVING_VERT) {
		createPlatform(index);
	}
	
	//If there is a surplus of breaking platforms, replace it with a normal platform
	if(breaking > (NUM_PLATFORMS / 2) && platformArr[index].type == BREAKING) {
		switch(currentGameState) {
			case STATE_GHOST:
				platformArr[index].type = GHOST;
				break;
			default:
				platformArr[index].type = NORMAL;
				break;
		}
	}
	
	if(platformArr[index].type == MOVING_VERT) {
		platformArr[index].y = minY - PLAYER_JUMP_HEIGHT - PLATFORM_MOVE_DISTANCE_VERT;
	} else {
		platformArr[index].y = minY - PLAYER_JUMP_HEIGHT;
	}
}


//---------------------------------------------------------------------------------
//Only if the index is EVEN
void createPlatformPvp(int index) {
//---------------------------------------------------------------------------------
	
	if(index % 2 == 1) {
		return;	//ERROR, this event should never occur
	}
	
	platformArrPvp[index].type = NORMAL;
	platformArrPvp[index].animation = 0;
	platformArrPvp[index].dy = 0; 	//reset dy
	platformArrPvp[index].dx = 0;	//reset dx	
	
	platformArrPvp[index].speed = (rand() % (PLATFORM_MOVE_SPEED_MAX - PLATFORM_MOVE_SPEED_MIN)) + PLATFORM_MOVE_SPEED_MIN;
	
	
	switch(currentGameState) {
		case STATE_NORMAL:
			if(rand() % 3 == 0) {
				platformArrPvp[index].type = SPRING;
			}
			break;	
		case STATE_NORMAL_MOV:
			// 1/2 probability
			if(rand() % 2 == 0) {
				if(rand() % 5 == 0) {
					platformArrPvp[index].type = MOVING_VERT;
				} else {
					platformArrPvp[index].type = MOVING_HORIZ;
				}
			}
			break;
		case STATE_NORMAL_BR:
			if(platformArrPvp[index].type != MOVING_HORIZ) {
				// 1/2 probability OUT OF non-moving platforms
				if(rand() % 2 == 0) {
					platformArrPvp[index].type = BREAKING;
				}
			}
			break;
		case STATE_GHOST:
			//if(score > 3000)
			if(rand() % 2 == 0) {
				platformArrPvp[index].type = GHOST;
			} else {
				platformArrPvp[index].type = BREAKING;
			}
			break;
		case STATE_COUNT_VAR:
			break;
	}
	
	if(rand() % PLATFORM_GOLD_RARITY == 0) {
		platformArrPvp[index].type = GOLD;
	}
	
	//dx and dy are only changed for moving platforms
	if(platformArrPvp[index].type == MOVING_HORIZ) {
	
		//cannot exceed too far
		platformArrPvp[index].x = rand() % (320 - 64 - PLATFORM_MOVE_DISTANCE);  	//This value takes into account the size of the platform
		platformArrPvp[index].dx = rand() % PLATFORM_MOVE_DISTANCE;				//Gives platform a random x value (so all generated platforms don't look the same)
		platformArrPvp[index].direction = rand() % 2;								//Gives platform a random direction
	} else if(platformArr[index].type == MOVING_VERT) {
		platformArrPvp[index].x = rand() % (320 - 64);  //This value takes into account the size of the platform
		platformArrPvp[index].dy = rand() % PLATFORM_MOVE_DISTANCE_VERT;
		platformArrPvp[index].direction = rand() % 2;
	} else {
		platformArrPvp[index].x = rand() % (320 - 64);  //This value takes into account the size of the platform
	}
	
	int minY = 480;
	
	int i;
	int breaking = 0;
	int moving_vert = 0;
	
	//Determins y value for this platform to be generated (the type has already been determined here)
	for(i = 0; i < NUM_PLATFORMS; i++) {
		//Ignore this index, we're writing to this index
		if(i == index) {
			continue;
		}
		//If platform is null?, ignore it TODO: Remove this
		if(platformArrPvp[i].y == 0) {
			continue;
		}
		
		//Ignore breaking platforms, these "don't exist"
		if(platformArrPvp[i].type == BREAKING) {
			breaking++;
			continue;
		}
		
		if(platformArrPvp[i].type == MOVING_VERT) {
			moving_vert++;
		}		
	
		//Get min value of y.
		if(platformArrPvp[i].y < minY) {
			minY = platformArrPvp[i].y;
		}
	}
	
	//Re-generate this platform, don't have more than 1 moving vertical one per NUM_PLATFORMS
	//(Can cause levels to be impossible)
	if(moving_vert == 1 && platformArrPvp[index].type == MOVING_VERT) {
		createPlatformPvp(index);
	}
	
	//If there is a surplus of breaking platforms, replace it with a normal platform
	if(breaking > (NUM_PLATFORMS / 2) && platformArrPvp[index].type == BREAKING) {
		switch(currentGameState) {
			case STATE_GHOST:
				platformArrPvp[index].type = GHOST;
				break;
			default:
				platformArrPvp[index].type = NORMAL;
				break;
		}
	}
	
	if(platformArrPvp[index].type == MOVING_VERT) {
		platformArrPvp[index].y = minY - PLAYER_JUMP_HEIGHT - PLATFORM_MOVE_DISTANCE_VERT;
	} else {
		platformArrPvp[index].y = minY - PLAYER_JUMP_HEIGHT;
	}
	
	/* Recreate the exact same platform for the next index */
	
	platformArrPvp[index + 1].x = platformArrPvp[index].x + 320;
	platformArrPvp[index + 1].y = platformArrPvp[index].y;
	platformArrPvp[index + 1].type = platformArrPvp[index].type;
	
	platformArrPvp[index + 1].dx = platformArrPvp[index].dx;
	platformArrPvp[index + 1].dy = platformArrPvp[index].dy;
	platformArrPvp[index + 1].direction = platformArrPvp[index].direction;
	platformArrPvp[index + 1].animation = platformArrPvp[index].animation;
	platformArrPvp[index + 1].speed = platformArrPvp[index].speed;
	
	
}

//---------------------------------------------------------------------------------
PlatformType touchesPlatform(Player p) {
//---------------------------------------------------------------------------------
	int j;
	for(j = 0; j < NUM_PLATFORMS; j++) {
		int px = (p.x + 32); //Center x-coordinate of the player
		
		int py = p.y + (64); //The foot of the character
		
		//Because spring platforms have a different y height, we take that into account here
		
		//21 pixels down from the texture is the top of the platform
		if(platformArr[j].type == SPRING) {
			if(py <= (platformArr[j].y + 36) && py >= (platformArr[j].y + 21) && (p.bitShiftDy >> 8) >= 0) {
				if(px > platformArr[j].x && px < (platformArr[j].x + (64))) {
					platformArr[j].animation = 1; //Animation frame
					MP3Player_PlayBuffer(spring_mp3, spring_mp3_size, NULL); 
					return platformArr[j].type;
				}
				break;
			}
		} else {
			//Now takes into account dy and dx for moving platforms (these are 0 if non-moving
			if(py <= (platformArr[j].y + platformArr[j].dy + 16) && py >= (platformArr[j].y + platformArr[j].dy) && (p.bitShiftDy >> 8) >= 0) {
				if(px > (platformArr[j].x + platformArr[j].dx) && px < ((platformArr[j].x + platformArr[j].dx) + 64)) { 
					switch(platformArr[j].type) {
						case NORMAL:
						case MOVING_HORIZ:
						case MOVING_VERT:
							MP3Player_PlayBuffer(jump_mp3, jump_mp3_size, NULL); 
							return platformArr[j].type;
						case BREAKING:					
							platformArr[j].animation = 1; //Begin the animation process
							MP3Player_PlayBuffer(break_mp3, break_mp3_size, NULL);
							return NO_PLATFORM; //for all intents and purposes, breaking platforms don't exist :P
						case GHOST:
							MP3Player_PlayBuffer(ghost_mp3, ghost_mp3_size, NULL); 
							createPlatform(j);
							return platformArr[j].type;
						case GOLD:
							MP3Player_PlayBuffer(win_mp3, win_mp3_size, NULL); 
							score += PLATFORM_GOLD_POINTS;
							createPlatform(j);
							return platformArr[j].type;
						case NO_PLATFORM:
						case SPRING:
							return platformArr[j].type;
					}
				}
			}
		}		
	}
	return NO_PLATFORM;
}

//---------------------------------------------------------------------------------
void writeHighScore() {
//---------------------------------------------------------------------------------

	//The file
	FILE *fp;
	
	//Open it for writing (removes all content if it exists)
	fp = fopen("sd:/apps/doodlejump/highscore.txt", "w+");
	
	//Character buffer (string) with highscore
	char buf[12];
	sprintf(buf, "%d\n", highscore);
	
	//Write it to the file and close.
	fputs(buf, fp);
	fclose(fp);
}

//---------------------------------------------------------------------------------
void loadHighScore() { 
//---------------------------------------------------------------------------------

	FILE *fp;
	int hScore;

	fp = fopen("sd:/apps/doodlejump/highscore.txt", "r");
	
	//Scan until the first space
	fscanf(fp, "%d", &hScore);
	
	highscore = hScore;
	fclose(fp);	
}
