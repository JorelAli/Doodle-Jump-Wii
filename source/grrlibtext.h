#include <grrlib.h>

#ifndef FUNCTIONS_H_INCLUDED
#define FUNCTIONS_H_INCLUDED
/* ^^ these are the include guards */

typedef enum {ALIGN_LEFT, ALIGN_RIGHT, ALIGN_CENTER, ALIGN_MIDDLE} Alignment;

/* Prototypes for the functions */
void drawText(Alignment align, f32 ypos, GRRLIB_texImg *font, u32 colour, char *text, ...);

#endif