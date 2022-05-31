#include <stdio.h>
#include <string.h>
#include <gigatron/sys.h>
#include <gigatron/console.h>


char has_expander;
char has_512krom;
char has_64k;
char has_128k;
char has_512k;
int  memsize;

#define videoModeB (*(char*)0xa)
#define hSysArgs4  (*(char*)(0x8000u+(unsigned)&sysArgs4))

#define VERBOSE 1

void probe_ram()
{
	has_expander = (ctrlBits_v5 != 0);
	has_512krom =  (videoModeB ^ 0xfc ==0xfc);
	has_64k =      (memSize == 0);

	// double check expander (reset memory detection bug)
	if (has_expander) {
		SYS_ExpanderControl(0x3c);    // set bank 0
		hSysArgs4 = sysArgs4 ^ 0xaa;
		if (hSysArgs4 != sysArgs4)
			has_expander = 0;
		hSysArgs4 = sysArgs4 ^ 0xaa;
		SYS_ExpanderControl(0x7c);    // back to default
	}
	
	// check for 128k
	if (has_expander) {
		SYS_ExpanderControl(0xbc);    // set bank 2
		hSysArgs4 = sysArgs4 ^ 0xaa;
		if (hSysArgs4 != sysArgs4)
			has_128k = 1;
		hSysArgs4 = sysArgs4 ^ 0xaa;
		SYS_ExpanderControl(0x7c);    // back to default
	}
	// check for 512k
	if (has_expander && has_512krom) { // only!
		SYS_ExpanderControl(0xf8f0); // select bank 15
		hSysArgs4 = 0xaa;
		SYS_ExpanderControl(0x58f0); // select bank 5
		hSysArgs4 = 0x55;
		SYS_ExpanderControl(0xf8f0); // select bank 15
		has_512k = (hSysArgs4 == 0xaa);
		SYS_ExpanderControl(0x00f0); // back to default
	}

	if (has_512k)
		memsize = 512;
	else if (has_128k)
		memsize = 128;
	else if (has_64k)
		memsize = 64;
	else
		memsize = memSize / 4;
}


#define blksize 64

void fill_upper(char seed, int bank)
{
	register unsigned int i;
	for (i=0x8000; i!=0; i += blksize) {
		register char v = seed ^ (i >> 4) ^ bank;
		memset((char*)i, v, blksize);
	}
}

void test_upper(char seed,  int bank)
{
	register unsigned int i, j;
	for (i=0x8000; i!=0; i += blksize) { // too slow
		register char v = seed ^ (i >> 4) ^ bank;
		register char *p = (char*)i;
		for (j=blksize; j; j--,p++)
			if (*p != v)
				cprintf("\n\t[err %04x @bank%d] ", p, bank);
	}
}

void set_bank(char bank)
{
	if (has_512k && (bank == 0 || bank >= 4)) {
		// map desired bank onto classic bank 0
		SYS_ExpanderControl(0x00f0 | ((bank & 15) << 12));
		bank = 0;
	}
	if (has_expander) {
		// set classic bank
		SYS_ExpanderControl(0x3c | ((bank & 3) << 6));
	}
}


void test(int seed)
{
	register int bank;
	cprintf("- Not testing bank 0");
	if (memsize > 32) {
		cprintf("\n- Filling bank:    ");
		for (bank = 1;  (bank << 5) < memsize; bank++) { 
			cprintf("\b\b\b\b%4d", bank);
			set_bank(bank);
			fill_upper(seed, bank);
		}
		cprintf("\n- Testing bank:    ");
		for (bank = 1;  (bank << 5) < memsize; bank++) { 
			cprintf("\b\b\b\b%4d", bank);
			set_bank(bank);
			test_upper(seed, bank);
		}
	}		
	cprintf("\n", bank);
}

int main()
{
	probe_ram();
	cprintf("Expansion  : %s\n", (has_expander) ? "yes" : "no");
	cprintf("Memory size: %dK\n\n", memsize);
	test(0xaa);
	test(0x55);
	cprintf("Done\n");
	return 0;
}
