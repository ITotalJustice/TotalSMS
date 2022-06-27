#include <sms.h>
#include <nds.h>
#include <stdio.h>

#include "rom.h"
#define ROM roms_Sonic_The_Hedgehog__USA__Europe__sms
#define ROM_SIZE roms_Sonic_The_Hedgehog__USA__Europe__sms_len

static struct SMS_Core sms = {0};

int main(void)
{
    consoleDemoInit();

    if (!SMS_init(&sms))
    {
        iprintf("failed to init sms");
    }

    if (!SMS_loadrom(&sms, ROM, ROM_SIZE, 0))
    {
        iprintf("failed to load sms rom");
    }

	iprintf("Hello World!");

	for (;;)
    {
        static int counter = 0;
        counter++;
        iprintf("\rHello World! %d\n", counter);
		// swiWaitForVBlank();
		scanKeys();
		int pressed = keysDown();
		if(pressed & KEY_START) break;

        SMS_run(&sms, SMS_CYCLES_PER_FRAME);
	}
}
