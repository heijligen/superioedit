/*	WARNING 
 * DO NOT RUN THIS SCRIPT. 
 * I DO NOT KNOW WHAT IT IS DOING EXACTLY
 */

#include <stdio.h>
#include <sys/io.h>

#define KBD_COMMAND 0x64
#define KBD_DATA    0x60

int main()
{
	if (iopl(3)) {
		printf("need to run as root\n");
		return -1;
	}
	// Support PS/2 mode
	outb (0xcb, KBD_COMMAND);
	outb (0x01, KBD_DATA);

	// keyboard init
	outb (0x60, KBD_COMMAND);
	outb (0x20, KBD_DATA);
	
	return 0;
}

