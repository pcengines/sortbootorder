#ifndef FLASH_ACCESS_H
#define FLASH_ACCESS_H

#include <stdint.h>

#define MAX_DEVICES        64
#define MAX_LENGTH         64
#define NEWLINE            0x0A
#define NUL                0x00

int init_flash(void);
int is_flash_locked(void);
int lock_flash(void);
int unlock_flash(void);
int read_sec_status(void);
int read_sec(u8 reg, u8 addr, void *buf, size_t len);
int prog_sec(u8 reg, u8 addr, const void *buf, size_t len);
void save_flash(int flash_address, char buffer[MAX_DEVICES][MAX_LENGTH], u8 max_lines, u8 spi_wp_toggle);

#endif
