#include "djplatforms.h"
#include "djtextures.h"

#include <stdio.h> //This is required for NULL
#include <stdlib.h> //This is required for malloc/free etc.

Platform platformArray[NUM_PLATFORMS];
Platform platformArray2[NUM_PLATFORMS];

// 0 = solo (and coop?)
// 1 = pvp
//void initPlatformArr(int mode) {
//	
//	if(mode == 0) {
//
//	} 
//	
//	//Say we were to malloc enough space for 32 bytes * NUM_PLATFORMS:
//	
//	int *arr;
//    
//    // malloc() allocate the memory for 5 integers
//   // containing garbage values
//    arr = (int *)malloc(14 * 32); // 5*32bytes = 448 bytes
//    
//    // Deallocates memory previously allocated by malloc() function
//    free( arr );
//	
//	
//	
//	
//	
//	//else if(mode == 1) {
//	//	platformArray = realloc(platformArray, NUM_PLATFORMS_PVP * sizeof(Platform));
//	//	
//	//	if(platformArray == NULL) {
//	//		//realloc failed...
//	//	}
//	//}
//}
//
////Frees the platform array memory. Use this before re-initialising the platform array
//void destroyPlatformArr() {
//	free(platformArray);
//}

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

//Gamestate (what the platform structure looks like)
typedef enum {
	STATE_NORMAL, 		//Regular green platforms. A few springs
	STATE_NORMAL_MOV,	//Green platforms and moving platforms
	STATE_NORMAL_BR,	//Green platforms, moving platforms and breaking platforms
	STATE_GHOST,		//Only ghost and breaking platforms
	STATE_COUNT_VAR		//Used to get the number of states in this enum (see http://www.cplusplus.com/forum/beginner/161968/)
} GameState;

//---------------------------------------------------------------------------------
void createPlatform2(int index, int currentGameState) {
//---------------------------------------------------------------------------------
	
	if(platformArray == NULL) {
		initPlatformArr(0);
	}
	
	platformArray[index].type = NORMAL;
	platformArray[index].animation = 0;
	platformArray[index].dy = 0; 	//reset dy
	platformArray[index].dx = 0;	//reset dx	
	
	platformArray[index].speed = (rand() % (PLATFORM_MOVE_SPEED_MAX - PLATFORM_MOVE_SPEED_MIN)) + PLATFORM_MOVE_SPEED_MIN;
	
	
	switch(currentGameState) {
		case STATE_NORMAL:
			if(rand() % 3 == 0) {
				platformArray[index].type = SPRING;
			}
			break;	
		case STATE_NORMAL_MOV:
			// 1/2 probability
			if(rand() % 2 == 0) {
				if(rand() % 5 == 0) {
					platformArray[index].type = MOVING_VERT;
				} else {
					platformArray[index].type = MOVING_HORIZ;
				}
			}
			break;
		case STATE_NORMAL_BR:
			if(platformArray[index].type != MOVING_HORIZ) {
				// 1/2 probability OUT OF non-moving platforms
				if(rand() % 2 == 0) {
					platformArray[index].type = BREAKING;
				}
			}
			break;
		case STATE_GHOST:
			//if(score > 3000)
			if(rand() % 2 == 0) {
				platformArray[index].type = GHOST;
			} else {
				platformArray[index].type = BREAKING;
			}
			break;
		case STATE_COUNT_VAR:
			break;
	}
	
	if(rand() % PLATFORM_GOLD_RARITY == 0) {
		platformArray[index].type = GOLD;
	}
	
	//dx and dy are only changed for moving platforms
	if(platformArray[index].type == MOVING_HORIZ) {
		platformArray[index].x = rand() % (640 - 64 - PLATFORM_MOVE_DISTANCE);  	//This value takes into account the size of the platform
		platformArray[index].dx = rand() % PLATFORM_MOVE_DISTANCE;				//Gives platform a random x value (so all generated platforms don't look the same)
		platformArray[index].direction = rand() % 2;								//Gives platform a random direction
	} else if(platformArray[index].type == MOVING_VERT) {
		platformArray[index].x = rand() % (640 - 64);  //This value takes into account the size of the platform
		platformArray[index].dy = rand() % PLATFORM_MOVE_DISTANCE_VERT;
		platformArray[index].direction = rand() % 2;
	} else {
		platformArray[index].x = rand() % (640 - 64);  //This value takes into account the size of the platform
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
		if(platformArray[i].y == 0) {
			continue;
		}
		
		//Ignore breaking platforms, these "don't exist"
		if(platformArray[i].type == BREAKING) {
			breaking++;
			continue;
		}
		
		if(platformArray[i].type == MOVING_VERT) {
			moving_vert++;
		}		
	
		//Get min value of y.
		if(platformArray[i].y < minY) {
			minY = platformArray[i].y;
		}
	}
	
	//Re-generate this platform, don't have more than 1 moving vertical one per NUM_PLATFORMS
	//(Can cause levels to be impossible)
	if(moving_vert == 1 && platformArray[index].type == MOVING_VERT) {
		createPlatform2(index, currentGameState);
	}
	
	//If there is a surplus of breaking platforms, replace it with a normal platform
	if(breaking > (NUM_PLATFORMS / 2) && platformArray[index].type == BREAKING) {
		switch(currentGameState) {
			case STATE_GHOST:
				platformArray[index].type = GHOST;
				break;
			default:
				platformArray[index].type = NORMAL;
				break;
		}
	}
	
	if(platformArray[index].type == MOVING_VERT) {
		platformArray[index].y = minY - 100 - PLATFORM_MOVE_DISTANCE_VERT;
	} else {
		platformArray[index].y = minY - 100;
	}
}

//---------------------------------------------------------------------------------
void drawAllPlatforms2(int paused) {
//---------------------------------------------------------------------------------

	int i;
	for(i = 0; i < NUM_PLATFORMS; i++) {
	
		switch(platformArray[i].type) {
			case MOVING_HORIZ:
				if(paused == 0) {
					//Changes direction value of platform
					if(platformArray[i].direction == 0) { //If it's going right
						
						if(platformArray[i].dx > PLATFORM_MOVE_DISTANCE) { //If it's gone as far as it can go
							platformArray[i].direction = 1; //Switch direction
						} else {
							platformArray[i].dx = platformArray[i].dx + platformArray[i].speed;	//else, move it
						}
						
					} else if(platformArray[i].direction == 1) {	//Otherwise, if it's going left
						if(platformArray[i].dx < 0) {
							platformArray[i].direction = 0; //Switch direction
						} else {
							platformArray[i].dx = platformArray[i].dx - platformArray[i].speed;
						}
					}
				}
				drawPlatform(platformArray[i].x + platformArray[i].dx, platformArray[i].y, platformArray[i].type, 0);
				break;
			case MOVING_VERT:
				if(paused == 0) {
					
					//Changes direction value of platform
					if(platformArray[i].direction == 0) { //If it's going up
						
						if(platformArray[i].dy < 0) { //If it's gone as far as it can go upwards
							platformArray[i].direction = 1; //Switch direction
						} else {
							platformArray[i].dy = platformArray[i].dy - platformArray[i].speed;	//else, move it
						}
						
					} else if(platformArray[i].direction == 1) {	//Otherwise, if it's going down
						if(platformArray[i].dy > PLATFORM_MOVE_DISTANCE_VERT) {
							platformArray[i].direction = 0; //Switch direction
						} else {
							platformArray[i].dy = platformArray[i].dy + platformArray[i].speed;
						}
					}
				}
				drawPlatform(platformArray[i].x, platformArray[i].y + platformArray[i].dy, platformArray[i].type, 0);
				break;
			case BREAKING:
				if(platformArray[i].animation > 0) {
					drawPlatform(platformArray[i].x, platformArray[i].y, platformArray[i].type, platformArray[i].animation++);
				} else {
					drawPlatform(platformArray[i].x, platformArray[i].y, platformArray[i].type, 0);
				}
				
				if(platformArray[i].animation == 5) {
					createPlatform2(i, 0);
				}
				break;
			case NORMAL:
				drawPlatform(platformArray[i].x, platformArray[i].y, platformArray[i].type, 0);
				break;
			case GHOST:
				drawPlatform(platformArray[i].x, platformArray[i].y, platformArray[i].type, 0);
				break;
			case GOLD:
				platformArray[i].animation++;
				
				//Loop animation
				if(platformArray[i].animation == 6) {
					platformArray[i].animation = 0;
				}
				drawPlatform(platformArray[i].x, platformArray[i].y, platformArray[i].type, platformArray[i].animation);
				break;
			case SPRING:
				if(platformArray[i].animation == 1) {
					drawPlatform(platformArray[i].x, platformArray[i].y, platformArray[i].type, 1);
				} else {
					drawPlatform(platformArray[i].x, platformArray[i].y, platformArray[i].type, 0);
				}
				break;
			case NO_PLATFORM:
				break;
		}
	}
	
}