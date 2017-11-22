#include <libpayload.h>
#include <curses.h>
#include <flash_access.h>
#include <sec_reg_menu.h>

static void print_reg_sec_menu(void) {
	printf("\n\n--- security registers menu ---\n\n");
	printf("  r        - read serial from security register 1\n");
	printf("  w serial - write serial to security register 1\n");
	printf("  s        - get security registers OTP status\n");
	printf("  l reg    - lock security register reg\n");
	printf("  q        - exit menu\n");
	printf("\n");
}

static void cmd_read_sec(char *cmd)
{
	u8 reg = 1;
	u8 offset = 0;
	u8 len = 9;
	u8 buf[16] = { 0 };
	int ret;

	ret = read_sec(reg, offset, buf, len);
	if (ret) {
		printf("can't read register\n");
		return;
	}

	printf("serial: %s\n", buf);
}

static void cmd_write_sec(char *cmd)
{
	u8 reg = 1;
	u8 offset = 0;
	u8 buf[16] = { 0 };
	int ret;

	strncpy((char *)buf, cmd, sizeof(buf));

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

static void cmd_lock_sec(void)
{
	printf("not implemented\n");
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
			cmd_read_sec(command + 2);
			break;
		case 'w':
			cmd_write_sec(command + 2);
			break;
		case 's':
			cmd_read_sec_sts();
			break;
		case 'l':
			cmd_lock_sec();
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
