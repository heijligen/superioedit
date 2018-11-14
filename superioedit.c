/*
* ----------------------------------------------------------------------------
* "THE BEER-WARE LICENSE" (Revision 42):
* <src@posteo.de> wrote this file. As long as you retain this notice you
* can do whatever you want with this stuff. If we meet some day, and you think
* this stuff is worth it, you can buy me a beer in return.
* ----------------------------------------------------------------------------
*/

#include <stdio.h>
#include <sys/io.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

/* 	superioedit
 * compile:
 * 	cc -O -o superioedit superioedit.c
 *
 */

#define USAGE "\nUsage: superioedit ...\n\
  modes:\n\
  -E                   | list all enabled devices\n\
  -D <LDN|'all'>       | get values from all registers from (all) devices\n\
  -e <LDN>             | enable device\n\
  -d <LDN>             | disable device\n\
  -g <LDN> <REG>       | get value from register at device\n\
  -s <LDN> <REG> <VAL> | set value to register at device\n\
  flags:\n\
  -v                   | verbose output\n\
  -i                   | interactive (y/n) bevor execute\n\n"

/*
 *
 * Index register (I/O port 0x2E or 0x4E, depending on the chip)
 *  Data register (I/O port 0x2F or 0x4F, obtained by adding 1 to the Index register)
 */

#define SUPERIO_INDEX_REGISTER 0x2e
#define SUPERIO_DATA_REGISRER SUPERIO_INDEX_REGISTER+1

#define DEVICE_SELECT_REGISTER 0x07
#define DEVICE_ENABLE_REGISTER 0x30
#define DEVICE_VERSION_REGISTER 0x20
#define DEVICE_REVISION_REGISTER 0x21
#define ENABLE_CONFMODE_VALUE 0x55


void select_device (uint16_t port, uint8_t ldn)
{
	outb(DEVICE_SELECT_REGISTER, port);
	outb(ldn, port+1);
}

void set (uint16_t port, uint8_t ldn, uint8_t reg, uint8_t val)
{
	select_device(port, ldn);

	outb(reg, port);
	outb(val, port+1);
}

uint8_t get (uint16_t port, uint8_t ldn, uint8_t reg)
{
	select_device(port, ldn);
	
	outb(reg, port);
	return inb(port+1);
}

void enable_confmode (uint16_t port)
{
	outb (ENABLE_CONFMODE_VALUE, port);
}



int main(int argc, char **argv)
{
	enum {
		mode_none,
		mode_set,
		mode_get,
		mode_dump,
		mode_list
	} mode = mode_none;

	int verbose = 0, interactive = 0;
	int ldn, reg, val;
	int argi = 1;
	while ((argi < argc) && (mode == mode_none)) {
		if (!strcmp(argv[argi], "-v")) {
			verbose = 1;
		} else if (!strcmp(argv[argi], "-i")) {
			interactive = 1;
		} else if (!strcmp(argv[argi], "-s") && ((argc-argi) > 3)) {
			mode = mode_set;
			ldn = atoi(argv[argi+1]);
			reg = atoi(argv[argi+2]);
			val = atoi(argv[argi+3]);
			argi+=3;
		} else if (!strcmp(argv[argi], "-g") && ((argc-argi) > 2)) {
			mode = mode_get;
			ldn = atoi(argv[argi+1]);
			reg = atoi(argv[argi+2]);
			argi+=2;
		}  else if (!strcmp(argv[argi], "-d") && ((argc-argi) > 1)) {
			mode = mode_set;
			reg = DEVICE_ENABLE_REGISTER;
			val = 0;
		} else if (!strcmp(argv[argi], "-e") && ((argc-argi) > 1)) {
			mode = mode_set;
			ldn = atoi(argv[argi+1]);
			reg = DEVICE_ENABLE_REGISTER;
			val = 1;
		} else if (!strcmp(argv[argi], "-D") && ((argc-argi) > 1)) {
			mode = mode_dump;
			ldn = atoi(argv[argi+1]);
			argi+=1;
		} else if (!strcmp(argv[argi], "-E")) {
			mode = mode_list;
		} else {
			printf(USAGE);
			return -1;
		}
		argi++;
	}
	if (mode == mode_none) {
		printf(USAGE);
		return -1;
	}

	// setup hardware
	if (ioperm(SUPERIO_INDEX_REGISTER, 2, 1)) {	
		printf("tool need to run as root\n");
		return -1;
	}
	enable_confmode(SUPERIO_INDEX_REGISTER);
	
	printf("ldn: %i, reg: %i, val: %i\n", ldn, reg, val);

	switch(mode) {
		case mode_set:
			set(SUPERIO_INDEX_REGISTER, ldn, reg, val);
			break;
		case mode_get:
			printf("%i\n", get(SUPERIO_INDEX_REGISTER, ldn, reg));
			break;
		case mode_dump:
			for (reg = 0; reg < 0xff; reg++) {
				 printf("%x %x\n",reg, get(SUPERIO_INDEX_REGISTER, ldn, reg));
			}
			break;
		case mode_list:
			break;
	}
	return 0;
}

