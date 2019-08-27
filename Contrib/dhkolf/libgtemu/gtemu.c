/*

    This file is part of libgtemu, a library for Gigatron emulation.
    Copyright (C) 2019 David Heiko Kolf

    Published under the BSD-2-Clause license.
    https://opensource.org/licenses/BSD-2-Clause

*/

#include "gtemu.h"

extern void gtloader_onfallingvsync(struct GTState *gt, struct GTPeriph *ph);
extern void gtloader_onrisinghsync (struct GTState *gt, struct GTPeriph *ph);

const int hbackporch = 48 / 4;
const int screenwidth = 640 / 4;
const int vbackporch = 33;
const int screenheight = 480;

static unsigned long xorshift32 (unsigned long x)
{
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	return x;
}

void gtemu_init (struct GTState *gt,
	struct GTRomEntry *rom, size_t romsize,
	unsigned char *ram, size_t ramsize)
{
	gt->rom = rom;
	gt->romcount = romsize / sizeof(rom[0]);
	gt->ram = ram;
	gt->rammask = ramsize - 1;
	gt->pc = 0;
	gt->in = 0xff;
	gt->out = 0x80;
}

unsigned long gtemu_randomizemem (unsigned long seed,
	void *mem, size_t size)
{
	int *imem = (int *) mem;
	int i;
	if (seed == 0) {
		/* 0 is an invalid value for xorshift */
		seed = 0x74007400;
	}
	for (i = 0; i < size / sizeof(int); i++) {
		imem[i] = seed = xorshift32(seed);
	}
	return seed;
}

void gtemu_initperiph (struct GTPeriph *ph, int audiofreq,
	unsigned long randseed)
{
	ph->board.booted = 0;
	if (randseed == 0) {
		/* 0 is an invalid value for xorshift */
		randseed = 0x74007400;
	}
	ph->board.undef = xorshift32(randseed);
	ph->board.clock = 0;
	ph->board.xout = 0;
	ph->video.x = ph->video.y = 1000;
	ph->audio.sampleticks1k = 6251940000ull / audiofreq;
	ph->audio.sampletickscount1k = 0;
	ph->loader.state = 0;
	ph->serialout.invpulse = 0;
	ph->serialout.bitcount = 0;
	ph->serialout.buffer = NULL;
	ph->serialout.buffersize = 0;
	ph->serialout.bufferpos = NULL;
	ph->serialout.sentfull = 0;
}

unsigned long long gtemu_getclock (struct GTPeriph *ph)
{
	return ph->board.clock;
}

unsigned char gtemu_getxout (struct GTPeriph *ph)
{
	return ph->board.xout;
}

static void cputick (struct GTState *gt, unsigned char undef)
{
	unsigned char ir, d, databus, ac;
	int ins, mod, bus, pc;
	int iswrite, isjump;
	unsigned char lo, hi = 0;
	unsigned char *to = NULL;
	size_t addr;

	ir = gt->ir;
	d = gt->d;

	pc = gt->pc;

	if (pc >= gt->romcount) {
		pc = 0;
	}

	gt->ir = gt->rom[pc].i;
	gt->d = gt->rom[pc].d;

	ins = ir >> 5;
	mod = (ir >> 2) & 7;
	bus = ir & 3;

	iswrite = ins == 6;
	isjump = ins == 7;

	lo = d;

	if (!isjump) {
		switch (mod) {
		case 0:
		case 1:
		case 2:
		case 3:
			if (!iswrite) {
				to = &gt->ac;
			}
			break;
		case 4:
			to = &gt->x;
			break;
		case 5:
			to = &gt->y;
			break;
		case 6:
		case 7:
			if (!iswrite) {
				to = &gt->out;
			}
			break;
		}
		switch (mod) {
		case 1:
			lo = gt->x;
			break;
		case 2:
			hi = gt->y;
			break;
		case 3:
			lo = gt->x;
			hi = gt->y;
			break;
		case 7:
			lo = gt->x;
			hi = gt->y;
			gt->x++;
			break;
		}
	}
	addr = ((hi << 8) | lo) & gt->rammask;

	switch (bus) {
	case 0:
		databus = d;
		break;
	case 1:
		if (iswrite) {
			databus = undef;
		} else {
			databus = gt->ram[addr];
		}
		break;
	case 2:
		databus = gt->ac;
		break;
	case 3:
		databus = gt->in;
		break;
	}

	if (iswrite) {
		gt->ram[addr] = databus;
	}

	ac = gt->ac;

	if (to != NULL) {
		switch (ins) {
		case 0:
			*to = databus;
			break;
		case 1:
			*to = ac & databus;
			break;
		case 2:
			*to = ac | databus;
			break;
		case 3:
			*to = ac ^ databus;
			break;
		case 4:
			*to = ac + databus;
			break;
		case 5:
			*to = ac - databus;
			break;
		case 6:
			*to = ac;
			break;
		case 7:
			*to = -ac;
			break;
		}
	}

	if (isjump) {
		if (mod != 0) {
			int cond = ac == 0 ? 2 : ac >> 7;
			if (mod & (1 << cond)) {
				gt->pc = (pc & 0xff00) | databus;
			} else {
				gt->pc = pc + 1;
			}
		} else {
			gt->pc = (gt->y << 8) | databus;
		}
	} else {
		gt->pc = pc + 1;
	}
}

static void gtserialout_onfallingvsync(struct GTPeriph *ph)
{
	ph->serialout.invpulse = 1;
	ph->serialout.count = 0;
}

static void gtserialout_onrisinghsync (struct GTPeriph *ph)
{
	if (ph->serialout.invpulse) {
		ph->serialout.count++;
	}
}

static void writechar (struct GTPeriph *ph, char c)
{
	if (ph->serialout.buffer != NULL &&
		ph->serialout.bufferpos != NULL) {

		size_t pos = *ph->serialout.bufferpos;
		if (pos < ph->serialout.buffersize) {
			ph->serialout.buffer[pos] = ph->serialout.outchar;
			pos++;
			*ph->serialout.bufferpos = pos;
		}
	}
}

static void gtserialout_onrisingvsync (struct GTState *gt,
	struct GTPeriph *ph)
{
	ph->serialout.invpulse = 0;
	if (ph->serialout.count == 9) {
		ph->serialout.outchar =
			0x80 | (ph->serialout.outchar >> 1);
		ph->serialout.bitcount++;
	} else if (ph->serialout.count == 7) {
		ph->serialout.outchar =
			0 | (ph->serialout.outchar >> 1);
		ph->serialout.bitcount++;
	} else {
		ph->serialout.bitcount = 0;
	}
	if (ph->serialout.bitcount > 0 && (ph->serialout.buffer == NULL ||
		ph->serialout.bufferpos == NULL ||
		*ph->serialout.bufferpos >= ph->serialout.buffersize)) {

		gt->in = 0x03;
		ph->serialout.sentfull = 1;
	} else if (ph->serialout.sentfull) {
		gt->in = 0xff;
		ph->serialout.sentfull = 0;
	}
	if (ph->serialout.bitcount == 8) {
		ph->serialout.bitcount = 0;
		writechar(ph, (char) ph->serialout.outchar);
	}
}

void gtserialout_setbuffer (struct GTPeriph *ph, char *buffer,
	size_t buffersize, size_t *bufferpos)
{
	ph->serialout.buffer = buffer;
	ph->serialout.buffersize = buffersize;
	ph->serialout.bufferpos = bufferpos;
}

int gtemu_processtick (struct GTState *gt, struct GTPeriph *ph)
{
	unsigned char prevout, risingout, fallingout;

	prevout = gt->out;

	cputick(gt, ph->board.undef);

	risingout = ~prevout & gt->out;
	fallingout = prevout & ~gt->out;

	if (!ph->board.booted) {
		gt->pc = 0;
		ph->board.booted = 1;
	}

	ph->board.clock++;
	ph->video.x++;
	ph->audio.sampletickscount1k += 1000;

	if (risingout & 0x40) {
		/* hsync */
		unsigned char ac = gt->ac;
		unsigned short sample8 = (ac & 0xf0) | (ac >> 4);
		/* only use 15 bit, sometimes SDL selects signed sound */
		ph->audio.sample = sample8 << 7 | sample8 >> 1;

		ph->board.xout = ac;

		ph->video.x = -hbackporch;
		ph->video.y++;

		gtloader_onrisinghsync(gt, ph);
		gtserialout_onrisinghsync(ph);
	}
	if (risingout & 0x80) {
		/* vsync */
		ph->video.y = -vbackporch + 5;
		ph->board.undef = xorshift32(ph->board.undef);

		gtserialout_onrisingvsync(gt, ph);
	}
	if (fallingout & 0x80) {
		gtloader_onfallingvsync(gt, ph);
		gtserialout_onfallingvsync(ph);
	}
	return risingout & 0xc0;
}

static const unsigned long long colors[] = {
/* a single value spans four pixels, each in SDL_PIXELFORMAT_RGB444 */
0x0000000000000000, 0x0500050005000500, 0x0a000a000a000a00, 0x0f000f000f000f00,
0x0050005000500050, 0x0550055005500550, 0x0a500a500a500a50, 0x0f500f500f500f50,
0x00a000a000a000a0, 0x05a005a005a005a0, 0x0aa00aa00aa00aa0, 0x0fa00fa00fa00fa0,
0x00f000f000f000f0, 0x05f005f005f005f0, 0x0af00af00af00af0, 0x0ff00ff00ff00ff0,
0x0005000500050005, 0x0505050505050505, 0x0a050a050a050a05, 0x0f050f050f050f05,
0x0055005500550055, 0x0555055505550555, 0x0a550a550a550a55, 0x0f550f550f550f55,
0x00a500a500a500a5, 0x05a505a505a505a5, 0x0aa50aa50aa50aa5, 0x0fa50fa50fa50fa5,
0x00f500f500f500f5, 0x05f505f505f505f5, 0x0af50af50af50af5, 0x0ff50ff50ff50ff5,
0x000a000a000a000a, 0x050a050a050a050a, 0x0a0a0a0a0a0a0a0a, 0x0f0a0f0a0f0a0f0a,
0x005a005a005a005a, 0x055a055a055a055a, 0x0a5a0a5a0a5a0a5a, 0x0f5a0f5a0f5a0f5a,
0x00aa00aa00aa00aa, 0x05aa05aa05aa05aa, 0x0aaa0aaa0aaa0aaa, 0x0faa0faa0faa0faa,
0x00fa00fa00fa00fa, 0x05fa05fa05fa05fa, 0x0afa0afa0afa0afa, 0x0ffa0ffa0ffa0ffa,
0x000f000f000f000f, 0x050f050f050f050f, 0x0a0f0a0f0a0f0a0f, 0x0f0f0f0f0f0f0f0f,
0x005f005f005f005f, 0x055f055f055f055f, 0x0a5f0a5f0a5f0a5f, 0x0f5f0f5f0f5f0f5f,
0x00af00af00af00af, 0x05af05af05af05af, 0x0aaf0aaf0aaf0aaf, 0x0faf0faf0faf0faf,
0x00ff00ff00ff00ff, 0x05ff05ff05ff05ff, 0x0aff0aff0aff0aff, 0x0fff0fff0fff0fff
};

const unsigned long long undefcolor = 0x0333066604440777;

int gtemu_processscreen (struct GTState *gt, struct GTPeriph *ph,
	void *pixels, int pitch, unsigned short *samples, size_t maxsamples,
	size_t *nsamples)
{
	unsigned long long *line = NULL;
	int x = ph->video.x, y = ph->video.y;
	int screenticks;
	int s = *nsamples;

	for (screenticks = 0; screenticks < 110000; screenticks++) {
		unsigned char risingout;

		risingout = gtemu_processtick(gt, ph);

		if (risingout & 0x80) {
			/* vsync */
			break;
		}
		x++;
		if (risingout & 0x40) {
			/* hsync */
			if (line != NULL) {
				if (x < 0) x = 0;
				for (; x < screenwidth; x++) {
					line[x] = undefcolor;
				}
			}
			x = ph->video.x;
			y = ph->video.y;
			if (y >= 0 && y < screenheight && pixels != NULL) {
				line = (unsigned long long *)
					(pixels + y * pitch);
			} else {
				line = NULL;
			}
		}
		if (line != NULL && x >= 0 && x < screenwidth) {
			line[x] = colors[gt->out & 0x3f];
		}
		if (ph->audio.sampletickscount1k >= ph->audio.sampleticks1k) {
			ph->audio.sampletickscount1k -= ph->audio.sampleticks1k;
			if (s < maxsamples && samples != NULL) {
				samples[s] = ph->audio.sample;
				s++;
			}
		}
	}

	*nsamples = s;

	if (pixels != NULL) {
		if (y < 0) y = 0;
		for (; y < screenheight; y++) {
			line = (unsigned long long *) (pixels + y * pitch);
			for (x = 0; x < 160; x++) {
				line[x] = undefcolor;
			}
		}
	}

	return screenticks;
}

void gtemu_placelights (struct GTPeriph *ph, void *pixels, int pitch,
	int power)
{
	int y;
	for (y = 474; y < 478; y++) {
		int i, j, x0;
		unsigned short c;
		unsigned short *line = (unsigned short *)
			(pixels + y * pitch);
		int xout = gtemu_getxout(ph);
		for (i = 0; i < 4; i++) {
			c = xout&1 ? 0x0f42 : 0x222;
			xout >>= 1;
			x0 = i * 8 + 6;
			for (j = x0; j < x0 + 4; j++) {
				line[j] = c;
			}
		}
		c = power ? 0x0f42 : 0x222;
		x0 = 632;
		for (j = x0; j < x0 + 4; j++) {
			line[j] = c;
		}
	}
}

