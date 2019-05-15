/*
 * Copyright (C) 2019 PC Engines GmbH
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
#include <curses.h>
#include <flash_access.h>
#include <string.h>
#include <spi/winbond_flash.h>
#include <spi/spi_lock_menu.h>

typedef union {
	struct  {
		u8 busy : 1,
		u8 wel  : 1,
		u8 bp0  : 1,
		u8 bp1  : 1,
		u8 bp2  : 1,
		u8 tb   : 1,
		u8 sec  : 1,
		u8 srp0 : 1,
	};
	u8 reg_value;
} winbond_sr1_t;

typedef union {
	struct  {
		u8 srp1 : 1,
		u8 qe   : 1,
		u8 rsvd : 1,
		u8 lb1  : 1,
		u8 lb2  : 1,
		u8 lb3  : 1,
		u8 cmp  : 1,
		u8 sus : 1,
	};
	u8 reg_value;
} winbond_sr2_t;

#define PRINT_RANGE(x, range, cond) \
	printf("%2d) Protected range #range%s\n", x, \
		((#cond) ? " (currently enabled)" : "");

static void print_block_protect_status1(void)
{
	winbond_sr1_t sr1;
	winbond_sr2_t sr2;

	if (!spi_cmd_read(CMD_W25_RDSR1, &sr1.reg_value, 1)) {
		printf("SPI status register 1 read failed!\n");
		return;
	}

	if (!spi_cmd_read(CMD_W25_RDSR2, &sr2.reg_value, 1)) {
		printf("SPI status register 2 read failed!\n");
		return;
	}

	u8 bp_bits = sr1.bp0 | (sr1.bp1 << 1) | (sr1.bp2 << 2);

	PRINT_RANGE(1, 000000h – 000000h, !sr2.cmp && (bp_bits == 0));
	PRINT_RANGE(2, 7E0000h – 7FFFFFh,
		    !sr2.cmp && (bp_bits == 1) && !sr1.sec && !sr1.tb));
	PRINT_RANGE(3, 7C0000h – 7FFFFFh,
		    !sr2.cmp && (bp_bits == 2) && !sr1.sec && !sr1.tb));
	PRINT_RANGE(4, 780000h – 7FFFFFh,
		    !sr2.cmp && (bp_bits == 3) && !sr1.sec && !sr1.tb));
	PRINT_RANGE(5, 700000h – 7FFFFFh,
		    !sr2.cmp && (bp_bits == 4) && !sr1.sec && !sr1.tb));
	PRINT_RANGE(6, 600000h – 7FFFFFh,
		    !sr2.cmp && (bp_bits == 5) && !sr1.sec && !sr1.tb));
	PRINT_RANGE(7, 400000h – 7FFFFFh,
		    !sr2.cmp && (bp_bits == 6) && !sr1.sec && !sr1.tb));
	PRINT_RANGE(8, 000000h – 01FFFFh,
		    !sr2.cmp && (bp_bits == 1) && !sr1.sec && sr1.tb));
	PRINT_RANGE(9, 000000h – 03FFFFh,
		    !sr2.cmp && (bp_bits == 2) && !sr1.sec && sr1.tb));
	PRINT_RANGE(10, 000000h – 07FFFFh,
		    !sr2.cmp && sr1.bp2 && !sr1.bp1 && !sr1.sec && sr1.tb));
	PRINT_RANGE(11, 000000h – 0FFFFFh,
		    !sr2.cmp && (bp_bits == 4) && !sr1.sec && sr1.tb));
	PRINT_RANGE(12, 000000h – 1FFFFFh,
		    !sr2.cmp && (bp_bits == 5) && !sr1.sec && sr1.tb));
	PRINT_RANGE(13, 000000h – 3FFFFFh,
		    !sr2.cmp && (bp_bits == 6) && !sr1.sec && sr1.tb));
	PRINT_RANGE(14, 000000h – 7FFFFFh, !sr2.cmp && (bp_bits == 7));
	PRINT_RANGE(15, 7FF000h – 7FFFFFh,
		    !sr2.cmp && (bp_bits == 1) && sr1.sec && !sr1.tb));
	PRINT_RANGE(16, 7FE000h – 7FFFFFh,
		    !sr2.cmp && (bp_bits == 2) && sr1.sec && !sr1.tb));
	PRINT_RANGE(17, 7FC000h – 7FFFFFh,
		    !sr2.cmp && (bp_bits == 3) && sr1.sec && !sr1.tb));
	PRINT_RANGE(18, 7F8000h – 7FFFFFh,
		    !sr2.cmp && sr1.bp2 && !sr1.bp1 && sr1.sec && !sr1.tb));
	PRINT_RANGE(19, 000000h – 000FFFh, 
		    sr2.cmp && sr1.sec && sr1.tb && (bp_bits == 1)));
	PRINT_RANGE(20, 000000h – 001FFFh,
		    !sr2.cmp && sr1.sec && sr1.tb && (bp_bits == 2)));
	PRINT_RANGE(21, 000000h – 003FFFh,
		    !sr2.cmp && (bp_bits == 3) && sr1.sec && sr1.tb));
	PRINT_RANGE(22, 000000h – 007FFFh,
		    !sr2.cmp && (bp_bits == 4) && sr1.sec && sr1.tb));
}

static void print_block_protect_status2(void)
{
	winbond_sr1_t sr1;
	winbond_sr2_t sr2;

	if (!spi_cmd_read(CMD_W25_RDSR1, &sr1.reg_value, 1)) {
		printf("SPI status register 1 read failed!\n");
		return;
	}

	if (!spi_cmd_read(CMD_W25_RDSR2, &sr2.reg_value, 1)) {
		printf("SPI status register 2 read failed!\n");
		return;
	}

	u8 bp_bits = sr1.bp0 | (sr1.bp1 << 1) | (sr1.bp2 << 2);

	PRINT_RANGE(23, 000000h – 7FFFFFh, sr2.cmp && (bp_bits == 0));
	PRINT_RANGE(24, 000000h – 7DFFFFh,
		    sr2.cmp && (bp_bits == 1) && !sr1.sec && !sr1.tb));
	PRINT_RANGE(25, 000000h – 7BFFFFh,
		    sr2.cmp && (bp_bits == 2) && !sr1.sec && !sr1.tb));
	PRINT_RANGE(26, 000000h – 77FFFFFh,
		    sr2.cmp && (bp_bits == 3) && !sr1.sec && !sr1.tb));
	PRINT_RANGE(27, 000000h – 6FFFFFh,
		    sr2.cmp && (bp_bits == 4) && !sr1.sec && !sr1.tb));
	PRINT_RANGE(28, 000000h – 5FFFFFh,
		    sr2.cmp && (bp_bits == 5) && !sr1.sec && !sr1.tb));
	PRINT_RANGE(29, 000000h – 3FFFFFh,
		    sr2.cmp && (bp_bits == 6) && !sr1.sec && !sr1.tb));
	PRINT_RANGE(30, 020000h – 7FFFFFh,
		    sr2.cmp && (bp_bits == 1) && !sr1.sec && sr1.tb));
	PRINT_RANGE(31, 040000h – 7FFFFFh,
		    sr2.cmp && (bp_bits == 2) && !sr1.sec && sr1.tb));
	PRINT_RANGE(32, 080000h – 7FFFFFh,
		    sr2.cmp && (bp_bits == 3) && !sr1.sec && sr1.tb));
	PRINT_RANGE(33, 100000h – 7FFFFFh,
		    sr2.cmp && (bp_bits == 4) && !sr1.sec && sr1.tb));
	PRINT_RANGE(34, 200000h – 7FFFFFh,
		    sr2.cmp && (bp_bits == 5) && !sr1.sec && sr1.tb));
	PRINT_RANGE(35, 400000h – 7FFFFFh,
		    sr2.cmp && (bp_bits == 6) && !sr1.sec && sr1.tb));
	PRINT_RANGE(36, 000000h – 000000h, sr2.cmp && (bp_bits == 7));
	PRINT_RANGE(37, 000000h – 7FEFFFh,
		    sr2.cmp && (bp_bits == 1) && sr1.sec && !sr1.tb));
	PRINT_RANGE(38, 000000h – 7FDFFFh,
		    sr2.cmp && (bp_bits == 2) && sr1.sec && !sr1.tb));
	PRINT_RANGE(39, 000000h – 7FBFFFh,
		    sr2.cmp && (bp_bits == 3) && sr1.sec && !sr1.tb));
	PRINT_RANGE(40, 000000h – 7F7FFFh,
		    sr2.cmp && sr1.bp2 && !sr1.bp1 && sr1.sec && !sr1.tb));
	PRINT_RANGE(41, 001000h – 7FFFFFh,
		    sr2.cmp && (bp_bits == 1)&& sr1.sec && sr1.tb));
	PRINT_RANGE(42, 002000h – 7FFFFFh,
		    sr2.cmp && (bp_bits == 2) && sr1.sec && sr1.tb));
	PRINT_RANGE(43, 004000h – 7FFFFFh,
		    sr2.cmp && (bp_bits == 3) && sr1.sec && sr1.tb));
	PRINT_RANGE(44, 008000h – 7FFFFFh,
		    sr2.cmp && sr1.bp2 && !sr1.bp1 && sr1.sec && sr1.tb));
}

/* Return WP pin state HIGH = 1 (inactive) or LOW = 0 (active) */
static int get_hw_protect_state(void)
{
	winbond_sr1_t sr1;
	winbond_sr2_t sr2;

	spi_cmd_read(CMD_W25_RDSR1, &sr1.reg_value, 1);
	spi_cmd_read(CMD_W25_RDSR2, &sr2.reg_value, 1);

	/* SRP0 not set, WP pin state does not matter */
	if (!sr1.srp0)
		return 1;

	/* Try clearing SRP0 bit, if success WP pin is high, else low */
	sr1.srp0 = 0;
	u8 status_regs[2] = { sr1.reg_value, sr2.reg_value };

	send_flash_cmd(CMD_W25_WREN, NULL, 0);
	send_flash_cmd_write(CMD_W25_WRSR, 1, status_regs, 2);
	spi_cmd_read(CMD_W25_RDSR1, &sr1.reg_value, 1);

	/* SRP0 did not change, return that WP pin is active */
	if (sr1.srp0)
		return 0;

	/* SRP0 cleared, WP pin inactive */
	sr1.srp0 = 1;
	status_regs[0] = sr1.reg_value;
	send_flash_cmd(CMD_W25_WREN, NULL, 0);
	send_flash_cmd_write(CMD_W25_WRSR, 1, status_regs, 2);
	spi_cmd_read(CMD_W25_RDSR1, &sr1.reg_value, 1);

	/* Failsafe check, if state restored, return WP active, else error */
	if(sr1.srp0)
		return 1;
	else
		return -1;
}

static void print_sr_lock_status(void)
{
	winbond_sr1_t sr1;
	winbond_sr2_t sr2;

	u8 wp_pin = get_hw_protect_state();
	if (wp_pin == -1)
		printf("Error in HW protect state\n");

	if (!spi_cmd_read(CMD_W25_RDSR1, &sr1.reg_value, 1)) {
		printf("SPI status register 1 read failed!\n");
		return;
	}

	if (!spi_cmd_read(CMD_W25_RDSR2, &sr2.reg_value, 1)) {
		printf("SPI status register 2 read failed!\n");
		return;
	}

	printf("SRP0=%d , SRP1=%d, WP=%c\n", sr1.srp0, sr2.srp1,
	       ((wp_pin == -1) || !sr1.srp0) ? '?' : wp_pin + '0');
	printf("1) Status register is %sin Software Protected mode\n%s",
	       (!sr1.srp0 && !sr2.srp1) ? "" : "NOT ",
	       !sr1.srp0 ? " WP pin may be active.\n" : "");
	printf("2) Status register is %sin Hardware Protected mode\n",
	       (sr1.srp0 && !sr2.srp1 && (wp_pin == 0) ? "" : "NOT ");
	printf("3) Status register is %sin Hardware Unprotected mode\n",
	       (sr1.srp0 && !sr2.srp1 && (wp_pin == 1)) ? "" : "NOT ");
	printf("4) Status register is %sin Power Supply Lock-Down mode\n",
	       (!sr1.srp0 && sr2.srp1) ? "" : "NOT ");
	printf("5) Status register is %sin One Time Program mode\n",
	       (sr1.srp0 && sr2.srp1) ? "" : "NOT ");
}

static void clear_block_protection(void)
{
	winbond_sr1_t sr1;
	winbond_sr2_t sr2;

	if (!spi_cmd_read(CMD_W25_RDSR1, &sr1.reg_value, 1)) {
		printf("SPI status register 1 read failed!\n");
		return;
	}

	if (!spi_cmd_read(CMD_W25_RDSR2, &sr2.reg_value, 1)) {
		printf("SPI status register 2 read failed!\n");
		return;
	}

	sr1.bp0 = 0;
	sr1.bp1 = 0;
	sr1.bp2 = 0;
	sr1.tb  = 0;
	sr1.sec = 0;
	sr2.cmp = 0;

	u8 status_regs[2] = { sr1.reg_value, sr2.reg_value };

	if (send_flash_cmd(CMD_W25_WREN, NULL, 0)) {
		printf("Sending write enable command failed!\n");
		printf("Clearing block protection failed!\n");
		return;
	}

	if (send_flash_cmd_write(CMD_W25_WRSR, 1, status_regs, 2)) {
		printf("Writing status registers failed!\n");
		printf("Clearing block protection failed!\n");
		return;
	}

	if (!spi_cmd_read(CMD_W25_RDSR1, &sr1.reg_value, 1)) {
		printf("SPI status register 1 read failed!\n");
		return;
	}

	if (!spi_cmd_read(CMD_W25_RDSR2, &sr2.reg_value, 1)) {
		printf("SPI status register 2 read failed!\n");
		return;
	}

	if (sr1.bp0 || sr1.bp1 || sr1.bp2 || sr1.tb || sr1.sec || sr2.cmp)
		printf("Clearing block protection failed!\n");
	else 
		printf("Clearing block protection success!\n");
}

static u8 bp_lookup[] = {
	// CMP = 0
	0x00, 0x04, 0x08, 0x0C, 0x10, 0x14, 0x18,	// SEC=0, TB=0
	0x24, 0x28, 0x2C, 0x30, 0x34, 0x38, 0x0E,	// SEC=0, TB=1
	0x44, 0x48, 0x4C, 0x50,				// SEC=1, TB=0
	0x64, 0x68, 0x6C, 0x70,				// SEC=1, TB=1
	// CMP = 1
	0x00, 0x04, 0x08, 0x0C, 0x10, 0x14, 0x18,	// SEC=0, TB=0
	0x24, 0x28, 0x2C, 0x30, 0x34, 0x38, 0x0E,	// SEC=0, TB=1
	0x44, 0x48, 0x4C, 0x50,				// SEC=1, TB=0
	0x64, 0x68, 0x6C, 0x70,				// SEC=1, TB=1
}

static void set_block_protection(char* command)
{
	int choice;
	winbond_sr1_t sr1;
	winbond_sr2_t sr2;
	char delim[] = " ";
	char *ptr = strtok(command, delim);

	if (ptr[0] == 'b')
		ptr = strtok(NULL, delim);
	else {
		printf("Command error\n");
		return;
	}

	if (ptr != NULL)
		choice = atol(ptr);
	else {
		printf("Command error\n");
		return;
	}

	if (choice < 1 && choice > 44)
		printf("Invalid lock option\n");
		return;

	if (!spi_cmd_read(CMD_W25_RDSR1, &sr1.reg_value, 1)) {
		printf("SPI status register 1 read failed!\n");
		return;
	}

	if (!spi_cmd_read(CMD_W25_RDSR2, &sr2.reg_value, 1)) {
		printf("SPI status register 2 read failed!\n");
		return;
	}

	if (choice > 22)
		sr2.cmp = 1;
	else
		sr2.cmp = 0;
	
	sr1.reg_value &= 0x83; // clear all BPs, SEC and TB
	sr1.reg_value |= bp_lookup[choice]; // set required bits

	u8 status_regs[2] = { sr1.reg_value, sr2.reg_value };

	if (send_flash_cmd(CMD_W25_WREN, NULL, 0)) {
		printf("Sending write enable command failed!\n");
		printf("Setting block protection failed!\n");
		return;
	}

	if (send_flash_cmd_write(CMD_W25_WRSR, 1, status_regs, 2)) {
		printf("Writing status registers failed!\n");
		printf("Setting block protection failed!\n");
		return;
	}

	if (!spi_cmd_read(CMD_W25_RDSR1, &sr1.reg_value, 1)) {
		printf("SPI status register 1 read failed!\n");
		return;
	}

	if (!spi_cmd_read(CMD_W25_RDSR2, &sr2.reg_value, 1)) {
		printf("SPI status register 2 read failed!\n");
		return;
	}

	if(status_regs[0] != sr1.reg_value &&
	   status_regs[1] != sr2.reg_value) {
		printf("Setting block protection failed!\n");
	} else {
		printf("Setting block protection success!\n");
	}
}


static void set_sr_lock(char* command)
{
	int choice;
	winbond_sr1_t sr1;
	winbond_sr2_t sr2;
	char delim[] = " ";
	char *ptr = strtok(command, delim);

	if (ptr[0] == 'l')
		ptr = strtok(NULL, delim);
	else {
		printf("Command error\n");
		return;
	}

	if (ptr != NULL)
		choice = atol(ptr);
	else {
		printf("Command error\n");
		return;
	}

	if (choice < 1 && choice > 5)
		printf("Invalid lock option\n");
		return;

	if (!spi_cmd_read(CMD_W25_RDSR1, &sr1.reg_value, 1)) {
		printf("SPI status register 1 read failed!\n");
		return;
	}

	if (!spi_cmd_read(CMD_W25_RDSR2, &sr2.reg_value, 1)) {
		printf("SPI status register 2 read failed!\n");
		return;
	}

	switch(choice)
	{
		case 0: sr1.srp0 = 0; sr2.srp1 = 0; break;
		case 1: sr1.srp0 = 0; sr2.srp1 = 0; break;
		case 2: sr1.srp0 = 0; sr2.srp1 = 0;
			printf("Check WP pin.\n");
			break;
		case 3: sr1.srp0 = 0; sr2.srp1 = 0;
			printf("Check WP pin.\n");
			break;
		case 4: sr1.srp0 = 0; sr2.srp1 = 1; break;
		case 5: sr1.srp0 = 1; sr2.srp1 = 1; break;
		default: printf("Invalid lock option\n"); return;
	}

	if (sr1.srp0 && sr2.srp1) {
		printf("Warning: You are going to permanently lock status\n"
		       "register. Are You sure? (y/n)\n");
		char *command = readline("> ");
		if (command[0] != 'y')
			return;
	}

	u8 status_regs[2] = { sr1.reg_value, sr2.reg_value };

	if (send_flash_cmd(CMD_W25_WREN, NULL, 0)) {
		printf("Sending write enable command failed!\n");
		printf("Setting status register protection failed!\n");
		return;
	}

	if (send_flash_cmd_write(CMD_W25_WRSR, 1, status_regs, 2)) {
		printf("Writing status registers failed!\n");
		printf("Setting status register protection failed!\n");
		return;
	}

	if (!spi_cmd_read(CMD_W25_RDSR1, &sr1.reg_value, 1)) {
		printf("SPI status register 1 read failed!\n");
		return;
	}

	if (!spi_cmd_read(CMD_W25_RDSR2, &sr2.reg_value, 1)) {
		printf("SPI status register 2 read failed!\n");
		return;
	}

	if(status_regs[0] != sr1.reg_value &&
	   status_regs[1] != sr2.reg_value) {
		printf("Setting status register protection failed!\n");
	} else {
		printf("Setting status register protection success!\n");
	}

}

static void print_spi_lock_menu(void) {
	printf("\n\n--- SPI flash lock menu ---\n\n");
	printf("  p		- print block protection status (CMP=0)\n");
	printf("  r		- print block protection status (CMP=1)\n");
	printf("  b block_no	- set block protection (see status)\n");
	printf("  c		- clear block protection\n");
	printf("  s		- show status register lock (see status)\n");
	printf("  l lock_type	- lock status register\n"); */
	printf("  q		- exit menu\n");
	printf("Choose an option:\n");
}

void handle_spi_lock_menu(void) {
	bool end = FALSE;
	char *command;

	while(1) {
		print_spi_lock_menu();
		command = readline("> ");

		switch(command[0]) {
		case 'p':
			print_block_protect_status();
			break;
		case 'b':
			set_block_protection(command);
			break;
		case 'c':
			clear_block_protection();
			break;
		case 's':
			print_sr_lock_status();
			break;
		case 'l':
			set_sr_lock(command);
			break;
		case 'q':
			end = TRUE;
			break;
		default:
			printf("wrong command: '%s'\n", command);
			break;
		}

		command[0] = '\0';

		if (end)
			break;
	}
}
