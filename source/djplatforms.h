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

/* Variables */
extern Platform *platformArray;


/* Prototypes for the functions */


#endif