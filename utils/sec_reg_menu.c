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
/*	printf("  l reg    - lock security register reg\n"); */
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
		case 's':
			cmd_read_sec_sts();
			break;
		case 'l':
			cmd_lock_sec(command);
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
