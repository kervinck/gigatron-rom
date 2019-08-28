/*

    This file is part of libgtemu, a library for Gigatron emulation.
    Copyright (C) 2019 David Heiko Kolf

    Published under the BSD-2-Clause license.
    https://opensource.org/licenses/BSD-2-Clause

*/

#include <stdio.h>
#include <time.h>

#include "gtsdl.h"

static int loadrom (const char *fname, struct GTState *gt)
{
	FILE *f;

	f = fopen(fname, "rb");

	if (!f) return 0;

	fread(gt->rom, sizeof(struct GTRomEntry), gt->romcount, f);

	fclose(f);

	return 1;
}

static void startloader(struct GTState *gt, struct GTPeriph *ph)
{
	unsigned char *ram = gt->ram;
	int i;
	unsigned long long startup, timeout;
	gtemu_randomizemem(time(0), ram, sizeof(ram));
	gt->pc = 0;
	gtemu_processtick(gt, ph);
	gt->pc = 0;
	startup = gtemu_getclock(ph) + 12000000ull;
	timeout = startup + 36000000ull;
	while (gtemu_getclock(ph) < startup) {
		gtemu_processtick(gt, ph);
	}
	for (i = 0; i < 5; i++) {
		/* Press the down button. */
		gt->in = 0xff ^ 4;
		/* Wait until the I/O loop copied the input to 0x07 (serialRaw)
		   and the Main app cleared it again in 0x11 (buttonState). */
		while (gtemu_getclock(ph) < timeout &&
			(ram[0x0f] != (0xff ^ 4) || ram[0x11] != 0xff)) {

			gtemu_processtick(gt, ph);
		}
		/* Release the down button. */
		gt->in = 0xff;
		while (gtemu_getclock(ph) < timeout && ram[0x0f] != 0xff) {
			gtemu_processtick(gt, ph);
		}
	}
	/* Press the A button. */
	gt->in = 0xff ^ 0x80;
	while (gtemu_getclock(ph) < timeout && (ram[0x0f] != (0xff ^ 0x80) || ram[0x11] != 0xff)) {
		gtemu_processtick(gt, ph);
	}
	gt->in = 0xff;
}

static int onkeydown(struct GTState *gt, struct GTPeriph *ph, SDL_KeyboardEvent *ev)
{
	if (ev->keysym.sym == 'l' && ev->keysym.mod == KMOD_LALT) {
		startloader(gt, ph);
		return 1;
	}
	return 0;
}

int main (int argc, char *argv[])
{
	struct GTSDLState s;
	struct GTState gt;
	struct GTPeriph ph;
	unsigned long randstate;

	static struct GTRomEntry rom[0x10000];
	static unsigned char ram[0x8000];
	static char outputbuffer[128];
	size_t outputpos = 0;

	gtemu_init(&gt, rom, sizeof(rom), ram, sizeof(ram));

	randstate = time(0);
	randstate = gtemu_randomizemem(randstate, rom, sizeof(rom));
	randstate = gtemu_randomizemem(randstate, ram, sizeof(ram));

	if (!loadrom(argc>1 ? argv[1] : "gigatron.rom", &gt)) {
		fprintf(stderr, "failed to open ROM.\n");
		return EXIT_FAILURE;
	}

	if (SDL_Init(0) < 0) {
		fprintf(stderr, "SDL error: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	if (gtsdl_openwindow(&s, "Gigatron")) {
		gtemu_initperiph(&ph, gtsdl_getaudiofreq(&s), randstate);
		gtserialout_setbuffer(&ph, outputbuffer,
			sizeof(outputbuffer), &outputpos);
		for (;;) {
			SDL_Event ev;
			int hasevent = gtsdl_runuiframe(&s, &gt, &ph, &ev);
			if (outputpos > 0) {
				fwrite(outputbuffer, sizeof(outputbuffer[0]),
					outputpos, stdout);
				outputpos = 0;
			}
			if (hasevent == 0) {
				continue;
			}
			if (ev.type == SDL_QUIT) {
				break;
			}
			if (ev.type == SDL_KEYDOWN) {
				if (ev.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
					break;
				}
				if (onkeydown(&gt, &ph, &ev.key)) {
					continue;
				}
			}
			gtsdl_handleevent(&s, &gt, &ev);
		}
	} else {
		fprintf(stderr, "SDL error: %s\n", SDL_GetError());
	}

	gtsdl_close(&s);

	SDL_Quit();

	return EXIT_SUCCESS;
}

