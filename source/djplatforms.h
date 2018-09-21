#ifndef __DOODLEJUMP_PLATFORMS__
#define __DOODLEJUMP_PLATFORMS__
/* ^^ these are the include guards */

#include <grrlib.h>

typedef enum {
	NORMAL, 
	MOVING_HORIZ, 
	MOVING_VERT, 
	BREAKING, 
	GHOST, 
	SPRING, 
	GOLD, 
	NO_PLATFORM
} PlatformType;

typedef struct {
	int x,y;
	PlatformType type;
	int dx;				//Used for moving platforms (horizontal)
	int dy;				//Used for moving platforms (vertical)
	int direction;		//Used for determining the direction of a moving horizontal platform: 0 = right, 1 = left
						//Also used for direction of vertical platforms: 0 = up, 1 = down
	int animation;		//Used for breaking animation for break platforms (brown)
	int speed;			//How fast the platform moves
  //int player;			//Which player this platform is assigned to (PVP)
}Platform;

//Platforms
#define NUM_PLATFORMS				14	//Number of platforms in the buffer
#define NUM_PLATFORMS_PVP			300	//Needs to accomodate for many players

#define PLATFORM_JUMP_CONSTANT		5	//The amount of "bounce" a platform has
#define PLATFORM_SPRING_CONSTANT	7	//The amount of "bounce" a springy platform has

#define PLATFORM_MOVE_SPEED_MIN		1	//How quickly moving platforms (blue) move (min speed)
#define PLATFORM_MOVE_SPEED_MAX		3	//How quickly moving platforms (blue) move (max speed)

#define PLATFORM_MOVE_DISTANCE		200	//How far a moving platform moves (horizontally)
#define PLATFORM_MOVE_DISTANCE_VERT	200	//How far a moving platform moves (vertically)

#define PLATFORM_GOLD_POINTS		100	//How many points a gold platform gives you
#define PLATFORM_GOLD_RARITY		100	//How rare gold platforms appear (1 / value)

/* Variables */
extern Platform platformArray[];
extern Platform platformArray2[];


/* Prototypes for the functions */


#endif