/*
 * Copyright (C) 2013-2014 Sage Electronic Engineering, LLC
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <libpayload.h>
#include <cbfs.h>
#include <curses.h>
#include "spi.h"

/*** defines ***/
#define CONFIG_SPI_FLASH_NO_FAST_READ
#define BOOTORDER_FILE     "bootorder"
#define BOOTORDER_DEF      "bootorder_def"
#define BOOTORDER_MAP      "bootorder_map"
#define MAX_DEVICES        64
#define MAX_LENGTH         64
#define NEWLINE            0x0A
#define NUL                0x00
#define FLASH_SIZE_CHUNK   0x1000 //4k

/*** prototypes ***/
static void show_boot_device_list( char buffer[MAX_DEVICES][MAX_LENGTH], u8 line_cnt, u8 lineDef_cnt );
static void move_boot_list( char buffer[MAX_DEVICES][MAX_LENGTH], u8 line, u8 max_lines );
static void copy_list_line( char *src, char *dest );
static int fetch_file_from_cbfs( char *filename, char destination[MAX_DEVICES][MAX_LENGTH], u8 *line_count);
static int get_line_number(u8 line_cnt, char key );
static void int_ids( char buffer[MAX_DEVICES][MAX_LENGTH], u8 line_cnt, u8 lineDef_cnt );
static void save_flash(char buffer[MAX_DEVICES][MAX_LENGTH], u8 max_lines);
static void update_tag_value(char buffer[MAX_DEVICES][MAX_LENGTH], u8 max_lines, const char * tag, char value);

/*** local variables ***/
static u8 ipxe_toggle;
static u8 serial_toggle;
static char bootlist_def[MAX_DEVICES][MAX_LENGTH];
static char bootlist_map[MAX_DEVICES][MAX_LENGTH];
static char id[MAX_DEVICES] = {0};
static int flash_address;

/* sortbootorder payload:
 * This payload allows the user to reorder the lines in the bootorder file.
 * When run it will...
 *     1) Display a list of boot devices
 *     2) Chosen device will be moved to the top and reset shuffled down
 *     3) Display the new boot order
 *     4) Give the option to:
 *        - Restore order defaults,
 *        - Serial console disable / enable
 *        - Network / IPXE disable / enable
 *        - Exit with or without saving order
 */

int main(void) {
	char bootlist[MAX_DEVICES][MAX_LENGTH];
	int i;
	char key;
	u8 max_lines = 0;
	u8 bootlist_def_ln = 0;
	u8 bootlist_map_ln = 0;
	char *ipxe_str;
	char *scon_str;

#ifdef CONFIG_USB /* this needs to be done in order to use the USB keyboard */
	usb_initialize();
	noecho(); /* don't echo keystrokes */
#endif

	printf("\n*********************************************************************");
	printf("\n*** Sortbootorder payload    ver 1.1   Sage Electronic Engineering  *");
	printf("\n*********************************************************************\n");

	// Find out where the bootorder file is in rom
	char *tmp = cbfs_get_file_content( CBFS_DEFAULT_MEDIA, BOOTORDER_FILE, CBFS_TYPE_RAW, NULL );
	flash_address = (int)tmp;
	if ((u32)tmp & 0xfff)
		printf("Warning: The bootorder file is not 4k aligned!\n");

	// Get required files from CBFS
	fetch_file_from_cbfs( BOOTORDER_FILE, bootlist, &max_lines );
	fetch_file_from_cbfs( BOOTORDER_DEF, bootlist_def, &bootlist_def_ln );
	fetch_file_from_cbfs( BOOTORDER_MAP, bootlist_map, &bootlist_map_ln );

	// Init ipxe and serial status
	ipxe_str = cbfs_find_string("pxen", BOOTORDER_FILE);
	ipxe_str += strlen("pxen");
	ipxe_toggle = ipxe_str ? strtoul(ipxe_str, NULL, 10) : 1;

	scon_str = cbfs_find_string("scon", BOOTORDER_FILE);
	scon_str += strlen("scon");
	serial_toggle = scon_str ? strtoul(scon_str, NULL, 10) : 1;

	show_boot_device_list( bootlist, max_lines, bootlist_def_ln );
	int_ids( bootlist, max_lines, bootlist_def_ln );

	// Start main loop for user input
	while (1) {
		printf("\n> ");
		key = getchar();
		printf("%c\n\n\n", key);
		switch(key) {
			case 'R':
				for (i = 0; i < max_lines && i < bootlist_def_ln; i++ )
					copy_list_line(&(bootlist_def[i][0]), &(bootlist[i][0]));
				int_ids( bootlist, max_lines, bootlist_def_ln );
				break;
			case 'S':
				serial_toggle ^= 0x1;
				break;
			case 'N':
				ipxe_toggle ^= 0x1;
				break;
			case 'E':
				update_tag_value(bootlist, max_lines, "scon", serial_toggle + '0');
				update_tag_value(bootlist, max_lines, "pxen", ipxe_toggle + '0');
				save_flash( bootlist, max_lines );
				// fall through to exit ...
			case 'X':
				printf("\nExiting ...");
				outb(0x06, 0x0cf9); /* reset */
				break;
			default:
				if (key >= 'a' && key <= 'z' ) {
					move_boot_list( bootlist, get_line_number(max_lines,key), max_lines );
				}
				break;
		}
		show_boot_device_list( bootlist, max_lines, bootlist_def_ln );
	}
	return 0;  /* should never get here! */
}

/*******************************************************************************/
static int strcmp_printable_char(const char *s1, const char *s2)
{
	int i, res;

	for (i = 0; 1; i++) {
		res = s1[i] - s2[i];
		if (s1[i] == '\0')
			break;
		if (res && (s1[i] > 31 && s2[i] > 31))
			break;
	}
	return res;
}

/*******************************************************************************/
static int get_line_number( u8 line_cnt, char key ) {
	int i;
	for (i = 0; i < line_cnt; i++ ) {
		if(id[i] == key)
			break;
	}
	return (i == line_cnt) ? 0 : i;
}

/*******************************************************************************/
static void copy_list_line( char *src, char *dest ) {
u8 i=0;

	do {
		dest[i] = src[i];
	} while ( src[i++] != NEWLINE);
	dest[i] = NUL;
}

/*******************************************************************************/
static void show_boot_device_list( char buffer[MAX_DEVICES][MAX_LENGTH], u8 line_cnt, u8 lineDef_cnt ) {
	int i,y;

	printf("==============================================\n");
	printf("Type lower case letter to move device to top\n");
	printf("==============================================\n");
	printf("boot devices\n\n");
	for (i = 0; i < line_cnt; i++ ) {
		for (y = 0; y < lineDef_cnt; y++) {
			if (strcmp_printable_char(&(buffer[i][0]), &(bootlist_def[y][0])) == 0)
				 break;
		}
		printf("  %s", &(bootlist_map[y][0]));
	}
	printf("==============================================\n");
	printf("Type upper case letter to invoke action\n");
	printf("==============================================\n");
	printf("  R Restore boot order defaults\n");
	printf("  N Network/PXE boot - Currently %s\n", (ipxe_toggle) ? "Enabled" : "Disabled");
	printf("  S Serial console - Currently %s\n", (serial_toggle) ? "Enabled" : "Disabled");
	printf("  E Exit setup with save\n");
	printf("  X Exit setup without save\n");
}

/*******************************************************************************/
static void int_ids( char buffer[MAX_DEVICES][MAX_LENGTH], u8 line_cnt, u8 lineDef_cnt ) {
int i,y;
	for (i = 0; i < line_cnt; i++ ) {
		for (y = 0; y < lineDef_cnt; y++) {
			if (strcmp_printable_char(&(buffer[i][0]), &(bootlist_def[y][0])) == 0) {
				strncpy(&id[i], &(bootlist_map[y][0]), 1);
				break;
			}
		}
	}
}

/*******************************************************************************/
static int fetch_file_from_cbfs( char *filename, char destination[MAX_DEVICES][MAX_LENGTH], u8 *line_count) {
	char *cbfs_dat, tmp;
	int cbfs_offset = 0, char_cnt = 0;
	size_t cbfs_length;

	cbfs_dat = (char *) cbfs_get_file_content( CBFS_DEFAULT_MEDIA, filename, CBFS_TYPE_RAW, &cbfs_length );
	if (!cbfs_dat) {
		printf("Error: file [%%s] not found!\n");
		return 1;
	}

	//count up the lines and display
	*line_count = 0;
	while (1) {
		tmp = *(cbfs_dat + cbfs_offset++);
		destination[*line_count][char_cnt] = tmp;
		if (tmp == NEWLINE) {
			(*line_count)++;
			char_cnt = 0;
		}
		else if ((tmp == NUL) || (cbfs_offset > cbfs_length))
			break;
		else
			char_cnt++;

		if ( *line_count > MAX_DEVICES) {
			printf("aborting due to excessive line_count\n");
			break;
		}
		if ( char_cnt > MAX_LENGTH) {
			printf("aborting due to excessive char count\n");
			break;
		}
		if ( cbfs_offset > (MAX_LENGTH*MAX_DEVICES)) {
			printf("aborting due to excessive cbfs ptr length\n");
			break;
		}
	}
	return 0;
}

/*******************************************************************************/
static void move_boot_list( char buffer[MAX_DEVICES][MAX_LENGTH], u8 line, u8 max_lines ) {
	char temp_line[MAX_LENGTH];
	char ln;
	u8 x;

	// do some early error checking
	if (line == 0)
		return;

	// copy selection into temp
	copy_list_line( &(buffer[line][0]), temp_line );
	ln = id[line];

	// shuffle entries down
	for (x = line; x > 0; x--) {
		copy_list_line( &(buffer[x-1][0]), &(buffer[x][0]) );
		id[x] = id[x - 1];
	}

	// copy selection into top position
	copy_list_line(temp_line, &(buffer[0][0]) );
	id[0] = ln;
}

/*******************************************************************************/
static void save_flash(char buffer[MAX_DEVICES][MAX_LENGTH], u8 max_lines) {
	int i = 0;
	int k = 0;
	int j;
	char cbfs_formatted_list[MAX_DEVICES * MAX_LENGTH];
	struct spi_flash *flash;
	u32 nvram_pos;

	// compact the table into the expected packed list
	for (j = 0; j < max_lines; j++) {
		k = 0;
		while (1) {
			cbfs_formatted_list[i++] = buffer[j][k];
			if (buffer[j][k] == NEWLINE )
				break;
			k++;
		}
	}

	cbfs_formatted_list[i++] = NUL;
	spi_init();
	flash = spi_flash_probe(0, 0, 0, 0);

	if (!flash)
		printf("Could not find SPI device\n");
	else {
		printf("Erasing Flash size 0x%x @ 0x%x\n", FLASH_SIZE_CHUNK, flash_address);
		flash->erase(flash, flash_address, FLASH_SIZE_CHUNK);
		flash->spi->rw = SPI_WRITE_FLAG;
		printf("Writing %d bytes @ 0x%x\n", i, flash_address);
		for (nvram_pos = 0; nvram_pos < (i & 0xFC); nvram_pos += 4) {
			flash->write(flash, nvram_pos + flash_address, sizeof(u32), (u32 *)(cbfs_formatted_list + nvram_pos));
		}
		flash->write(flash, nvram_pos + flash_address, sizeof(i % 4), (u32 *)(cbfs_formatted_list + nvram_pos));
	}
}

/*******************************************************************************/
static void update_tag_value(char buffer[MAX_DEVICES][MAX_LENGTH], u8 max_lines, const char * tag, char value)
{
	int i;

	for (i = 0; i < max_lines; i++) {
		if (!strncmp(tag, &buffer[i][0], strlen(tag))) {
			buffer[i][strlen(tag)] = value;
			break;
		}
	}
}
