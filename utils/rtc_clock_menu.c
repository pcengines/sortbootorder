/*
 * Copyright (C) 2022 PC Engines GmbH
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

static void print_rtc_clock_menu(struct tm *t) {
	printf("\n\n--- Clock menu ---\n");
	printf("  Date: %04d-%02d-%02d\n", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
	printf("  Time: %02d:%02d:%02d\n", t->tm_hour, t->tm_min, t->tm_sec);
	printf("  Y year   - Set year\n");
	printf("  M month  - Set month\n");
	printf("  D day    - Set day\n");
	printf("  h hour   - Set hour\n");
	printf("  m minute - Set minute\n");
	printf("  s second - Set second\n");
	printf("  w - Write and exit\n");
	printf("  x - Exit without writing\n");
	printf("\n");
}

static void handle_year_command(struct tm *t, char *command) {
	int year;

	year = strtoul(command+2, NULL, 10);
	if (year >= 1900 && year <= 2099)
		t->tm_year = year - 1900;
	else
		printf("Please enter a year between 1900 and 2099\n");
}

static void handle_mon_command(struct tm *t, char *command) {
	int mon;

	mon = strtoul(command+2, NULL, 10);
	if (mon >= 1 && mon <= 12)
		t->tm_mon = mon - 1;
	else
		printf("Please enter a month between 1 and 12\n");
}

static void handle_day_command(struct tm *t, char *command) {
	int day;

	day = strtoul(command+2, NULL, 10);
	if (day >= 1 && day <= 31)
		t->tm_mday = day;
	else
		printf("Please enter a day between 1 and 31\n");
}

static void handle_hour_command(struct tm *t, char *command) {
	int hour;

	hour = strtoul(command+2, NULL, 10);
	if (hour >= 0 && hour <= 23)
		t->tm_hour = hour;
	else
		printf("Please enter hours between 0 and 23\n");
}

static void handle_min_command(struct tm *t, char *command) {
	int min;

	min = strtoul(command+2, NULL, 10);
	if (min >= 0 && min <= 59)
		t->tm_min = min;
	else
		printf("Please enter minutes between 0 and 59\n");
}

static void handle_sec_command(struct tm *t, char *command) {
	int sec;

	sec = strtoul(command+2, NULL, 10);
	if (sec >= 0 && sec <= 59)
		t->tm_sec = sec;
	else
		printf("Please enter seconds between 0 and 59\n");
}

void handle_rtc_clock_menu(void) {
	struct tm time;
	bool end = FALSE;
	char *command;

	rtc_read_clock(&time);

	while(1) {
		print_rtc_clock_menu(&time);
		command = readline("> ");

		switch(command[0]) {
		case 'Y':
			handle_year_command(&time, command);
			break;
		case 'M':
			handle_mon_command(&time, command);
			break;
		case 'D':
			handle_day_command(&time, command);
			break;
		case 'h':
			handle_hour_command(&time, command);
			break;
		case 'm':
			handle_min_command(&time, command);
			break;
		case 's':
			handle_sec_command(&time, command);
			break;
		case 'w':
			rtc_write_clock(&time);
		case 'x':
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
