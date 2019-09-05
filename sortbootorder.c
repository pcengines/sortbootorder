/*
 * Copyright (C) 2013-2014 Sage Electronic Engineering, LLC
 * Copyright (C) 2014-2017 PC Engines GmbH
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
#include <flash_access.h>
#include <sec_reg_menu.h>
#include <spi/spi_lock_menu.h>

#include "version.h"

/*** defines ***/
#define BOOTORDER_FILE     "bootorder"
#define BOOTORDER_DEF      "bootorder_def"
#define BOOTORDER_MAP      "bootorder_map"

// These names come from bootorder_map file
// indexes depend on device order in this file
#define USB_1              0
#define USB_2              1
#define USB_3              2
#define USB_4              3
#define USB_5              4
#define USB_6              5
#define USB_7              6
#define USB_8              7
#define USB_9              8
#define USB_10             9
#define USB_11            10
#define USB_12            11
#define SDCARD            12
#define MSATA             13
#define SATA              14
#define MPCIE1_SATA1      15
#define MPCIE1_SATA2      16
#define IPXE              17

#define RESET() outb(0x06, 0x0cf9)

/*** prototypes ***/
static void show_boot_device_list(char buffer[MAX_DEVICES][MAX_LENGTH],
				  u8 line_cnt, u8 lineDef_cnt);
static void move_boot_list(char buffer[MAX_DEVICES][MAX_LENGTH], u8 line,
			   u8 max_lines);
static void copy_list_line(char *src, char *dest);
static int fetch_file_from_cbfs(char *filename,
				char destination[MAX_DEVICES][MAX_LENGTH],
				u8 *line_count);
static int fetch_bootorder(char destination[MAX_DEVICES][MAX_LENGTH],
			   u8 *line_count);
static int get_line_number(u8 line_start, u8 line_end, char key);
static void int_ids(char buffer[MAX_DEVICES][MAX_LENGTH], u8 line_cnt,
		    u8 lineDef_cnt );
static void update_tag_value(char buffer[MAX_DEVICES][MAX_LENGTH],
			     u8 *max_lines, const char * tag, char value);
#ifndef TARGET_APU1
static void update_wdg_timeout(char buffer[MAX_DEVICES][MAX_LENGTH],
			       u8 *max_lines, u16 value);
#endif
static void update_tags(char bootlist[MAX_DEVICES][MAX_LENGTH], u8 *max_lines);
static void refresh_tag_values(u8 max_lines);
static char *get_vpd_tag(const char *name);
static u8 is_tag_enabled(const char *name);

/*** local variables ***/
static int flash_address;

static u8 ipxe_toggle;
static u8 usb_toggle;
static u8 console_toggle;
static u8 com2_toggle;
static u8 com2_available;
static u8 uartc_toggle;
static u8 uartd_toggle;

#ifndef TARGET_APU1
static u8 ehci0_toggle;
static u8 boost_toggle;
static u8 sd3_toggle;
static u16 wdg_timeout;
#endif

static char bootlist_def[MAX_DEVICES][MAX_LENGTH];
static char bootlist_map[MAX_DEVICES][MAX_LENGTH];
static char id[MAX_DEVICES] = {0};

static u8 device_toggle[MAX_DEVICES];

/* sortbootorder payload:
 * This payload allows the user to reorder the lines in the bootorder file.
 * When run it will...
 *     1) Display a list of boot devices
 *     2) Chosen device will be moved to the top and reset shuffled down
 *     3) Display the new boot order
 *     4) Enable/disable the chosen option
 */

int main(void) {
	char bootlist[MAX_DEVICES][MAX_LENGTH];
	int i;
	char key;
	u8 max_lines = 0;
	u8 bootlist_def_ln = 0;
	u8 bootlist_map_ln = 0;
	u8 line_start = 0;
	u8 line_number = 0;

	// Set to enabled because enable toggle is not (yet) implemented for these devices
	device_toggle[SDCARD] = 1;
	device_toggle[MSATA] = 1;
	device_toggle[SATA] = 1;
	device_toggle[MPCIE1_SATA1] = 1;
	device_toggle[MPCIE1_SATA2] = 1;

#ifdef CONFIG_USB /* this needs to be done in order to use the USB keyboard */
	usb_initialize();
	noecho(); /* don't echo keystrokes */
#endif

#ifdef TARGET_APU1
	printf("\n### PC Engines apu1 setup %s ###\n", SORTBOOTORDER_VER);
#else
	printf("\n### PC Engines apu2 setup %s ###\n", SORTBOOTORDER_VER);
#endif

	if (init_flash()) {
		printf("Can't initialize flash device!\n");
		RESET();
	}


	lib_get_sysinfo();

	// Get required files from CBFS
	fetch_bootorder(bootlist, &max_lines);
	fetch_file_from_cbfs( BOOTORDER_DEF, bootlist_def, &bootlist_def_ln );
	fetch_file_from_cbfs( BOOTORDER_MAP, bootlist_map, &bootlist_map_ln );

	show_boot_device_list( bootlist, max_lines, bootlist_def_ln );
	int_ids( bootlist, max_lines, bootlist_def_ln );

	// Init ipxe and serial status

	ipxe_toggle	= is_tag_enabled("pxen");
	usb_toggle	= is_tag_enabled("usben");
	console_toggle	= is_tag_enabled("scon");
	com2_toggle	= is_tag_enabled("com2en");
	uartc_toggle	= is_tag_enabled("uartc");
	uartd_toggle	= is_tag_enabled("uartd");
	if(get_vpd_tag("com2en") == NULL)
		com2_available = 0;
	else
		com2_available = 1;
	
#ifndef TARGET_APU1
	ehci0_toggle	= is_tag_enabled("ehcien");
	boost_toggle	= is_tag_enabled("boosten");
	sd3_toggle	= is_tag_enabled("sd3mode");
	wdg_timeout	= (u16) strtoul(get_vpd_tag("watchdog"), NULL, 10);
#endif

	show_boot_device_list( bootlist, max_lines, bootlist_def_ln );
	int_ids( bootlist, max_lines, bootlist_def_ln );

	// Start main loop for user input
	while (1) {
		key = getchar();
		printf("%c\n\n\n", key);
		switch(key) {
			case 'r':
			case 'R':
				for (i = 0; i < max_lines && i < bootlist_def_ln; i++ )
					copy_list_line(&(bootlist_def[i][0]), &(bootlist[i][0]));
				int_ids( bootlist, max_lines, bootlist_def_ln);
				refresh_tag_values(bootlist_def_ln);
				break;
			case 'n':
			case 'N':
				ipxe_toggle ^= 0x1;
				break;
			case 'u':
			case 'U':
				usb_toggle ^= 0x1;
				break;
			case 'k':
			case 'K':
				if (com2_available)
					com2_toggle ^= 0x1;
				break;
			case 't':
			case 'T':
				console_toggle ^= 0x1;
				break;
			case 'o':
			case 'O':
				uartc_toggle ^= 0x1;
				break;
			case 'p':
			case 'P':
				uartd_toggle ^= 0x1;
				break;
#ifndef TARGET_APU1
			case 'h':
			case 'H':
				ehci0_toggle ^= 0x1;
				break;
			case 'l':
			case 'L':
				boost_toggle ^= 0x1;
				break;
			case 'i':
			case 'I':
				; // empty statement to avoid compilation error
				char *prompt = readline("Specify the watchdog"
					" timeout in seconds (0 to disable): ");
				wdg_timeout = (u16) strtoul(prompt, NULL, 10);
				prompt[0] = '\0';
				break;
			case 'j':
			case 'J':
				sd3_toggle ^= 0x1;
				break;
			case 'Q':
				handle_spi_lock_menu();
				break;
			case 'Z':
				handle_reg_sec_menu();
				break;
#endif
			case 's':
			case 'S':
				update_tags(bootlist, &max_lines);
				save_flash(flash_address, bootlist, max_lines);
				// fall through to exit ...
			case 'x':
			case 'X':
				printf("\nExiting...\n");
				RESET();
				break;
			default:
				if (key >= 'a' && key <= 'j' ) {
					line_start = 0;
					while ((line_number =  get_line_number(line_start, max_lines, key)) > line_start) {
						move_boot_list( bootlist, line_number , max_lines );
						line_start++;
					}
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
static int get_line_number(u8 line_start, u8 line_end, char key)
{
	int i;
	for (i = line_end - 1; i >= line_start; i-- ) {
		if(id[i] == key)
			break;
	}
	return (i == line_start - 1) ? 0 : i;
}

/*******************************************************************************/
static void copy_list_line(char *src, char *dest)
{
	u8 i=0;

	do {
		dest[i] = src[i];
	} while ( src[i++] != NEWLINE);
	dest[i] = NUL;
}

/*******************************************************************************/
static void show_boot_device_list(char buffer[MAX_DEVICES][MAX_LENGTH],
				  u8 line_cnt, u8 lineDef_cnt )
{
	int i,j,y,unique;
	char print_device[MAX_LENGTH];
	u8 usb_status = is_tag_enabled("usben");

	device_toggle[USB_1]  = usb_status;
	device_toggle[USB_2]  = usb_status;
	device_toggle[USB_3]  = usb_status;
	device_toggle[USB_4]  = usb_status;
	device_toggle[USB_5]  = usb_status;
	device_toggle[USB_6]  = usb_status;
	device_toggle[USB_7]  = usb_status;
	device_toggle[USB_8]  = usb_status;
	device_toggle[USB_9]  = usb_status;
	device_toggle[USB_10] = usb_status;
	device_toggle[USB_11] = usb_status;
	device_toggle[USB_12] = usb_status;
	device_toggle[IPXE]   = is_tag_enabled("pxen");

	printf("Boot order - type letter to move device to top.\n\n");
	for (i = 0; i < line_cnt; i++ ) {
		for (y = 0; y < lineDef_cnt; y++) {
			if(bootlist_def[y][0] == '/') {
				if (strcmp_printable_char(&(buffer[i][0]), &(bootlist_def[y][0])) == 0) {
					unique = 1;
					for (j = 0; j < y; j++) {
						if (strcmp_printable_char(&bootlist_map[y][0], &bootlist_map[j][0])  ==  0)
							unique = 0;
					}
					if (unique) {
						strcpy(print_device, &bootlist_map[y][0]);
						print_device[strlen(print_device)-1] = '\0';
						printf("  %s %s\n", print_device, (device_toggle[y]) ? "" : "(disabled)");
						break;
					}
				}
			}
		}
	}
	printf("\n\n");
	printf("  r Restore boot order defaults\n");
	printf("  n Network/PXE boot - Currently %s\n",
		(ipxe_toggle) ? "Enabled" : "Disabled");
	printf("  u USB boot - Currently %s\n",
		(usb_toggle) ? "Enabled" : "Disabled");
	printf("  t Serial console - Currently %s\n",
		(console_toggle) ? "Enabled" : "Disabled");
	if (com2_available)
		printf("  k Redirect console output to COM2 - Currently %s\n",
			(com2_toggle) ? "Enabled" : "Disabled");
	printf("  o UART C - Currently %s\n",
		(uartc_toggle) ? "Enabled" : "Disabled");
	printf("  p UART D - Currently %s\n",
		(uartd_toggle) ? "Enabled" : "Disabled");
#ifndef TARGET_APU1
	printf("  h EHCI0 controller - Currently %s\n",
		(ehci0_toggle) ? "Enabled" : "Disabled");
	printf("  l Core Performance Boost - Currently %s\n",
		(boost_toggle) ? "Enabled" : "Disabled");
	printf("  i Watchdog - Currently %s\n",
		(wdg_timeout) ? "Enabled" : "Disabled");
	printf("  j SD 3.0 mode - Currently %s\n",
		(sd3_toggle) ? "Enabled" : "Disabled");
#endif
	printf("  x Exit setup without save\n");
	printf("  s Save configuration and exit\n");
}

/*******************************************************************************/
static void int_ids(char buffer[MAX_DEVICES][MAX_LENGTH], u8 line_cnt,
		    u8 lineDef_cnt )
{
	int i,y;
	for (i = 0; i < line_cnt; i++ ) {
		if (buffer[i][0] == '/') {
			for (y = 0; y < lineDef_cnt; y++) {
				if (strcmp_printable_char(&(buffer[i][0]), &(bootlist_def[y][0])) == 0) {
					strncpy(&id[i], &(bootlist_map[y][0]), 1);
					break;
				}
			}
		}
	}
}

/*******************************************************************************/
static int fetch_file_from_cbfs(char *filename,
				char destination[MAX_DEVICES][MAX_LENGTH],
				u8 *line_count)
{
	char *cbfs_dat, tmp;
	int cbfs_offset = 0, char_cnt = 0;
	size_t cbfs_length;

	cbfs_dat = (char *) cbfs_get_file_content( CBFS_DEFAULT_MEDIA, filename, CBFS_TYPE_RAW, &cbfs_length );
	if (!cbfs_dat) {
		printf("Error: file [%s] not found!\n", filename);
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

		if (*line_count > MAX_DEVICES) {
			printf("aborting due to excessive line_count\n");
			break;
		}
		if (char_cnt > MAX_LENGTH) {
			printf("aborting due to excessive char count\n");
			break;
		}
		if (cbfs_offset > (MAX_LENGTH*MAX_DEVICES)) {
			printf("aborting due to excessive cbfs ptr length\n");
			break;
		}
	}
	return 0;
}

static int fetch_bootorder(char destination[MAX_DEVICES][MAX_LENGTH],
			   u8 *line_count)
{
	char tmp;
	int offset = 0, char_cnt = 0;
	u32 bootorder_offset, bootorder_size;
	struct cbfs_media default_media;
	struct cbfs_media *media = &default_media;
	char *bootorder_dat;

	u32 rom_begin = (0xFFFFFFFF - lib_sysinfo.spi_flash.size) + 1;

	int rc = fmap_region_by_name("BOOTORDER", &bootorder_offset,
				     &bootorder_size);
	if (rc == -1) {
		printf("Error: BOOTORDER not found!\n");
		return 1;
	}

	flash_address = (void *)(rom_begin + bootorder_offset);
	if ((u32)flash_address & 0xfff)
		printf("Warning: The bootorder file is not 4k aligned!\n");


	bootorder_dat = (char *)malloc(bootorder_size);

	if(!bootorder_dat)
		return -1;

	if (init_default_cbfs_media(media) != 0) {
		free(bootorder_dat);
		return -1;
	}

	media->open(media);

	if (!media->read(media, bootorder_dat, bootorder_offset,
			 bootorder_size)) {
		free(bootorder_dat);
		return -1;
	}

	media->close(media);

	tmp = *(bootorder_dat);
	if (tmp == 0xFF) {
		printf("Error: bootorder is empty!\n");
		free(bootorder_dat);
		return -1;
	}

	//count up the lines and display
	*line_count = 0;
	while (1) {
		tmp = *(bootorder_dat + offset++);
		destination[*line_count][char_cnt] = tmp;
		if (tmp == NEWLINE) {
			(*line_count)++;
			char_cnt = 0;
		}
		else if ((tmp == 0xFF) || (tmp == NUL) ||
			 (offset > bootorder_size))
			break;
		else
			char_cnt++;

		if (*line_count > MAX_DEVICES) {
			printf("aborting due to excessive line_count\n");
			break;
		}
		if (char_cnt > MAX_LENGTH) {
			printf("aborting due to excessive char count\n");
			break;
		}
		if (offset > (MAX_LENGTH*MAX_DEVICES)) {
			printf("aborting due to excessive cbfs ptr length\n");
			break;
		}
	}
	free(bootorder_dat);
	return 0;
}


/*******************************************************************************/
static void move_boot_list(char buffer[MAX_DEVICES][MAX_LENGTH], u8 line,
			   u8 max_lines )
{
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
static void update_tag_value(char buffer[MAX_DEVICES][MAX_LENGTH],
			     u8 *max_lines, const char * tag, char value)
{
	int i;
	bool found = FALSE;

	for (i = 0; i < *max_lines; i++) {
		if (!strncmp(tag, &buffer[i][0], strlen(tag))) {
			found = TRUE;
			buffer[i][strlen(tag)] = value;
			break;
		}
	}

	if (!found) {
		if ((*max_lines + 1) > MAX_DEVICES) {
			return;
		}
		strcpy(&buffer[*max_lines][0], tag);
		buffer[*max_lines][strlen(tag)] = value;
		buffer[*max_lines][strlen(tag)+1] = '\r';
		buffer[*max_lines][strlen(tag)+2] = '\n';
		buffer[*max_lines][strlen(tag)+3] = '0';
		(*max_lines)++;
	}
}
#ifndef TARGET_APU1
static void update_wdg_timeout(char buffer[MAX_DEVICES][MAX_LENGTH],
				u8 *max_lines, u16 value)
{
	int i;
	bool found = FALSE;
	char *tag = "watchdog";

	for (i = 0; i < *max_lines; i++) {
		if (!strncmp(tag, &buffer[i][0], strlen(tag))) {
			found = TRUE;
			snprintf(&buffer[i][strlen(tag)], 5, "%04x", value);
			buffer[*max_lines][strlen(tag)+4] = '\r';
			buffer[*max_lines][strlen(tag)+5] = '\n';
			break;
		}
	}

	if (!found) {
		if ((*max_lines + 1) > MAX_DEVICES) {
			return;
		}
		strcpy(&buffer[*max_lines][0], tag);
		snprintf(&buffer[*max_lines][strlen(tag)], 5, "%04x", value);
		buffer[*max_lines][strlen(tag)+4] = '\r';
		buffer[*max_lines][strlen(tag)+5] = '\n';
		buffer[*max_lines][strlen(tag)+6] = '0';
		(*max_lines)++;
	}	
}
#endif

static void update_tags(char bootlist[MAX_DEVICES][MAX_LENGTH], u8 *max_lines)
{
	update_tag_value(bootlist, max_lines, "pxen", ipxe_toggle);
	update_tag_value(bootlist, max_lines, "usben",	usb_toggle);
	update_tag_value(bootlist, max_lines, "scon", console_toggle);
	if (get_vpd_tag("com2en")) {
		update_tag_value(bootlist, max_lines, "com2en", com2_toggle);
	}
	update_tag_value(bootlist, max_lines, "uartc",	uartc_toggle);
	update_tag_value(bootlist, max_lines, "uartd", uartd_toggle);
#ifndef TARGET_APU1
	update_tag_value(bootlist, max_lines, "ehcien", ehci0_toggle;
	update_tag_value(bootlist, max_lines, "boosten", boost_toggle);
	update_tag_value(bootlist, max_lines, "sd3mode", sd3_toggle);
	update_wdg_timeout(bootlist, max_lines, wdg_timeout);
#endif
}
/*******************************************************************************/
static void refresh_tag_values(u8 max_lines)
{
	int i;
	char *token;

	for ( i = 0; i < max_lines; i++) {
		token = strstr(&(bootlist_def[i][0]), "pxen");
		if(token) {
			token += strlen("pxen");
			ipxe_toggle = strtoul(token, NULL, 10);
	  	}

		token = strstr(&(bootlist_def[i][0]), "usben");
		if(token) {
			token += strlen("usben");
			usb_toggle = strtoul(token, NULL, 10);
		}

		token = strstr(&(bootlist_def[i][0]), "scon");
		if(token) {
			token += strlen("scon");
			console_toggle = strtoul(token, NULL, 10);
		}
		token = strstr(&(bootlist_def[i][0]), "com2en");
		if(token && com2_available) {
			token += strlen("com2en");
			com2_toggle = strtoul(token, NULL, 10);
		}

		token = strstr(&(bootlist_def[i][0]), "uartc");
		if(token) {
			token += strlen("uartc");
			uartc_toggle = strtoul(token, NULL, 10);
		}

		token = strstr(&(bootlist_def[i][0]), "uartd");
		if(token) {
			token += strlen("uartd");
			uartd_toggle = strtoul(token, NULL, 10);
		}

#ifndef TARGET_APU1
		token = strstr(&(bootlist_def[i][0]), "ehcien");
		if(token) {
			token += strlen("ehcien");
			ehci0_toggle = strtoul(token, NULL, 10);
		}
		token = strstr(&(bootlist_def[i][0]), "mpcie2_clk");
		if(token) {
			token += strlen("mpcie2_clk");
			mpcie2_clk_toggle = strtoul(token, NULL, 10);
		}
		token = strstr(&(bootlist_def[i][0]), "boosten");
		if(token) {
			token += strlen("boosten");
			boost_toggle = strtoul(token, NULL, 10);
		}
		token = strstr(&(bootlist_def[i][0]), "sd3mode");
		if(token) {
			token += strlen("sd3mode");
			sd3_toggle = strtoul(token, NULL, 10);
		}

		token = strstr(&(bootlist_def[i][0]), "watchdog");
		if(token) {
			token += strlen("watchdog");
			wdg_timeout = (u16) strtoul(token, NULL, 16);
		}
#endif
	}
}

static char *get_vpd_tag(const char *name)
{
	int tag_size = 10;
	char vpd_tag[tag_size];
	if (!vpd_gets(name, vpd_tag, tag_size, VPD_RW)) {
		if(!vpd_gets(name, vpd_tag, tag_size, VPD_RO))
			return NULL;
	}

	if (!memcmp(vpd_tag, "enabled", strlen("enabled")))
		return "enabled";
	else if (!memcmp(vpd_tag, "disabled", strlen("disabled")))
		return "disabled";
	else if (!strcmp(name, "watchdog")) {
		return vpd_tag;
	}
	else
		return NULL;
}



static u8 is_tag_enabled(const char *name)
{
	if (!memcmp(get_vpd_tag(name), "enabled", strlen("enabled")))
		return 1;
	else if (!memcmp(get_vpd_tag(name), "disabled", strlen("disabled")))
		return 0;
	else
		return 0;
}
