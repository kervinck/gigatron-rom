/*

    This file is part of libgtemu, a library for Gigatron emulation.
    Copyright (C) 2019 David Heiko Kolf

    Published under the BSD-2-Clause license.
    https://opensource.org/licenses/BSD-2-Clause

*/

#include "gtemu.h"

const char loaderframe = 'L';
const unsigned char initchecksum = (unsigned char) 'g';
const char maxframesize = 60;

enum LoaderState {
	L_IDLE = 0,
	L1_INIT_FV,
	L2_INIT_RH,
	L3_HEADER_RH,
	L4_PAYLOAD_RH,
	L5_FILLER_RH,
	L6_CHECKSUM_RH,
	L7_FINISH_RH,
	LT1_KEYUP_FV,
	LT2_KEYDOWN_FV
};

static void sendbit (struct GTState *gt, struct GTPeriph *ph,
	unsigned char value, int n)
{
	gt->in = gt->in << 1 | (1 & (value >> (7 - n)));
	ph->loader.bitcount++;
}

static void loadnextsegment (struct GTPeriph *ph)
{
	ph->loader.addr = (ph->loader.data[0] << 8) | ph->loader.data[1];
	if (ph->loader.data[2] != 0) {
		ph->loader.remainingsegment = ph->loader.data[2];
	} else {
		ph->loader.remainingsegment = 256;
	}
	ph->loader.remainingdata -= 3;
	ph->loader.data += 3;
	if (ph->loader.remainingsegment > ph->loader.remainingdata) {
		ph->loader.remainingsegment = ph->loader.remainingdata;
	}
	if (ph->loader.remainingsegment > maxframesize) {
		ph->loader.remainingframe = maxframesize;
	} else {
		ph->loader.remainingframe = ph->loader.remainingsegment;
	}
	ph->loader.state = L1_INIT_FV;
}

static void sendheaderbit (struct GTState *gt, struct GTPeriph *ph)
{
	int bc = ph->loader.bitcount;
	if (bc < 8) {
		sendbit(gt, ph, loaderframe, bc);
	} else if (bc < 14) {
		sendbit(gt, ph, ph->loader.remainingframe, bc - 6);
	} else if (bc < 22) {
		sendbit(gt, ph, (unsigned char) ph->loader.addr, bc - 14);
	} else {
		sendbit(gt, ph, (unsigned char) (ph->loader.addr >> 8), bc - 22);
		if (bc + 1 >= 30) {
			ph->loader.checksum += loaderframe +
				(loaderframe << 6) +
				ph->loader.remainingframe +
				ph->loader.addr +
				(ph->loader.addr >> 8);
			ph->loader.bitcount = 0;
			ph->loader.remainingfiller = maxframesize - ph->loader.remainingframe;
			if (ph->loader.remainingframe > 0) {
				ph->loader.addr += ph->loader.remainingframe;
				ph->loader.state = L4_PAYLOAD_RH;
			} else {
				ph->loader.state = L5_FILLER_RH;
			}
		}
	}
}

static void sendpayloadbit (struct GTState *gt, struct GTPeriph *ph)
{
	int bc = ph->loader.bitcount;
	sendbit(gt, ph, ph->loader.data[0], bc);
	if (bc + 1 >= 8) {
		ph->loader.checksum += ph->loader.data[0];
		ph->loader.bitcount = 0;
		ph->loader.remainingframe--;
		ph->loader.remainingsegment--;
		ph->loader.remainingdata--;
		ph->loader.data++;
		if (ph->loader.remainingframe == 0) {
			if (ph->loader.remainingfiller > 0) {
				ph->loader.state = L5_FILLER_RH;
			} else {
				ph->loader.checksum = -ph->loader.checksum;
				ph->loader.state = L6_CHECKSUM_RH;
			}
		}
	}
}

static void sendfillerbit (struct GTState *gt, struct GTPeriph *ph)
{
	int bc = ph->loader.bitcount;
	sendbit(gt, ph, 0, bc);
	if (bc + 1 >= 8) {
		ph->loader.bitcount = 0;
		ph->loader.remainingfiller--;
		if (ph->loader.remainingfiller == 0) {
			ph->loader.checksum = -ph->loader.checksum;
			ph->loader.state = L6_CHECKSUM_RH;
		}
	}
}

static void sendchecksumbit (struct GTState *gt, struct GTPeriph *ph)
{
	int bc = ph->loader.bitcount;
	sendbit(gt, ph, ph->loader.checksum, bc);
	if (bc + 1 >= 8) {
		if (ph->loader.remainingsegment > 0) {
			if (ph->loader.remainingsegment > maxframesize) {
				ph->loader.remainingframe = maxframesize;
			} else {
				ph->loader.remainingframe = (unsigned char) ph->loader.remainingsegment;
			}
			ph->loader.state = L1_INIT_FV;
		} else if (ph->loader.remainingdata > 2) {
			if (ph->loader.data[0] == 0) {
				ph->loader.addr = (ph->loader.data[1] << 8) | ph->loader.data[2];
				ph->loader.remainingdata = 0;
				ph->loader.state = L1_INIT_FV;
			} else {
				loadnextsegment(ph);
			}
		} else {
			ph->loader.state = L7_FINISH_RH;
		}
	}
}

static void sendtextrelease (struct GTState *gt, struct GTPeriph *ph)
{
	ph->loader.remainingframe--;
	if (ph->loader.remainingframe == 0) {
		ph->loader.remainingframe = 3;
		gt->in = 0xff;
		if (ph->loader.remainingdata > 0) {
			ph->loader.state = LT2_KEYDOWN_FV;
		} else {
			ph->loader.state = L_IDLE;
		}
	}
}

static void sendtextbyte (struct GTState *gt, struct GTPeriph *ph)
{
	ph->loader.remainingframe--;
	if (ph->loader.remainingframe == 0) {
		gt->in = ph->loader.data[0];
		if (gt->in == '\n') {
			ph->loader.remainingframe = 9;
		} else {
			ph->loader.remainingframe = 3;
		}
		ph->loader.data++;
		ph->loader.remainingdata--;
		ph->loader.state = LT1_KEYUP_FV;
	}
}

void gtloader_onfallingvsync (struct GTState *gt, struct GTPeriph *ph)
{
	switch (ph->loader.state) {
	case L1_INIT_FV:
		ph->loader.state = L2_INIT_RH;
		return;
	case LT1_KEYUP_FV:
		sendtextrelease(gt, ph);
		return;
	case LT2_KEYDOWN_FV:
		sendtextbyte(gt, ph);
		return;
	default:
		return;
	}
}

void gtloader_onrisinghsync (struct GTState *gt, struct GTPeriph *ph)
{
	switch (ph->loader.state) {
	case L2_INIT_RH:
		ph->loader.bitcount = 0;
		ph->loader.state = L3_HEADER_RH;
		return;
	case L3_HEADER_RH:
		sendheaderbit(gt, ph);
		return;
	case L4_PAYLOAD_RH:
		sendpayloadbit(gt, ph);
		return;
	case L5_FILLER_RH:
		sendfillerbit(gt, ph);
		return;
	case L6_CHECKSUM_RH:
		sendchecksumbit(gt, ph);
		return;
	case L7_FINISH_RH:
		gt->in = 0xff;
		ph->loader.state = L_IDLE;
		return;
	}
}

int gtloader_sendgt1 (struct GTPeriph *ph,
	const char *data, size_t datasize)
{
	if (ph->loader.state != L_IDLE) {
		return 0;
	}
	if (datasize < 3) {
		return -1;
	}

	ph->loader.data = (const unsigned char *) data;
	ph->loader.remainingdata = datasize;
	ph->loader.checksum = initchecksum;

	loadnextsegment(ph);

	return 1;
}

int gtloader_isactive (struct GTPeriph *ph)
{
	return ph->loader.state != L_IDLE;
}

int gtloader_validategt1 (const char *data, size_t datasize)
{
	const unsigned char *ud = (const unsigned char *) data;
	unsigned char adhi;
	if (datasize < 3) {
		return 0;
	}
	adhi = ud[0];
	do {
		unsigned char adlo = ud[1];
		int size = ud[2];
		if (size == 0) {
			size = 256;
		}
		ud += 3;
		datasize -= 3;
		if (size + 3 > datasize) {
			return 0;
		}
		if (size + adlo > 256) {
			return 0;
		}
		ud += size;
		datasize -= size;
		adhi = ud[0];
	} while (adhi != 0 && datasize > 3);
	if (datasize < 3) {
		return 0;
	}

	return 1;
}

int gtloader_sendtext (struct GTPeriph *ph,
	const char *data, size_t datasize)
{
	if (ph->loader.state != L_IDLE) {
		return 0;
	}

	ph->loader.data = (const unsigned char *) data;
	ph->loader.remainingdata = datasize;
	ph->loader.remainingframe = 3;

	ph->loader.state = LT1_KEYUP_FV;

	return 1;
}

