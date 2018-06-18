// Concept tester for interfacing from the raspberry ppi
// with theGigatron TTL microcomputer using a microcontroller
// hooked to the input port (J4).
//  (adapted from ../Utils/LoaderTest/LoaderTest.ino)

// /!\ it is not very reliable. /!\
// The problem is that the raspberry pi is not a realtime system, whereas precise timing is needed to
// communicate with the gigatron.
// So:
//  - on the one hand, the program tries to hint the kernel that it really 
//        needs highpriority as-real-time-as-possible scheduling during certain periods 
//        (and try to yield control back to the system when it needs to wait, hopping that the 
//        system tasks will be scheduled at those times)
//  - on the other hand it measures the time spent during a payload frame, and try to detect if
//        it has been interrupted (sequence took longer than expected), and thus that the frame
//        is probably corrupted on the receiving end.
//  - if it thinks the frame is corrupt, reset the checksum and tries to send it again.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sched.h>
#include <time.h>

typedef unsigned char byte;
typedef int bool;

// === GPIO acess ============
// ===========================
//pins:  		 rpi2 BCM	 / rpi2 board  /  Gigatron DB9 (J4)
static int GROUND    = 0;	//	34				8
static int SER_DATA  = 12;	//	32				2 blanc 
static int SER_LATCH = 16;	//	36				3 vert 
static int SER_PULSE = 20;	//	38				4 rouge


// ------------------------------------------ 
#define BCM2708_PERI_BASE  0x20000000	//rpi1
#define BCM2709_PERI_BASE	0x3F000000	//rpi2
#define GPIO_BASE  (BCM2709_PERI_BASE + 0x200000) /* GPIO controller */
#define FSEL_OFFSET                 0   // 0x0000
#define SET_OFFSET                  7   // 0x001c / 4
#define CLR_OFFSET                  10  // 0x0028 / 4
#define PINLEVEL_OFFSET             13  // 0x0034 / 4

volatile unsigned *gpio;
void gpio_setup()
{
	int  mem_fd;
	if ((mem_fd = open("/dev/gpiomem", O_RDWR|O_SYNC) ) < 0) {
		printf("can't open /dev/gpiomem \n");
		exit(-1);
	}

	size_t BLOCK_SIZE = (4*1024);
	static void *gpio_map;
	gpio_map = mmap(NULL, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, 0);
	close(mem_fd); //No need to keep mem_fd open after mmap

	if (gpio_map == MAP_FAILED) {
		printf("mmap error %d %m\n", (int)gpio_map);
		exit(-1);
	}

	gpio = (volatile unsigned *)gpio_map;

	// try minimizing interruption causes...
	mlockall(MCL_CURRENT); // (probably useless very little mem is used)
}

void gpio_setrealtimesched(int enable)
{
	if (enable) {
		struct sched_param sp = {32};
		int err = sched_setscheduler(0, SCHED_FIFO, &sp);
		if (err!=0) {
			printf("could not switch to realtime prio: %m\n");
		}
		// nb for pthreads: pthread_setschedparam(pthread_self(), SCHED_FIFO, &sp))
	} else {
		struct sched_param sp = {0};
		int err = sched_setscheduler(0, SCHED_OTHER, &sp);
		if (err!=0) {
			printf("could not switch back to normal prio: %m\n");
		}
	}
}

void gpio_setmode_out(int pin)
{
	int offset = FSEL_OFFSET + (pin/10);
	int shift = (pin%10)*3;
	*(gpio+offset) = (*(gpio+offset) & ~(7<<shift)) | (1<<shift);
}

void gpio_setmode_in(int pin)
{
	int offset = FSEL_OFFSET + (pin/10);
	int shift = (pin%10)*3;
	*(gpio+offset) = (*(gpio+offset) & ~(7<<shift));
}

void gpio_write(int pin, int val) 
{
	int offset = val ? SET_OFFSET :  CLR_OFFSET;
	offset += (pin/32);
	int mask = 1<<(pin%32);

	*(gpio+offset) = mask;
}

bool gpio_read(int pin)
{
	int offset = PINLEVEL_OFFSET + (pin/32);
	int mask = 1<<(pin%32);
	int val = *(gpio+offset);
	return (val & mask) ? 1 : 0;
}

// === timing ============
// ===========================
static unsigned Clock()		// microsecond
{
	static struct timespec Init = {0};
	if (Init.tv_sec == 0)
		clock_gettime(CLOCK_MONOTONIC, &Init);

	struct timespec Now = {0};
	clock_gettime(CLOCK_MONOTONIC, &Now);
	
	return ((Now.tv_sec - Init.tv_sec)*1000000)+((Now.tv_nsec - Init.tv_nsec)/1000);
}

static void Delay(unsigned ms)	//milliseconds
{
	usleep(ms * 1000);
}

void Nop()
{
//	DbgTiming_ticks++;
}

// === Gigatron ============
// ===========================
static const byte buttonRight  = 1;
static const byte buttonLeft   = 2;
static const byte buttonDown   = 4;
static const byte buttonUp     = 8;
static const byte buttonStart  = 16;
static const byte buttonSelect = 32;
static const byte buttonB      = 64;
static const byte buttonA      = 128;

bool DetectGigatron()
{
	gpio_setrealtimesched(1);

	unsigned timeout = Clock() + 85*1000;

	unsigned T[4] = {0,0,0,0};
	while (Clock() < timeout) {
		int latch = gpio_read(SER_LATCH);
		int pulse = gpio_read(SER_PULSE);
		T[latch*2+pulse*1] ++;
	}

	unsigned Total = T[0] + T[1] + T[2] + T[3];
	float vSync = (float)(T[0] + T[1]) / ( 8 * Total / 521.0); // Adjusted vSync signal
	float hSync = (float)(T[0] + T[2]) / (96 * Total / 800.0); // Standard hSync signal

	gpio_setrealtimesched(0);

	printf("detection: ticks=%d  vSync=%g  hSync=%g\n", Total, vSync, hSync);
	return (0.95 <= vSync && vSync <= 1.20) && (0.95 <= hSync && hSync <= 1.05);
}

// wait vsync and send one byte
void SendFirstByte(byte val)
{
	// Wait vertical sync NEGATIVE edge to sync with loader
	while (!gpio_read(SER_LATCH)) {Nop();} 	// Ensure vSync is HIGH first
	while (gpio_read(SER_LATCH))  {Nop();}	// Then wait for vSync to drop

	unsigned mask = 0x80;
	while(mask) {
		int bit = (val&mask)?1:0;
		mask = mask >> 1;

		// Send bit
		gpio_write(SER_DATA, bit);

		// Wait for bit transfer at horizontal sync POSITIVE edge.
		// This timing is tight for the first bit of the first byte and
		while (gpio_read(SER_PULSE))  {Nop();} // Ensure hSync is LOW first
		while (!gpio_read(SER_PULSE)) {Nop();} // Then wait for hSync to rise
	}
}

void SendController(int button, int frames)
{
	gpio_setrealtimesched(1);

	// Note: The kit's controller gives inverted signals.
	for (int i=0; i<frames; i++) {
		SendFirstByte(~button);
		gpio_write(SER_DATA, 1);	// Send 1 when idle
		Delay(10);	// try to focus os interruptions out of the time sensitive code.
	}

	gpio_setrealtimesched(0);
}

void SendReset()
{
	SendController(buttonStart, 128+32);
	
	// Wait for main menu to be ready
	Delay(1500);
}


void StartLoader()
{
	// Navigate menu. 'Loader' is at the bottom
	for (int i=0; i<10; i++) {
		SendController(buttonDown, 2);
		Delay(20);
	}

	// Start 'Loader' application on Gigatron
	SendController(buttonA, 2);

	// Wait for Loader to be running
	Delay(1500);
}

//  ----- transfer -------------
static const int FramePayload = 60;
byte checksum; // Global is simplest
void ResetChecksum()
{
	checksum = 'g';
}

void SendBits(byte val, int bits)
{
	unsigned mask = 1 << (bits-1);
	while(mask) {
		int bit = (val&mask)?1:0;
		mask = mask >> 1;

		// Send bit
		gpio_write(SER_DATA, bit);

		// Wait for bit transfer at horizontal sync POSITIVE edge.
		// This timing is tight for the first bit of the first byte and
		while (gpio_read(SER_PULSE))  { Nop(); } // Ensure hSync is LOW first
		while (!gpio_read(SER_PULSE)) { Nop(); } // Then wait for hSync to rise
	}
	checksum += val;
}

bool SendFrame(byte ProtocolByte, int len, unsigned address, const byte* message)
{
	// Send one frame of data
	//
	// A frame has 65*8-2=518 bits, including protocol byte and checksum.
	// The reasons for the two "missing" bits are:
	// 1. From start of vertical pulse, there are 35 scanlines remaining
	// in vertical blank. But we also need the payload bytes to align
	// with scanlines where the interpreter runs, so the Gigatron doesn't
	// have to shift everything it receives by 1 bit.
	// 2. There is a 1 bit latency inside the 74HCT595 for the data bit,
	// but (obviously) not for the sync signals.
	// All together, we drop 2 bits from the 2nd byte in a frame. This achieves
	// byte alignment for the Gigatron at visible scanline 3, 11, 19, ... etc.

	// Wait vertical sync NEGATIVE edge to sync with loader
	while (!gpio_read(SER_LATCH)) {Nop();} 	// Ensure vSync is HIGH first
	while (gpio_read(SER_LATCH))  {Nop();}	// Then wait for vSync to drop
	unsigned begin = Clock();

	SendBits(ProtocolByte, 8);
	checksum += ProtocolByte << 6;  // Keep Loader.gcl dumb
	SendBits((byte)len, 6);            // Length 0, 1..60
	SendBits((byte)(address&255), 8);    // Low address bits
	SendBits((byte)(address>>8), 8);     // High address bits
	for (byte i=0; i<FramePayload; i++)     // Payload bytes
		SendBits(message[i], 8);
	byte lastByte = -checksum;   // Checksum must come out as 0
	SendBits(lastByte, 8);
	checksum = lastByte;         // Concatenate checksums
	gpio_write(SER_DATA, 1);	// Send 1 when idle

	unsigned end = Clock();
	bool IsValidFrame = (end - begin) <= 16548;		// full frame is 16684 be we expect to be done before!
	return IsValidFrame;
}

// Send execute command
void SendGt1Execute(unsigned address)
{
	byte zero[60] = {0};
	for(;;) {
		ResetChecksum();
		bool valid = SendFrame('L', 0, address, zero);
		if (valid)
			break;
		while (gpio_read(SER_LATCH))  { Nop(); }	// skip a frame for the checksum to reset on the other side
		Delay(10);									// let the raspi run during the frame.
	}
}

// Send a 1..256 byte code or data segment into the Gigatron by
// repacking it into Loader frames of max N=60 payload bytes each.
void SendGt1Segment(unsigned address, int len, const byte* data)
{
	// Send segment data
	ResetChecksum();
	while (len > 0) {
		byte n = len < FramePayload ? len : FramePayload;
		bool valid = SendFrame('L', n, address, data);
		if (valid) {
			address += n;
			data += n;
			len -= n;
		} else {
			// retry... (and reset checksum)
			ResetChecksum();
			while (gpio_read(SER_LATCH))  { Nop(); }	// skip a frame for the checksum to reset on the other side
			Delay(10);									// let the raspi run during the frame.
		}
	}

	// Wait for vBlank to start so we're 100% sure to skip one frame and
	// the checksum resets on the other side. (This is a bit pedantic)
	while (gpio_read(SER_LATCH))  { Nop(); }
}

void SendGt1File(const byte* gt1)
{
	const byte* ptr = gt1;
	int firstsegment = 1;

	gpio_setrealtimesched(1);

	for (;;) {
		unsigned addrhi = *ptr++;
		if (addrhi == 0 && !firstsegment)
			break;		// done.
		unsigned addrlo = *ptr++;
		unsigned  address = (addrhi<<8) + addrlo;
		int len = *ptr++;
		if (len == 0) len = 256;

		// Check that segment doesn't cross the page boundary
		if ((address & 255) + len > 256) {
			printf("GT1 data error (page overflow)\n");
			return;
		}

		//printf("loading %d bytes at @%X\n", len, address);
		SendGt1Segment(address, len, ptr);
		ptr += len;
		firstsegment = 0;

		Delay(10);	// we have a full frame to wait (16ms), so hopefully os activity can take place now instead of interrupting us during time sensitive code.
	};

	unsigned addrhi = *ptr++;
	unsigned addrlo = *ptr++;
	unsigned  startaddress = (addrhi<<8) + addrlo;
	if (startaddress != 0) {
		printf("executing from @%X\n", startaddress);
		SendGt1Execute(startaddress);
	}

	gpio_setrealtimesched(0);
}


int main(int argc, char **argv)
{
	if (argc < 2) {
		printf("usage: %s <file.gt1> [skip]\n", argv[0]);
		return -1;
	}

	int fd = open(argv[1], O_RDONLY);
	if (fd<0) {
		printf("could not open '%s'\n", argv[1]);
		return -1;
	}

	byte gt1data[32768];
	int len = (int)read(fd, gt1data, 32768);
	if (len == 0) {
		printf("could not read '%s'\n", argv[1]);
		return -1;
	}
	else {
		printf("loaded %d bytes\n", len);
	}

	close(fd);

	int skipmenu = (argc >= 3 && strcmp(argv[2], "skip") == 0) ;

	// Set up gpi pointer for direct register access
	gpio_setup();

	printf("Establishing communication with gigatron....\n");

	// Set GPIO pins mode for gigatron communication
	gpio_setmode_in(SER_LATCH);
	gpio_setmode_in(SER_PULSE);
	gpio_setmode_out(SER_DATA);
	gpio_write(SER_DATA, 1);	// Send 1 when idle

	printf("Detecting device: %s\n", (DetectGigatron()?"ok":"failed"));
	Delay(500);

	if (!skipmenu) {
		printf("Reset\n");
		SendReset();

		printf("start loader\n");
		StartLoader();
	}

	SendGt1File(gt1data);

	return 0;
}

