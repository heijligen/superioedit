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
  -I                   | show some infos\n\
  -D <LDN|'all'>       | get values from all registers from (all) devices\n\
  -e <LDN>             | enable device\n\
  -d <LDN>             | disable device\n\
  -g <LDN> <REG>       | get value from register at device\n\
  -s <LDN> <REG> <VAL> | set value to register at device\n\
  flags:\n\
  -v                   | verbose outbut (not working)\n\
  -y                   | no interaction, accept input without confirmation (not working)\n\n"

/*
 *
 * Index register (I/O port 0x2E or 0x4E, depending on the chip)
 *  Data register (I/O port 0x2F or 0x4F, obtained by adding 1 to the Index register)
 */

#define SUPERIO_INDEX_REGISTER 0x2e
#define SUPERIO_DATA_REGISTER SUPERIO_INDEX_REGISTER+1

#define DEVICE_SELECT_REGISTER 0x07
#define DEVICE_ENABLE_REGISTER 0x30
#define DEVICE_VERSION_REGISTER 0x20
#define DEVICE_REVISION_REGISTER 0x21
#define ENABLE_CONFMODE_VALUE 0x55
#define DISABLE_CONFMODE_VALUE 0xaa


void select_device (uint8_t ldn)
{
	outb_p(DEVICE_SELECT_REGISTER, SUPERIO_INDEX_REGISTER);
	outb_p(ldn, SUPERIO_DATA_REGISTER);
}

void set (uint8_t reg, uint8_t val)
{
	outb_p(reg, SUPERIO_INDEX_REGISTER);
	outb_p(val, SUPERIO_DATA_REGISTER);
}

uint8_t get (uint8_t reg)
{
	outb_p(reg, SUPERIO_INDEX_REGISTER);
	return inb_p(SUPERIO_DATA_REGISTER);
}

void enable_confmode ()
{
	/* Thinkpad
	outb (0x55, SUPERIO_INDEX_REGISTER);
	*/
	// ITE
	outb (0x89, SUPERIO_INDEX_REGISTER);
	outb (0x91, SUPERIO_INDEX_REGISTER);
	outb (0x55, SUPERIO_INDEX_REGISTER);
	outb (0x55, SUPERIO_INDEX_REGISTER);
}

int parsearg(int argc, char **argv, int mode, ...)

int main(int argc, char **argv)
{
	enum {
		mode_none = 0,
		mode_set,
		mode_get,
		mode_dump,
		mode_list,
		mode_info
	} mode = mode_none;

	int verbose = 0, interactive = 1, dump_all = 0;
	int ldn = 0, reg = 0, val = 0;
	for (int argi = 1; argi < argc; argi++) {
		if (!strcmp(argv[argi], "-v")) {
			verbose = 1;
		}
		else if (!strcmp(argv[argi], "-y")) {
			interactive = 0;
		}
		else if (!mode && !strcmp(argv[argi], "-s") && ((argc-argi) > 3)) {
			mode = mode_set;
			ldn = strtoul(argv[argi+1], NULL, 0);
			reg = strtoul(argv[argi+2], NULL, 0);
			val = strtoul(argv[argi+3], NULL, 0);
			argi+=3;
		}
		else if (!mode && !strcmp(argv[argi], "-g") && ((argc-argi) > 2)) {
			mode = mode_get;
			ldn = strtoul(argv[argi+1], NULL, 0);
			reg = strtoul(argv[argi+2], NULL, 0);
			argi+=2;
		}
		else if (!mode && !strcmp(argv[argi], "-d") && ((argc-argi) > 1)) {
			mode = mode_set;
			ldn = strtoul(argv[argi+1], NULL, 0);
			reg = DEVICE_ENABLE_REGISTER;
			val = 0;
			argi+=1;
		}
		else if (!mode && !strcmp(argv[argi], "-e") && ((argc-argi) > 1)) {
			mode = mode_set;
			ldn = strtoul(argv[argi+1], NULL, 0);
			reg = DEVICE_ENABLE_REGISTER;
			val = 1;
			argi+=1;
		}
		else if (!mode && !strcmp(argv[argi], "-D") && ((argc-argi) > 1)) {
			mode = mode_dump;
			if (!strcmp(argv[argi+1], "all")) {
				dump_all = 1;
				ldn = 0;
				printf("dump_all\n");
			} else 
				ldn = strtoul(argv[argi+1], NULL, 0);
			reg = 0;
			argi+=1;
		}
		else if (!mode && !strcmp(argv[argi], "-E")) {
			mode = mode_list;
		}
		else if (!mode && !strcmp(argv[argi], "-I")) {
			mode = mode_info;
		}
		else {
			printf(USAGE);
			return -1;
		}
	}

	if (mode == mode_none) {
		printf(USAGE);
		return -1;
	}

	// setup hardware
	if (iopl(3)) {
		printf("tool need to run as root\n");
		return -1;
	}
	enable_confmode();
	
	switch(mode) {
		case mode_set:
			select_device(ldn);
			set(reg, val);
			break;

		case mode_get:
			select_device(ldn);
			val = get(reg);
			printf("0x%02x\n", val);
			break;

		case mode_dump:
			do {
				select_device(ldn);
				printf("Device 0x%02x\n", ldn);
				
				for (reg = 0x00; reg <= 0xff; reg++) {
					if (reg == DISABLE_CONFMODE_VALUE)
						continue;
					val = get(reg);
					if (val)
						printf("\treg: 0x%02x, val: 0x%02x\n", reg, val);
				}
				ldn++;
			} while (ldn < (dump_all ? 0x100 : ldn));

			break;

		case mode_list:
			for (ldn = 0; ldn <= 0xff; ldn++) {
				select_device(ldn);
				val = get(DEVICE_ENABLE_REGISTER);
				if (val)
					printf("Device 0x%02x enabled\n", ldn);
			}
			break;

		case mode_info:
			printf("Not implemented yet :(\n");
			break;
	}
	return 0;
}

