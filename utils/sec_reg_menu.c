/*
 * Copyright (C) 2017 PC Engines GmbH
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
#include <sec_reg_menu.h>

#define SERIAL_REG_NO	1
#define SERIAL_OFFSET	0
#define MAX_SERIAL_LEN	10

/* Locking functionality n/a due to not working hardware lock procedure */

static void print_reg_sec_menu(void) {
	printf("\n\n--- Security registers menu ---\n\n");
	printf("  r        - read serial from security register 1\n");
	printf("  w serial - write serial to security register 1\n");
	printf("  s        - get security registers OTP status\n");
#if 0
	printf("  l reg    - lock security register reg\n");
#endif
	printf("  q        - exit menu\n");
	printf("\n");
}

static void cmd_read_serial(void)
{
	u8 reg = SERIAL_REG_NO;
	u8 offset = SERIAL_OFFSET;
	u8 buf[MAX_SERIAL_LEN] = { 0 };
	int ret;

	ret = read_sec(reg, offset, buf, sizeof(buf));
	if (ret) {
		printf("can't read register\n");
		return;
	}

	printf("serial: %s\n", buf);
}

static void cmd_write_serial(char *cmd)
{
	u8 reg = SERIAL_REG_NO;
	u8 offset = SERIAL_OFFSET;
	u8 buf[MAX_SERIAL_LEN] = { 0 };
	int ret;

	strncpy((char *)buf, cmd+2, sizeof(buf)-1);

	ret = prog_sec(reg, offset, buf, sizeof(buf));
	if (ret) {
		printf("can't write to register\n");
		return;
	}

	printf("serial written\n");
}

static void cmd_read_sec_sts(void)
{
	int status = read_sec_status();

	printf("Security registers status:\n");
	printf("  reg 1 = %s\n", (status & 0x1) ? "locked" : "writeable");
	printf("  reg 2 = %s\n", (status & 0x2) ? "locked" : "writeable");
	printf("  reg 3 = %s\n", (status & 0x4) ? "locked" : "writeable");
}


static void cmd_erase_sec(void)
{
	u8 reg = SERIAL_REG_NO;
	u8 offset = SERIAL_OFFSET;
	int ret;

	ret = erase_sec(reg, offset, MAX_SERIAL_LEN);
	if (ret) {
		printf("can't erase security registers\n");
		return;
	}

	printf("serial erased\n");
}

static void cmd_lock_sec(char *cmd)
{
	u8 reg;
	int ret;

	reg = strtoul(cmd+2, NULL, 10);

	if (reg < 1 || reg > 3) {
		printf("Wrong register number!\n");
		return;
	}

	printf("Warning! This will permamently lock the security register %d."
		" Are you sure? (yes/no)\n", reg);

	cmd[0] = '\0';
	cmd = readline("> ");

	if (strcmp(cmd, "yes") != 0) {
		return;
	}

	ret = lock_sec(reg);
	printf("%s\n", ret ? "Can't lock!" : "Locked!");

	return;
}

void handle_reg_sec_menu(void) {
	bool end = FALSE;
	char *command;

	print_reg_sec_menu();

	while(1) {
		command = readline("> ");

		switch(command[0]) {
		case 'r':
			cmd_read_serial();
			break;
		case 'w':
			cmd_write_serial(command);
			break;
		case 'e':
		        cmd_erase_sec();
			break;
		case 's':
			cmd_read_sec_sts();
			break;
#if 0
		case 'l':
			cmd_lock_sec(command);
			break;
#endif
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
