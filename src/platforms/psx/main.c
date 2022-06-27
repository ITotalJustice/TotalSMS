#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <sms.h>

#include <psxetc.h>
#include <psxgte.h>
#include <psxgpu.h>

#include "rom.h"
#define ROM roms_Sonic_The_Hedgehog__USA__Europe__sms
#define ROM_SIZE roms_Sonic_The_Hedgehog__USA__Europe__sms_len
#define ROM_NAME "rom.sms"

static struct SMS_Core sms = {0};
// Define display/draw environments for double buffering
static DISPENV disp[2];
static DRAWENV draw[2];
static int db;

// see: https://github.com/Lameguy64/PSn00bSDK/blob/master/libpsn00b/include/assert.h
// not a macro!
void assert(int e) {}

// Init function
static void init(void)
{
	// This not only resets the GPU but it also installs the library's
	// ISR subsystem to the kernel
	ResetGraph(0);

	// Define display environments, first on top and second on bottom
	SetDefDispEnv(&disp[0], 0, 0, 320, 240);
	SetDefDispEnv(&disp[1], 0, 240, 320, 240);

	// Define drawing environments, first on bottom and second on top
	SetDefDrawEnv(&draw[0], 0, 240, 320, 240);
	SetDefDrawEnv(&draw[1], 0, 0, 320, 240);

	// Set and enable clear color
	setRGB0(&draw[0], 0, 96, 0);
	setRGB0(&draw[1], 0, 96, 0);
	draw[0].isbg = 1;
	draw[1].isbg = 1;

	// Clear double buffer counter
	db = 0;

	// Apply the GPU environments
	PutDispEnv(&disp[db]);
	PutDrawEnv(&draw[db]);

	// Load test font
	FntLoad(960, 0);

	// Open up a test font text stream of 100 characters
	FntOpen(0, 8, 320, 224, 0, 100);
}

// Display function
static void display(void)
{
	// Flip buffer index
	db = !db;

	// Wait for all drawing to complete
	DrawSync(0);

	// Wait for vertical sync to cap the logic to 60fps (or 50 in PAL mode)
	// and prevent screen tearing
	VSync(0);

	// Switch pages
	PutDispEnv(&disp[db]);
	PutDrawEnv(&draw[db]);

	// Enable display output, ResetGraph() disables it by default
	SetDispMask(1);
}

static volatile int fps_counter = 0;
static void vs(void)
{
	fps_counter++;
}

int main(int argc, char** argv)
{
    init();
    SMS_init(&sms);
    SMS_loadrom(&sms, ROM, ROM_SIZE, 0);
	VSyncCallback(vs);

    while (1)
    {
        static int counter = 0;

        SMS_run(&sms, SMS_CYCLES_PER_FRAME);

		// Increment the counter
		counter++;

		if (fps_counter >= 60)
		{
			// Print the obligatory hello world and counter to show that the
			// program isn't locking up to the last created text stream
			FntPrint(-1, "HELLO WORLD\n");
			FntPrint(-1, "COUNTER=%d %d\n", counter, fps_counter);

			// Draw the last created text stream
			FntFlush(-1);
			display();
			fps_counter -= 60;
			counter = 0;
		}
    }

    return 0;
}
