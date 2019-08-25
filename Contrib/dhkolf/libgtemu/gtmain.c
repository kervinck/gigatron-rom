/*

    This file is part of libgtemu, a library for Gigatron emulation.
    Copyright (C) 2019 David Heiko Kolf

    Published under the BSD-2-Clause license.
    https://opensource.org/licenses/BSD-2-Clause

*/

#include <stdio.h>
#include <time.h>

#include "gtsdl.h"

static void randfill (void *mem, size_t size)
{
	int *imem = (int *) mem;
	int i;
	for (i = 0; i < size / sizeof(int); i++) {
		imem[i] = rand();
	}
}

static int loadrom (const char *fname, struct GTState *gt)
{
	FILE *f;

	f = fopen(fname, "rb");

	if (!f) return 0;

	fread(gt->rom, sizeof(struct GTRomEntry), gt->romcount, f);

	fclose(f);

	return 1;
}

int main (int argc, char *argv[])
{
	struct GTSDLState s;
	struct GTState gt;
	struct GTPeriph ph;

	static struct GTRomEntry rom[0x10000];
	static unsigned char ram[0x8000];
	static char outputbuffer[128];
	size_t outputpos = 0;

	gtemu_init(&gt, rom, sizeof(rom), ram, sizeof(ram));

	srand(time(0));
	randfill(rom, sizeof(rom));
	randfill(ram, sizeof(ram));

	if (!loadrom(argc>1 ? argv[1] : "gigatron.rom", &gt)) {
		fprintf(stderr, "failed to open ROM.\n");
		return EXIT_FAILURE;
	}

	if (SDL_Init(0) < 0) {
		fprintf(stderr, "SDL error: %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	if (gtsdl_openwindow(&s, "Gigatron")) {
		gtemu_initperiph(&ph, gtsdl_getaudiofreq(&s));
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
			if (ev.type == SDL_KEYDOWN &&
				ev.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {

				break;
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

