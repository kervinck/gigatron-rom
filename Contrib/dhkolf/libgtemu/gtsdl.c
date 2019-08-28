/*

    This file is part of libgtemu, a library for Gigatron emulation.
    Copyright (C) 2019 David Heiko Kolf

    Published under the BSD-2-Clause license.
    https://opensource.org/licenses/BSD-2-Clause

*/

#include "gtsdl.h"

static void resettime (struct GTSDLState *s)
{
	unsigned int currenttime = SDL_GetTicks();
	s->nextsecondtime = currenttime + 1000;
	s->nextframe3 = currenttime * 3 + 50; /* 1000/60 => 50/3 ms */
}

static void clearstate (struct GTSDLState *s, int standalone)
{
	s->mainwin = NULL;
	s->mainrenderer = NULL;
	s->gamescreen = NULL;
	s->audiodev = 0;
	s->fps = 0;
	s->standalone = standalone;
}

static SDL_Texture *createtexture (SDL_Renderer *renderer)
{
	return SDL_CreateTexture(renderer,
		SDL_PIXELFORMAT_RGB444,
		SDL_TEXTUREACCESS_STREAMING,
		GT_SCREENWIDTH * 4, GT_SCREENHEIGHT);
}

static int setupaudio (struct GTSDLState *s, SDL_AudioDeviceID audiodev,
	SDL_AudioSpec *audiospec)
{
	if (audiodev == 0) {
		/* No sound is being played but to simplify further
		   calculations, provide a plausible frequency. */
		s->audiofreq = 48000;
		return 1;
	}
	if (audiospec->channels != 1) {
		SDL_SetError("Only a single sound channel can be used, not %d",
			audiospec->channels);
		return 0;
	}
	if (SDL_AUDIO_ISFLOAT(audiospec->format)) {
		SDL_SetError("Floating point sound is not supported");
		return 0;
	}
	if (SDL_AUDIO_BITSIZE(audiospec->format) != 16) {
		SDL_SetError("Only 16 bit sounds are supported, not %d",
			SDL_AUDIO_BITSIZE(audiospec->format));
		return 0;
	}
	if (audiospec->callback != NULL) {
		SDL_SetError("Audio output must be in queue mode, without callback");
		return 0;
	}

	s->audiodev = audiodev;
	s->audiofreq = audiospec->freq;

	return 1;
}

int gtsdl_openwindow (struct GTSDLState *s, const char *title)
{
	SDL_AudioSpec desired, obtained;
	SDL_AudioDeviceID audiodev;

	clearstate(s, 1);

	if (SDL_InitSubSystem(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		return 0;
	}

	resettime(s);

	s->mainwin = SDL_CreateWindow(title,
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		GT_SCREENWIDTH * 4, GT_SCREENHEIGHT, SDL_WINDOW_SHOWN);

	if (s->mainwin == NULL) {
		return 0;
	}

	s->mainrenderer = SDL_CreateRenderer(s->mainwin, -1,
		SDL_RENDERER_PRESENTVSYNC);

	if (s->mainrenderer == NULL) {
		return 0;
	}

	s->gamescreen = createtexture(s->mainrenderer);

	if (s->gamescreen == NULL) {
		return 0;
	}

	SDL_memset(&desired, 0, sizeof(desired));
	desired.freq = 48000;
	desired.format = AUDIO_U16;
	desired.channels = 1;
	desired.samples = 4096;
	desired.callback = NULL;
#if 0
	/* For some reason SDL_OpenAudioDevice doesn't work for me,
	   but SDL_OpenAudio does. */
	audiodev = SDL_OpenAudioDevice(NULL, 0, &desired, &obtained,
		SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
#else
	if (SDL_OpenAudio(&desired, &obtained) < 0) {
		audiodev = 0;
	} else {
		audiodev = 1;
	}
#endif

	if (!setupaudio(s, audiodev, &obtained)) {
		return 0;
	} else if (s->audiodev != 0) {
		SDL_PauseAudio(0);
	}

	SDL_StartTextInput();

	return 1;
}

int gtsdl_setup (struct GTSDLState *s, SDL_Renderer *renderer,
	SDL_AudioDeviceID audiodev, SDL_AudioSpec *audiospec)
{
	clearstate(s, 0);

	resettime(s);

	s->gamescreen = createtexture(renderer);

	if (s->gamescreen == NULL) {
		return 0;
	}

	if (!setupaudio(s, audiodev, audiospec)) {
		return 0;
	}

	return 1;
}

void gtsdl_close (struct GTSDLState *s)
{
	if (s->gamescreen != NULL) {
		SDL_DestroyTexture(s->gamescreen);
		s->gamescreen = NULL;
	}

	if (s->standalone) {
		SDL_StopTextInput();

		if (s->audiodev != 0) {
			SDL_CloseAudioDevice(s->audiodev);
			s->audiodev = 0;
		}
		if (s->mainrenderer != NULL) {
			SDL_DestroyRenderer(s->mainrenderer);
			s->mainrenderer = NULL;
		}
		if (s->mainwin != NULL) {
			SDL_DestroyWindow(s->mainwin);
			s->mainwin = NULL;
		}

		SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	}
}

int gtsdl_getaudiofreq (struct GTSDLState *s)
{
	return s->audiofreq;
}

SDL_Texture *gtsdl_render (struct GTSDLState *s, struct GTState *gt,
	struct GTPeriph *ph)
{
	void *pixels;
	int pitch;
	static unsigned short audiobuffer[1024];
	size_t audiolen = 0;

	SDL_LockTexture(s->gamescreen, NULL, &pixels, &pitch);

	gtemu_processscreen(gt, ph, pixels, pitch,
		audiobuffer, 1024, &audiolen);
	gtemu_placelights(ph, pixels, pitch, s->fps >= 59 && s->fps <= 61);

	SDL_UnlockTexture(s->gamescreen);

	if (s->audiodev != 0 && audiolen > 0) {
		SDL_QueueAudio(s->audiodev, audiobuffer,
			audiolen * sizeof(audiobuffer[0]));
	}

	return s->gamescreen;
}

int gtsdl_runuiframe (struct GTSDLState *s, struct GTState *gt,
	struct GTPeriph *ph, SDL_Event *ev)
{
	SDL_Texture *texture;

	unsigned int currenttime, currenttime3;

	if (s->mainrenderer == NULL) {
		SDL_SetError("no standalone renderer configured");
		return -1;
	}

	if (SDL_PollEvent(ev)) {
		return 1;
	}

	texture = gtsdl_render(s, gt, ph);

	SDL_RenderCopy(s->mainrenderer, texture, NULL, NULL);
	currenttime = SDL_GetTicks();
	currenttime3 = 3 * currenttime;
	if (currenttime3 < s->nextframe3) {
		unsigned int difftime3 = s->nextframe3 - currenttime3;
		SDL_Delay(difftime3/3);
	}
	s->nextframe3 += 50;
	if (s->nextframe3 < currenttime3) {
		/* too slow, skip frames */
		s->nextframe3 = currenttime3 + 20;
	}
	s->framecount++;
	if (currenttime > s->nextsecondtime) {
		s->fps = s->framecount;
		s->framecount = 0;
		s->nextsecondtime += 1000;
	}

	SDL_RenderPresent(s->mainrenderer);
	return 0;
}

static int onplainkeydown (struct GTSDLState *s, struct GTState *gt,
	SDL_KeyboardEvent *ev)
{
	switch (ev->keysym.scancode) {
	case SDL_SCANCODE_DELETE:
		gt->in &= ~0x80;
		return 1;
	case SDL_SCANCODE_PAGEDOWN:
		gt->in &= ~0x20;
		return 1;
	case SDL_SCANCODE_INSERT:
		gt->in &= ~0x40;
		return 1;
	case SDL_SCANCODE_PAGEUP:
		gt->in &= ~0x10;
		return 1;
	case SDL_SCANCODE_DOWN:
		gt->in &= ~0x04;
		return 1;
	case SDL_SCANCODE_LEFT:
		gt->in &= ~0x02;
		return 1;
	case SDL_SCANCODE_RIGHT:
		gt->in &= ~0x01;
		return 1;
	case SDL_SCANCODE_UP:
		gt->in &= ~0x08;
		return 1;
	default:
		break;
	}
	switch (ev->keysym.sym) {
	case SDLK_RETURN:
		gt->in = '\n';
		s->textkeydown = 1;
		return 1;
	case SDLK_BACKSPACE:
		gt->in = 127;
		s->textkeydown = 1;
		return 1;
	case SDLK_TAB:
		gt->in = '\t';
		s->textkeydown = 1;
		return 1;
	default:
		return 0;
	}
}

static int onctrlkeydown (struct GTSDLState *s, struct GTState *gt,
	SDL_KeyboardEvent *ev)
{
	if (ev->keysym.sym >= 'a' && ev->keysym.sym <= 'z') {
		gt->in = ev->keysym.sym & 0x1f;
		s->textkeydown = 1;
		return 1;
	}
	return 0;
}

static int onkeydown (struct GTSDLState *s, struct GTState *gt,
	SDL_KeyboardEvent *ev)
{
	if (ev->keysym.mod == 0) {
		return onplainkeydown(s, gt, ev);
	}
	if ((ev->keysym.mod & KMOD_CTRL) && !(ev->keysym.mod & ~KMOD_CTRL)) {
		return onctrlkeydown(s, gt, ev);
	}
	return 0;
}

static int ontextinput (struct GTSDLState *s, struct GTState *gt,
	SDL_TextInputEvent *ev)
{
	if (SDL_GetModState() == KMOD_LALT) {
		/* Alt+X should not be text input */
		return 0;
	}
	if (ev->text[0] > 0 && ev->text[0] < 128) {
		gt->in = ev->text[0];
		s->textkeydown = 1;
	}
	return 1;
}

static int onkeyup (struct GTSDLState *s, struct GTState *gt,
	SDL_KeyboardEvent *ev)
{
	switch (ev->keysym.scancode) {
	case SDL_SCANCODE_DELETE:
		gt->in |= 0x80;
		return 1;
	case SDL_SCANCODE_PAGEDOWN:
		gt->in |= 0x20;
		return 1;
	case SDL_SCANCODE_INSERT:
		gt->in |= 0x40;
		return 1;
	case SDL_SCANCODE_PAGEUP:
		gt->in |= 0x10;
		return 1;
	case SDL_SCANCODE_DOWN:
		gt->in |= 0x04;
		return 1;
	case SDL_SCANCODE_LEFT:
		gt->in |= 0x02;
		return 1;
	case SDL_SCANCODE_RIGHT:
		gt->in |= 0x01;
		return 1;
	case SDL_SCANCODE_UP:
		gt->in |= 0x08;
		return 1;
	default:
		if (s->textkeydown) {
			s->textkeydown = 0;
			gt->in = 0xff;
		}
		return 0;
	}
}

int gtsdl_handleevent (struct GTSDLState *s, struct GTState *gt,
	SDL_Event *ev)
{
	switch (ev->type) {
	case SDL_KEYDOWN:
		return onkeydown(s, gt, &ev->key);
	case SDL_KEYUP:
		return onkeyup(s, gt, &ev->key);
	case SDL_TEXTINPUT:
		return ontextinput(s, gt, &ev->text);
	default:
		return 0;
	}
}

