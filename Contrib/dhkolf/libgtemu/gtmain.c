/*

    This file is part of libgtemu, a library for Gigatron emulation.
    Copyright (C) 2019 David Heiko Kolf

    Published under the BSD-2-Clause license.
    https://opensource.org/licenses/BSD-2-Clause

*/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "gtsdl.h"

struct MainState {
	const char *romfile;
	const char *sendfile;
	const char *textfile;
	int displayhelp;
	int ramexpansion;
	char *sendbuffer;
	size_t sendbuffersize;
};

static int loadfile (const char *fname, void *buffer, size_t elementsize,
	size_t elementcount, size_t *readcount)
{
	FILE *f = fopen(fname, "rb");

	if (f == NULL) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
			fname, strerror(errno), NULL);
		return 0;
	}

	*readcount = fread(buffer, elementsize, elementcount, f);

	if (*readcount >= elementcount) {
		char dummy;
		/* to check for EOF, try to read a further byte */
		if (fread(&dummy, 1, 1, f) > 0) {
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
				fname, "File is too large.", NULL);
			fclose(f);
			return 0;
		}
	} else if (ferror(f)) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
			fname, strerror(errno), NULL);
		fclose(f);
		return 0;
	}

	fclose(f);

	return 1;
}

static int loadrom (const char *fname, struct GTState *gt)
{
	size_t datasize;
	if (!loadfile(fname, gt->rom,
		sizeof(struct GTRomEntry), gt->romcount, &datasize)) {
		return 0;
	}
	return 1;
}

static void startloader (struct GTState *gt, struct GTPeriph *ph, int wait)
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
	if (wait) {
		/* wait until vPC is in the screen page 0x5a */
		while (gtemu_getclock(ph) < timeout && ram[0x17] < 0x5a) {
			gtemu_processtick(gt, ph);
		}
	}
}

static void sendgt1file (struct MainState *mstate, struct GTState *gt,
	struct GTPeriph *ph, int restart)
{
	size_t datasize;

	if (mstate->sendfile == NULL) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION,
			"Sending GT1 program",
			"No file specified for sending, use "
			"the -l option to specify a file.",
			NULL);
		return;
	}

	if (gtloader_isactive(ph)) {
		/* Check first whether a file is still being sent, to
		   avoid changing data during the progress. */
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING,
			"Sending GT1 program",
			"A file is already being sent.",
			NULL);
		return;
	}

	if (!loadfile(mstate->sendfile, mstate->sendbuffer,
		1, mstate->sendbuffersize, &datasize)) {
		return;
	}

	if (!gtloader_validategt1(mstate->sendbuffer, datasize)) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
			mstate->sendfile,
			"File is not a valid GT1 file.",
			NULL);
		return;
	}

	if (restart) {
		startloader(gt, ph, 1);
	}

	gtloader_sendgt1(ph, mstate->sendbuffer, datasize);
}

static void sendtextfile (struct MainState *mstate, struct GTPeriph *ph)
{
	size_t datasize;

	if (mstate->textfile == NULL) {
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION,
			"Sending text",
			"No file specified for sending, use "
			"the -t option to specify a file.",
			NULL);
		return;
	}

	if (gtloader_isactive(ph)) {
		/* Check first whether a file is still being sent, to
		   avoid changing data during the progress. */
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING,
			"Sending text",
			"A file is already being sent.",
			NULL);
		return;
	}

	if (!loadfile(mstate->textfile, mstate->sendbuffer,
		1, mstate->sendbuffersize, &datasize)) {
		return;
	}

	gtloader_sendtext(ph, mstate->sendbuffer, datasize);
}

static int onkeydown (struct MainState *mstate, struct GTState *gt,
	struct GTPeriph *ph, SDL_KeyboardEvent *ev)
{
	if (ev->keysym.mod == KMOD_LALT) {
		switch (ev->keysym.sym) {
		case 'l':
			if (!ev->repeat) {
				startloader(gt, ph, 0);
			}
			return 1;
		case 'x':
			if (!ev->repeat) {
				sendgt1file(mstate, gt, ph, 1);
			}
			return 1;
		default:
			return 0;
		}
	}
	if (ev->keysym.mod == KMOD_LCTRL || ev->keysym.mod == KMOD_RCTRL) {
		switch (ev->keysym.sym) {
		case SDLK_F2:
			if (!ev->repeat) {
				sendgt1file(mstate, gt, ph, 0);
			}
			return 1;
		case SDLK_F3:
			if (!ev->repeat) {
				sendtextfile(mstate, ph);
			}
			return 1;
		default:
			return 0;
		}
	}
	return 0;
}

static void ondroppedfile (struct MainState *mstate, struct GTState *gt,
	struct GTPeriph *ph, const char *fname)
{
	size_t datasize;

	if (gtloader_isactive(ph)) {
		/* Check first whether a file is still being sent, to
		   avoid changing data during the progress. */
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING,
			fname,
			"A file is already being sent.",
			NULL);
		return;
	}

	if (!loadfile(fname, mstate->sendbuffer,
		1, mstate->sendbuffersize, &datasize)) {
		return;
	}

	if (gtloader_validategt1(mstate->sendbuffer, datasize)) {
		startloader(gt, ph, 1);
		gtloader_sendgt1(ph, mstate->sendbuffer, datasize);
	} else {
		gtloader_sendtext(ph, mstate->sendbuffer, datasize);
	}

}

static void parseargs (int argc, char *argv[], struct MainState *a)
{
	int i;
	a->romfile = NULL;
	a->sendfile = NULL;
	a->textfile = NULL;
	a->displayhelp = 0;
	a->ramexpansion = 0;
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
		case 't':
			if (argv[i][2] != '\0') {
				a->textfile = argv[i] + 2;
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
			a->textfile = argv[i];
			break;
		case '6':
			if (argv[i][2] != '4' || argv[i][3] != '\0') {
				a->displayhelp = 1;
				return;
			}
			a->ramexpansion = 1;
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
	SDL_version linkedsdl;
	SDL_GetVersion(&linkedsdl);
	if (progname == NULL) {
		progname = "gtemu";
	}
	fprintf(stderr,
		"usage: %s [-h] [-l filename.gt1] [-t filename.gtb] "
		"[-64] [filename.rom]\n"
		"\n"
		"Arguments:\n"
		" -h               Display this help.\n"
		" -l filename.gt1  GT1 program to be loaded at the start.\n"
		" -t filename.gtb  Text file to be sent with Ctrl-F3.\n"
		" -64              Expand RAM to 64k.\n"
		"    filename.rom  ROM file (default name: gigatron.rom).\n"
		"\n"
		"Special keys:\n"
		"    Ctrl-F2       Send designated GT1 file.\n"
		"    Ctrl-F3       Send designated text file.\n"
		"    Alt-L         Perform hard reset and select loader.\n"
		"    Alt-X         Perform hard reset and send GT1 file.\n"
		"    ESC           Close the emulation.\n"
		"\n"
		"libgtemu version 0.3.0, using SDL version %d.%d.%d.\n",
		progname, linkedsdl.major, linkedsdl.minor,
		linkedsdl.patch);
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
	static unsigned char ram[0x10000];
	static char sendbuffer[0x11000];
	static char outputbuffer[128];

	mstate.sendbuffer = sendbuffer;
	mstate.sendbuffersize = sizeof(sendbuffer);

	parseargs(argc, argv, &mstate);

	if (mstate.displayhelp) {
		displayhelp(argc > 0 ? argv[0] : NULL);
		return EXIT_FAILURE;
	}

	gtemu_init(&gt, rom, sizeof(rom), ram,
		mstate.ramexpansion ? 0x10000 : 0x8000);

	randstate = time(0);
	randstate = gtemu_randomizemem(randstate, rom, sizeof(rom));
	randstate = gtemu_randomizemem(randstate, ram, sizeof(ram));

	if (!loadrom(mstate.romfile != NULL ? mstate.romfile :
		"gigatron.rom", &gt)) {

		return EXIT_FAILURE;
	}

	if (SDL_Init(0) < 0) {
		const char *sdlerror = SDL_GetError();
		fprintf(stderr, "SDL error: %s\n", sdlerror);
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
			"SDL error", sdlerror, NULL);
		return EXIT_FAILURE;
	}

	if (gtsdl_openwindow(&s, "Gigatron")) {
		gtemu_initperiph(&ph, gtsdl_getaudiofreq(&s), randstate);
		gtserialout_setbuffer(&ph, outputbuffer,
			sizeof(outputbuffer), &outputpos);

		if (mstate.sendfile != NULL) {
			sendgt1file(&mstate, &gt, &ph, 1);
		}

		for (;;) {
			SDL_Event ev;
			int hasevent = gtsdl_runuiframe(&s, &gt, &ph, &ev);
			if (outputpos > 0) {
				fwrite(outputbuffer, sizeof(outputbuffer[0]),
					outputpos, stdout);
				fflush(stdout);
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
			if (ev.type == SDL_DROPFILE) {
				ondroppedfile(&mstate, &gt, &ph,
					ev.drop.file);
				SDL_free(ev.drop.file);
				continue;
			}
			gtsdl_handleevent(&s, &gt, &ev);
		}
	} else {
		const char *sdlerror = SDL_GetError();
		fprintf(stderr, "SDL error: %s\n", sdlerror);
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
			"SDL error", sdlerror, NULL);
	}

	gtsdl_close(&s);

	SDL_Quit();

	return EXIT_SUCCESS;
}

