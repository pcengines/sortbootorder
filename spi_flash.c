/*
 * SPI flash interface
 *
 * Copyright (C) 2008 Atmel Corporation
 * Copyright (C) 2010 Reinhard Meyer, EMK Elektronik
 * Copyright (c) 2014 Sage Electronic Engineering, Inc.
 *
 * Licensed under the GPL-2 or later.
 */

//#define SPI_DEBUG

#include <libpayload-config.h>
#include <stdlib.h>
#include <string.h>
#include "spi_flash.h"
#include "spi_flash_internal.h"
#include "spi.h"

static void spi_flash_addr(u32 addr, u8 *cmd)
{
	/* cmd[0] is actual command */
	cmd[1] = addr >> 16;
	cmd[2] = addr >> 8;
	cmd[3] = addr >> 0;
}

int spi_flash_cmd(struct spi_slave *spi, u8 cmd, void *response, size_t len)
{
	int ret = spi_xfer(spi, &cmd, 8, response, len * 8);
	if (ret)
		spi_debug("SF: Failed to send command %02x: %d\n", cmd, ret);

	return ret;
}

int spi_flash_cmd_read(struct spi_slave *spi, const u8 *cmd,
		size_t cmd_len, void *data, size_t data_len)
{
	int ret = spi_xfer(spi, cmd, cmd_len * 8, data, data_len * 8);
	if (ret) {
		spi_debug("SF: Failed to send read command (%u bytes): %d\n",
				data_len, ret);
	}

	return ret;
}

int spi_flash_cmd_write(struct spi_slave *spi, const u8 *cmd, size_t cmd_len,
		const void *data, size_t data_len)
{
	int ret;
	u8 buff[cmd_len + data_len];
	memcpy(buff, cmd, cmd_len);
	memcpy(buff + cmd_len, data, data_len);

	ret = spi_xfer(spi, buff, (cmd_len + data_len) * 8, NULL, 0);
	if (ret) {
		spi_debug("SF: Failed to send write command (%u bytes): %d\n",
				data_len, ret);
	}

	return ret;
}

int spi_flash_read_common(struct spi_flash *flash, const u8 *cmd,
		size_t cmd_len, void *data, size_t data_len)
{
	struct spi_slave *spi = flash->spi;
	int ret;

	spi->rw = SPI_READ_FLAG;
	spi_claim_bus(spi);
	ret = spi_flash_cmd_read(spi, cmd, cmd_len, data, data_len);
	spi_release_bus(spi);

	return ret;
}

int spi_flash_cmd_read_fast(struct spi_flash *flash, u32 offset,
		size_t len, void *data)
{
	struct spi_slave *spi = flash->spi;
	u8 cmd[5];

	cmd[0] = CMD_READ_ARRAY_FAST;
	spi_flash_addr(offset, cmd);
	cmd[4] = 0x00;

	return spi_flash_cmd_read(spi, cmd, sizeof(cmd), data, len);
}

int spi_flash_cmd_read_slow(struct spi_flash *flash, u32 offset,
		size_t len, void *data)
{
	struct spi_slave *spi = flash->spi;
	u8 cmd[4];

	cmd[0] = CMD_READ_ARRAY_SLOW;
	spi_flash_addr(offset, cmd);

	return spi_flash_cmd_read(spi, cmd, sizeof(cmd), data, len);
}

int spi_flash_cmd_poll_bit(struct spi_flash *flash, unsigned long timeout,
			   u8 cmd, u8 poll_bit)
{
	struct spi_slave *spi = flash->spi;
	unsigned long timebase;
	int ret;
	u8 status;

	timebase = timeout * 500;
	do {
		ret = spi_flash_cmd_read(spi, &cmd, 1, &status, 1);
		if (ret)
			return -1;

		if ((status & poll_bit) == 0)
			break;

		udelay(1);
	} while (timebase--);

	if ((status & poll_bit) == 0)
		return 0;

	/* Timed out */
	spi_debug("SF: time out!\n");
	return -1;
}

int spi_flash_cmd_wait_ready(struct spi_flash *flash, unsigned long timeout)
{
	return spi_flash_cmd_poll_bit(flash, timeout,
		CMD_READ_STATUS, STATUS_WIP);
}

int spi_flash_cmd_erase(struct spi_flash *flash, u8 erase_cmd,
			u32 offset, size_t len)
{
	u32 start, end, erase_size;
	int ret;
	u8 cmd[4];

	erase_size = flash->sector_size;
	if (offset % erase_size || len % erase_size) {
		spi_debug("SF: Erase offset/length not multiple of erase size\n");
		return -1;
	}

	flash->spi->rw = SPI_WRITE_FLAG;
	ret = spi_claim_bus(flash->spi);
	if (ret) {
		spi_debug("SF: Unable to claim SPI bus\n");
		return ret;
	}

	cmd[0] = erase_cmd;
	start = offset;
	end = start + len;

	while (offset < end) {
		spi_flash_addr(offset, cmd);
		offset += erase_size;

		spi_debug("SF: erase %2x %2x %2x %2x (%x)\n", cmd[0], cmd[1],
		      cmd[2], cmd[3], offset);

		ret = spi_flash_cmd(flash->spi, CMD_WRITE_ENABLE, NULL, 0);
		if (ret)
			goto out;

		ret = spi_flash_cmd_write(flash->spi, cmd, sizeof(cmd), NULL, 0);
		if (ret)
			goto out;

		ret = spi_flash_cmd_wait_ready(flash, SPI_FLASH_PAGE_ERASE_TIMEOUT);
		if (ret)
			goto out;
	}

	spi_debug("SF: Successfully erased %u bytes @ %#x\n", len, start);

out:
	spi_release_bus(flash->spi);
	return ret;
}

/*
 * The following table holds all device probe functions
 *
 * shift:  number of continuation bytes before the ID
 * idcode: the expected IDCODE or 0xff for non JEDEC devices
 * probe:  the function to call
 *
 * Non JEDEC devices should be ordered in the table such that
 * the probe functions with best detection algorithms come first.
 *
 * Several matching entries are permitted, they will be tried
 * in sequence until a probe function returns non NULL.
 *
 * IDCODE_CONT_LEN may be redefined if a device needs to declare a
 * larger "shift" value.  IDCODE_PART_LEN generally shouldn't be
 * changed.  This is the max number of bytes probe functions may
 * examine when looking up part-specific identification info.
 *
 * Probe functions will be given the idcode buffer starting at their
 * manu id byte (the "idcode" in the table below).  In other words,
 * all of the continuation bytes will be skipped (the "shift" below).
 */
#define IDCODE_CONT_LEN 0
#define IDCODE_PART_LEN 5
static struct {
	const u8 shift;
	const u8 idcode;
	struct spi_flash *(*probe) (struct spi_slave *spi, u8 *idcode);
} flashes[] = {
	/* Keep it sorted by define name */
	{ 0, 0x1c, spi_flash_probe_eon, },
	{ 0, 0xc8, spi_flash_probe_gigadevice, },
	{ 0, 0xc2, spi_flash_probe_macronix, },
	{ 0, 0x01, spi_flash_probe_spansion, },
	{ 0, 0xbf, spi_flash_probe_sst, },
	{ 0, 0x20, spi_flash_probe_stmicro, },
	{ 0, 0xef, spi_flash_probe_winbond, },
	/* Keep it sorted by best detection */
	{ 0, 0xff, spi_flash_probe_stmicro, },
};
#define IDCODE_LEN (IDCODE_CONT_LEN + IDCODE_PART_LEN)

struct spi_flash *spi_flash_probe(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int spi_mode)
{
	struct spi_slave *spi;
	struct spi_flash *flash = NULL;
	int ret, i, shift;
	u8 idcode[IDCODE_LEN], *idp;

	spi = spi_setup_slave(bus, cs, max_hz, spi_mode);
	if (!spi) {
		spi_debug("SF: Failed to set up slave\n");
		return NULL;
	}

	spi->rw = SPI_READ_FLAG;
	ret = spi_claim_bus(spi);
	if (ret) {
		spi_debug("SF: Failed to claim SPI bus: %d\n", ret);
		goto err_claim_bus;
	}

	/* Read the ID codes */
	ret = spi_flash_cmd(spi, CMD_READ_ID, idcode, sizeof(idcode));
	if (ret)
		goto err_read_id;

	spi_debug("SF: Got idcodes\n");

	/* count the number of continuation bytes */
	for (shift = 0, idp = idcode;
	     shift < IDCODE_CONT_LEN && *idp == 0x7f;
	     ++shift, ++idp)
		continue;

	/* search the table for matches in shift and id */
	for (i = 0; i < ARRAY_SIZE(flashes); ++i)
		if (flashes[i].shift == shift && flashes[i].idcode == *idp) {

			/* we have a match, call probe */
			flash = flashes[i].probe(spi, idp);
			if (flash)
				break;
		}

	if (!flash) {
		spi_debug("SF: Unsupported manufacturer %02x\n", *idp);
		goto err_manufacturer_probe;
	}

	spi_debug("SF: Detected %s with page size %x, total %x\n",
			flash->name, flash->sector_size, flash->size);

	spi_release_bus(spi);

	return flash;

err_manufacturer_probe:
err_read_id:
	spi_release_bus(spi);
err_claim_bus:
	return NULL;
}
