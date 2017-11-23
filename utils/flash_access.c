#define CONFIG_SPI_FLASH_NO_FAST_READ

#include <spi/spi_flash.h>
#include <flash_access.h>

#define FLASH_SIZE_CHUNK   0x1000 //4k

static struct spi_flash *flash_device;

/*******************************************************************************/
inline int init_flash(void)
{
	flash_device = spi_flash_probe(0, 0, 0, 0);

	if (!flash_device)
		return -1;

	return 0;
}

/*******************************************************************************/
inline int is_flash_locked(void)
{
	return spi_flash_is_locked(flash_device);
}

/*******************************************************************************/
inline int lock_flash(void)
{
	return spi_flash_lock(flash_device);
}

/*******************************************************************************/
inline int unlock_flash(void)
{
	return spi_flash_unlock(flash_device);
}

inline int read_sec_status(void)
{
	return spi_flash_sec_sts(flash_device);
}

inline int read_sec(u8 reg, u8 addr, void *buf, size_t len)
{
	u32 offset = reg;
	offset <<= 12;
	offset |= addr;

	return spi_flash_sec_read(flash_device, offset, len, buf);
}

inline int prog_sec(u8 reg, u8 addr, const void *buf, size_t len)
{
	u32 offset = reg;
	offset <<= 12;
	offset |= addr;

	return spi_flash_sec_prog(flash_device, offset, len, buf);
}

inline int lock_sec(u8 reg)
{
	return spi_flash_sec_lock(flash_device, reg);
}

/*******************************************************************************/
void save_flash(int flash_address, char buffer[MAX_DEVICES][MAX_LENGTH], u8 max_lines, u8 spi_wp_toggle) {
	int i = 0;
	int k = 0;
	int j, ret;
	char cbfs_formatted_list[MAX_DEVICES * MAX_LENGTH];
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

	// try to unlock the flash if it is locked
	if (spi_flash_is_locked(flash_device)) {
		spi_flash_unlock(flash_device);
		if (spi_flash_is_locked(flash_device)) {
			printf("Flash is write protected. Exiting...\n");
			return;
		}
	}

	printf("Erasing Flash size 0x%x @ 0x%x\n", FLASH_SIZE_CHUNK, flash_address);
	ret = spi_flash_erase(flash_device, flash_address, FLASH_SIZE_CHUNK);
	if (ret) {
		printf("Erase failed, ret: %d\n", ret);
	}

	printf("Writing %d bytes @ 0x%x\n", i, flash_address);
	// write first 512 bytes
	for (nvram_pos = 0; nvram_pos < (i & 0xFFFC); nvram_pos += 4) {
		ret = spi_flash_write(flash_device, nvram_pos + flash_address, sizeof(u32), (u32 *)(cbfs_formatted_list + nvram_pos));
		if (ret) {
			printf("Write failed, ret: %d\n", ret);
		}
	}
	// write remaining filler characters in one run
	ret = spi_flash_write(flash_device, nvram_pos + flash_address, sizeof(i % 4), (u32 *)(cbfs_formatted_list + nvram_pos));
	if (ret) {
		printf("Write failed, ret: %d\n", ret);
	}

	if (spi_wp_toggle) {
		printf("Enabling flash write protect...\n");
		spi_flash_lock(flash_device);
	}

	spi_wp_toggle = spi_flash_is_locked(flash_device);

	printf("Done\n");
}
