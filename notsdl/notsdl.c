#include "SDL2/SDL.h"

#include <proto/lowlevel.h>
#include <proto/exec.h>

#include <SDI_compiler.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct EClockVal timeval;
static int timecount = 0;

// various

const char* SDL_GetError(void)
{
	return "Something in the way.";
}

/*
void SDL_Log(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
}
*/

int SDL_Init(Uint32 flags)
{
	ElapsedTime(&timeval);
	return 0;
}

void SDL_Quit(void)
{
	// maybe clean up something here
}

// timer

Uint32 SDL_GetTicks(void)
{
	timecount += ElapsedTime(&timeval);
	return (int)((double)timecount*1000.0/65536.0); // fixed to msec
}
