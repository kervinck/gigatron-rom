/*

    This file is part of libgtemu, a library for Gigatron emulation.
    Copyright (C) 2019 David Heiko Kolf

    Published under the BSD-2-Clause license.
    https://opensource.org/licenses/BSD-2-Clause

*/

#ifndef GTSDL_H
#define GTSDL_H 1

#include <SDL2/SDL.h>

#include "gtemu.h"

struct GTSDLState {
	SDL_Window *mainwin;
	SDL_Renderer *mainrenderer;
	SDL_Texture *gamescreen;
	SDL_AudioDeviceID audiodev;
	int standalone;
	int audiofreq;
	int fps;
	unsigned int nextframe3, nextsecondtime;
	unsigned int framecount;
	int textkeydown;
};

extern int gtsdl_openwindow (struct GTSDLState *s, const char *title);

extern int gtsdl_setup (struct GTSDLState *s, SDL_Renderer *renderer,
	SDL_AudioDeviceID audiodev, SDL_AudioSpec *audiospec);

extern void gtsdl_close (struct GTSDLState *s);

extern int gtsdl_getaudiofreq (struct GTSDLState *s);

extern SDL_Texture *gtsdl_render (struct GTSDLState *s, struct GTState *gt,
	struct GTPeriph *ph);

extern int gtsdl_runuiframe (struct GTSDLState *s, struct GTState *gt,
	struct GTPeriph *ph, SDL_Event *ev);

extern int gtsdl_handleevent (struct GTSDLState *s, struct GTState *gt,
	SDL_Event *ev);

#endif /* GTSDL_H */

