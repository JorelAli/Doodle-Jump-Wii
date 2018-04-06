#include <grrlib.h>

#ifndef FUNCTIONS_H_INCLUDED
#define FUNCTIONS_H_INCLUDED
/* ^^ these are the include guards */

typedef enum {LEFT, RIGHT, CENTER} Alignment;

/* Prototypes for the functions */
void drawText(Alignment align, f32 ypos, GRRLIB_texImg *font, u32 colour, char *text);

#endif