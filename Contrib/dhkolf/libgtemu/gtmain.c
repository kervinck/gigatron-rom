/*

    This file is part of libgtemu, a library for Gigatron emulation.
    Copyright (C) 2019 David Heiko Kolf

    Published under the BSD-2-Clause license.
    https://opensource.org/licenses/BSD-2-Clause

*/

#include <stdio.h>
#include <errno.h>
#include <time.h>

#include "gtsdl.h"

struct MainState {
	const char *romfile;
	const char *sendfile;
	int displayhelp;
	char *sendbuffer;
	size_t sendbuffersize;
};

static int loadrom (const char *fname, struct GTState *gt)
{
	FILE *f;

	f = fopen(fname, "rb");

	if (!f) return 0;

	fread(gt->rom, sizeof(struct GTRomEntry), gt->romcount, f);

	fclose(f);

	return 1;
}

static void startloader (struct GTState *gt, struct GTPeriph *ph)
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

static void sendgt1file (struct MainState *mstate, struct GTPeriph *ph)
{
	FILE *f;
	size_t datasize;

	if (mstate->sendfile == NULL) {
		fprintf(stderr, "No file specified for sending, use "
			"the -l option to specify a file.\n");
		return;
	}

	if (gtloader_isactive(ph)) {
		/* Check first whether a file is still being sent, to
		   avoid changing data during the progress. */
		fprintf(stderr, "A file is already being sent.\n");
		return;
	}

	f = fopen(mstate->sendfile, "rb");

	if (f == NULL) {
		fprintf(stderr, "Failed to open %s: %s\n",
			mstate->sendfile, strerror(errno));
		return;
	}

	datasize = fread(mstate->sendbuffer, 1, mstate->sendbuffersize, f);

	if (datasize >= mstate->sendbuffersize) {
		fprintf(stderr, "File too large.\n");
		fclose(f);
		return;
	} else if (ferror(f)) {
		fprintf(stderr, "Error reading %s: %s\n",
			mstate->sendfile, strerror(errno));
		fclose(f);
		return;
	}

	fclose(f);

	if (!gtloader_validategt1(mstate->sendbuffer, datasize)) {
		fprintf(stderr, "%s is not a valid GT1 file.\n",
			mstate->sendfile);
		return;
	}

	if (!gtloader_sendgt1(ph, mstate->sendbuffer, datasize)) {
		/* Should not happen as we checked isactive earlier. */
		fprintf(stderr, "Loader peripherals are not ready.\n");
	}
}

static int onkeydown (struct MainState *mstate, struct GTState *gt,
	struct GTPeriph *ph, SDL_KeyboardEvent *ev)
{
	if (ev->keysym.sym == 'l' && ev->keysym.mod == KMOD_LALT) {
		startloader(gt, ph);
		return 1;
	}
	if (ev->keysym.mod == KMOD_LCTRL || ev->keysym.mod == KMOD_RCTRL) {
		if (ev->keysym.sym == SDLK_F2) {
			sendgt1file(mstate, ph);
			return 1;
		}
	}
	return 0;
}

static void parseargs (int argc, char *argv[], struct MainState *a)
{
	int i;
	a->romfile = NULL;
	a->sendfile = NULL;
	a->displayhelp = 0;
	for (i = 1; i < argc; i++) {
		if (argv[i][0] != '-') {
			break;
		}
		switch(argv[i][1]) {
		case '-':
			if (argv[i][2] != '\0') {
				/* no long options yet */
				a->displayhelp = 1;
				return;
			}
			i++;
			break;
		case 'h':
			a->displayhelp = 1;
			break;
		case 'l':
			if (argv[i][2] != '\0') {
				a->sendfile = argv[i] + 2;
				break;
			}
			i++;
			if (i >= argc) {
				/* no file name given */
				a->displayhelp = 1;
				return;
			}
			if (argv[i][0] == '-') {
				/* another option */
				a->displayhelp = 1;
				return;
			}
			a->sendfile = argv[i];
			break;
		default:
			a->displayhelp = 1;
			return;
		}
	}
	if (i < argc) {
		a->romfile = argv[i];
	}
}

static void displayhelp (const char *progname)
{
	if (progname == NULL) {
		progname = "gtemu";
	}
	fprintf(stderr,
		"usage: %s [-h] [-l filename.gt1] [filename.rom]\n"
                "\n"
		"Arguments:\n"
                " -h               Display this help.\n"
                " -l filename.gt1  File to be sent with Ctrl-F2.\n"
                "    filename.rom  ROM file (default name: gigatron.rom).\n"
		"\n"
		"Special keys:\n"
		"    Ctrl-F2       Send designated GT1 file.\n"
		"    Alt-L         Perform hard reset and select loader.\n"
		"    ESC           Close the emulation.\n",
		progname);
}

int main (int argc, char *argv[])
{
	struct MainState mstate;
	struct GTSDLState s;
	struct GTState gt;
	struct GTPeriph ph;
	unsigned long randstate;
	size_t outputpos = 0;

	static struct GTRomEntry rom[0x10000];
	static unsigned char ram[0x8000];
	static char sendbuffer[0x11000];
	static char outputbuffer[128];

	mstate.sendbuffer = sendbuffer;
	mstate.sendbuffersize = sizeof(sendbuffer);

	parseargs(argc, argv, &mstate);

	if (mstate.displayhelp) {
		displayhelp(argc > 0 ? argv[0] : NULL);
		return EXIT_FAILURE;
	}

	gtemu_init(&gt, rom, sizeof(rom), ram, sizeof(ram));

	randstate = time(0);
	randstate = gtemu_randomizemem(randstate, rom, sizeof(rom));
	randstate = gtemu_randomizemem(randstate, ram, sizeof(ram));

	if (!loadrom(mstate.romfile != NULL ? mstate.romfile :
		"gigatron.rom", &gt)) {

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
				if (onkeydown(&mstate, &gt, &ph, &ev.key)) {
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

