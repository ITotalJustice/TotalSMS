#include "sms.h"

#include <stdio.h>


static struct SMS_Core CORE = {0};


int main(int argc, char const *argv[]) {
	test();

	printf("%s\n", "hello world!\n");
	return 0;
}