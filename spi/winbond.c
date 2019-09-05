/*
 * Copyright 2008, Network Appliance Inc.
 * Author: Jason McMullan <mcmullan <at> netapp.com>
 *
 * Copyright (c) 2014 Sage Electronic Engineering, Inc.
 *
 * Licensed under the GPL-2 or later.
 */

//#define SPI_DEBUG

#include <stdlib.h>
#include <spi/spi_flash.h>
#include <spi/spi.h>
#include <spi/winbond_flash.h>
#include <spi/spi_flash_internal.h>

struct winbond_spi_flash_params {
	uint16_t	id;
	/* Log2 of page size in power-of-two mode */
	uint8_t		l2_page_size;
	uint16_t	pages_per_sector;
	uint16_t	sectors_per_block;
	uint16_t	nr_blocks;
	const char	*name;
};

/* spi_flash needs to be first so upper layers can free() it */
struct winbond_spi_flash {
	struct spi_flash flash;
	const struct winbond_spi_flash_params *params;
};

static inline struct winbond_spi_flash *
to_winbond_spi_flash(struct spi_flash *flash)
{
	return container_of(flash, struct winbond_spi_flash, flash);
}

static const struct winbond_spi_flash_params winbond_spi_flash_table[] = {
	{
		.id			= 0x3015,
		.l2_page_size		= 8,
		.pages_per_sector	= 16,
		.sectors_per_block	= 16,
		.nr_blocks		= 32,
		.name			= "W25X16",
	},
	{
		.id			= 0x3016,
		.l2_page_size		= 8,
		.pages_per_sector	= 16,
		.sectors_per_block	= 16,
		.nr_blocks		= 64,
		.name			= "W25X32",
	},
	{
		.id			= 0x3017,
		.l2_page_size		= 8,
		.pages_per_sector	= 16,
		.sectors_per_block	= 16,
		.nr_blocks		= 128,
		.name			= "W25X64",
	},
	{
		.id			= 0x4015,
		.l2_page_size		= 8,
		.pages_per_sector	= 16,
		.sectors_per_block	= 16,
		.nr_blocks		= 32,
		.name			= "W25Q16",
	},
	{
		.id			= 0x4016,
		.l2_page_size		= 8,
		.pages_per_sector	= 16,
		.sectors_per_block	= 16,
		.nr_blocks		= 64,
		.name			= "W25Q32",
	},
	{
		.id			= 0x4017,
		.l2_page_size		= 8,
		.pages_per_sector	= 16,
		.sectors_per_block	= 16,
		.nr_blocks		= 128,
		.name			= "W25Q64",
	},
	{
		.id			= 0x4018,
		.l2_page_size		= 8,
		.pages_per_sector	= 16,
		.sectors_per_block	= 16,
		.nr_blocks		= 256,
		.name			= "W25Q128",
	},
};

static int winbond_write(struct spi_flash *flash, u32 offset, size_t len, const void *buf)
{
	struct winbond_spi_flash *stm = to_winbond_spi_flash(flash);
	unsigned long byte_addr;
	unsigned long page_size;
	size_t chunk_len;
	size_t actual;
	int ret;
	u8 cmd[4];

	page_size = min(1 << stm->params->l2_page_size, CONTROLLER_PAGE_LIMIT);
	byte_addr = offset % page_size;

	flash->spi->rw = SPI_WRITE_FLAG;
	ret = spi_claim_bus(flash->spi);
	if (ret) {
		spi_debug("SF: Unable to claim SPI bus\n");
		return ret;
	}

	for (actual = 0; actual < len; actual += chunk_len) {
		chunk_len = min(len - actual, page_size - byte_addr);

		cmd[0] = CMD_W25_PP;
		cmd[1] = (offset >> 16) & 0xff;
		cmd[2] = (offset >> 8) & 0xff;
		cmd[3] = offset & 0xff;

		spi_debug("PP: 0x%p => cmd = { 0x%02x 0x%02x%02x%02x }"
		        " chunk_len = %u\n", buf + actual,
			cmd[0], cmd[1], cmd[2], cmd[3], chunk_len);

		ret = spi_flash_cmd(flash->spi, CMD_W25_WREN, NULL, 0);
		if (ret < 0) {
			spi_debug("SF: Enabling Write failed\n");
			goto out;
		}

		ret = spi_flash_cmd_write(flash->spi, cmd, 4,
				buf + actual, chunk_len);
		if (ret < 0) {
			spi_debug("SF: Winbond Page Program failed\n");
			goto out;
		}

		ret = spi_flash_cmd_wait_ready(flash, SPI_FLASH_PROG_TIMEOUT);
		if (ret)
			goto out;

		offset += chunk_len;
		byte_addr = 0;
	}

	spi_debug("SF: Winbond: Successfully programmed %u bytes @"
			" 0x%lx\n", len, (unsigned long)(offset - len));

	ret = 0;

out:
	spi_release_bus(flash->spi);
	return ret;
}

static int winbond_erase(struct spi_flash *flash, u32 offset, size_t len)
{
	return spi_flash_cmd_erase(flash, CMD_W25_SE, offset, len);
}

static int winbond_set_lock_flags(struct spi_flash *flash, int lock)
{
	int ret;
	u8 cmd;
	u8 status[2];

	flash->spi->rw = SPI_WRITE_FLAG;
	ret = spi_claim_bus(flash->spi);
	if (ret) {
		spi_debug("SF: Unable to claim SPI bus\n");
		return ret;
	}

	ret = spi_flash_cmd(flash->spi, CMD_W25_RDSR1, &status[0], 1);
	if (ret) {
		spi_debug("SF: Unable to read Status Register 1\n");
		goto out;
	}

	ret = spi_flash_cmd(flash->spi, CMD_W25_RDSR2, &status[1], 1);
	if (ret) {
		spi_debug("SF: Unable to read Status Register 2\n");
		goto out;
	}

	if (lock) {
		status[0] |= REG_W25_SRP0 | REG_W25_BP2 | REG_W25_BP1 | REG_W25_BP0;
	} else {
		status[0] &= ~(REG_W25_SRP0 | REG_W25_BP2 | REG_W25_BP1 | REG_W25_BP0);
	}

	status[0] &= ~(REG_W25_SEC | REG_W25_TB);
	status[1] &= ~(REG_W25_SRP1 | REG_W25_CMP);

	ret = spi_flash_cmd(flash->spi, CMD_W25_WREN, NULL, 0);
	if (ret < 0) {
		spi_debug("SF: Enabling Write failed\n");
		goto out;
	}

	cmd = CMD_W25_WRSR;
	ret = spi_flash_cmd_write(flash->spi, &cmd, sizeof(cmd),
				  status, sizeof(status));
	if (ret < 0) {
		spi_debug("SF: Status register write failed\n");
		goto out;
	}

out:
	spi_release_bus(flash->spi);
	return ret;
}

static int winbond_unlock(struct spi_flash *flash)
{
	return winbond_set_lock_flags(flash, 0);
}

static int winbond_lock(struct spi_flash *flash)
{
	return winbond_set_lock_flags(flash, 1);
}

static int winbond_is_locked(struct spi_flash *flash)
{
	u8 status = 0;

	spi_flash_cmd(flash->spi, CMD_W25_RDSR1, &status, 1);

	if ((status & (REG_W25_SRP0 | REG_W25_BP2 | REG_W25_BP1 | REG_W25_BP0))
		return 1;

	return 0;
}

static int winbond_sec_read(struct spi_flash *flash, u32 offset, size_t len, void *buf)
{
	int ret = 1;
	u8 cmd[5];
	u8 reg = (offset >> 8) & 0xFF;
	u8 addr = offset & 0xFF;

	if (reg != ADDR_W25_SEC1 && reg != ADDR_W25_SEC2 && reg != ADDR_W25_SEC3) {
		spi_debug("SF: Wrong security register\n");
		return 1;
	}

	cmd[0] = CMD_W25_RD_SEC;
	cmd[1] = 0x0;
	cmd[2] = reg;
	cmd[3] = addr;
	cmd[4] = 0x0; // dummy byte needed for this instruction

	flash->spi->rw = SPI_READ_FLAG;
	ret = spi_claim_bus(flash->spi);
	if (ret) {
		spi_debug("SF: Unable to claim SPI bus\n");
		return ret;
	}

	ret = spi_flash_cmd_read(flash->spi, cmd, sizeof(cmd), buf, len);
	if (ret) {
		spi_debug("SF: Can't read sec register %d\n", reg >> 4);
		goto out;
	}

out:
	spi_release_bus(flash->spi);
	return ret;
}

static int winbond_sec_program(struct spi_flash *flash, u32 offset, size_t len, const void *buf)
{
	int ret = 1;
	u8 cmd[4];
	u8 reg = (offset >> 8) & 0xFF;
	u8 addr = offset & 0xFF;
	u32 tmp_sect_size = flash->sector_size;

	if (reg != ADDR_W25_SEC1 && reg != ADDR_W25_SEC2 && reg != ADDR_W25_SEC3) {
		spi_debug("SF: Wrong security register\n");
		return 1;
	}

	flash->sector_size = 1;
	ret = spi_flash_cmd_erase(flash, CMD_W25_ER_SEC, offset & (0xFF << 8), 1);
	flash->sector_size = tmp_sect_size;
	if (ret) {
		spi_debug("SF: Can't erase sec reg\n");
		return ret;
	}

	flash->spi->rw = SPI_WRITE_FLAG;
	ret = spi_claim_bus(flash->spi);
	if (ret) {
		spi_debug("SF: Unable to claim SPI bus\n");
		return ret;
	}

	ret = spi_flash_cmd(flash->spi, CMD_W25_WREN, NULL, 0);
	if (ret) {
		spi_debug("SF: Enabling Write failed\n");
		goto out;
	}

	cmd[0] = CMD_W25_WR_SEC;
	cmd[1] = 0x0;
	cmd[2] = reg;
	cmd[3] = addr;
	ret = spi_flash_cmd_write(flash->spi, cmd, sizeof(cmd), buf, len);
	if (ret) {
		spi_debug("SF: Can't write to sec register %d\n", reg >> 4);
		goto out;
	}

	ret = spi_flash_cmd_wait_ready(flash, SPI_FLASH_PROG_TIMEOUT);
	if (ret) {
		spi_debug("SF: Programming sec register failed - timeout\n");
		goto out;
	}

out:
	spi_release_bus(flash->spi);
	return ret;
}

static int winbond_sec_erase(struct spi_flash *flash, u32 offset, size_t len)
{
	int ret = 1;
	u8 reg = (offset >> 8) & 0xFF;
	u32 tmp_sect_size = flash->sector_size;

	if (reg != ADDR_W25_SEC1 && reg != ADDR_W25_SEC2 && reg != ADDR_W25_SEC3) {
		spi_debug("SF: Wrong security register\n");
		return 1;
	}

	flash->sector_size = 1;
	ret = spi_flash_cmd_erase(flash, CMD_W25_ER_SEC, offset & (0xFF << 8), 1);
	flash->sector_size = tmp_sect_size;

	if (ret)
		spi_debug("SF: Can't erase sec reg\n");

	return ret;
}

static int winbond_sec_sts(struct spi_flash *flash)
{
	u8 status = 0;
	int ret;

	ret = spi_flash_cmd(flash->spi, CMD_W25_RDSR2, &status, 1);
	if (ret) {
		return -1;
	}

	status = status & ((REG_W25_LB1 | REG_W25_LB2 | REG_W25_LB3) >> 3);

	return status;
}

static int winbond_sec_lock(struct spi_flash *flash, u8 reg)
{
	int ret;
	u8 cmd;
	u8 status[2];

	flash->spi->rw = SPI_WRITE_FLAG;
	ret = spi_claim_bus(flash->spi);
	if (ret) {
		spi_debug("SF: Unable to claim SPI bus\n");
		return ret;
	}

	ret = spi_flash_cmd(flash->spi, CMD_W25_RDSR1, &status[0], 1);
	if (ret) {
		spi_debug("SF: Unable to read Status Register 1\n");
		goto out;
	}

	ret = spi_flash_cmd(flash->spi, CMD_W25_RDSR2, &status[1], 1);
	if (ret) {
		spi_debug("SF: Unable to read Status Register 2\n");
		goto out;
	}

	switch (reg) {
	case 1:
		status[1] |= REG_W25_LB1;
		break;
	case 2:
		status[1] |= REG_W25_LB2;
		break;
	case 3:
		status[1] |= REG_W25_LB3;
		break;
	default:
		spi_debug("SF: can't lock sec register, wrong index given\n");
		goto out;
	}

	ret = spi_flash_cmd(flash->spi, CMD_W25_WREN, NULL, 0);
	if (ret) {
		spi_debug("SF: Enabling Write failed\n");
		goto out;
	}

	cmd = CMD_W25_WRSR;
	ret = spi_flash_cmd_write(flash->spi, &cmd, sizeof(cmd),
				  status, sizeof(status));
	if (ret) {
		spi_debug("SF: Status register write failed\n");
		goto out;
	}

out:
	spi_release_bus(flash->spi);
	return ret;
}

struct spi_flash *spi_flash_probe_winbond(struct spi_slave *spi, u8 *idcode)
{
	const struct winbond_spi_flash_params *params;
	unsigned page_size;
	struct winbond_spi_flash *stm;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(winbond_spi_flash_table); i++) {
		params = &winbond_spi_flash_table[i];
		if (params->id == ((idcode[1] << 8) | idcode[2]))
			break;
	}

	if (i == ARRAY_SIZE(winbond_spi_flash_table)) {
		spi_debug("SF: Unsupported Winbond ID %02x%02x\n",
				idcode[1], idcode[2]);
		return NULL;
	}

	stm = malloc(sizeof(struct winbond_spi_flash));
	if (!stm) {
		spi_debug("SF: Failed to allocate memory\n");
		return NULL;
	}

	stm->params = params;
	stm->flash.spi = spi;
	stm->flash.name = params->name;

	/* Assuming power-of-two page size initially. */
	page_size = 1 << params->l2_page_size;

	stm->flash.write = winbond_write;
	stm->flash.spi_erase = winbond_erase;
	stm->flash.lock = winbond_lock;
	stm->flash.unlock = winbond_unlock;
	stm->flash.is_locked = winbond_is_locked;
	stm->flash.sec_sts = winbond_sec_sts;
	stm->flash.sec_read = winbond_sec_read;
	stm->flash.sec_prog = winbond_sec_program;
	stm->flash.sec_erase = winbond_sec_erase;
	stm->flash.sec_lock = winbond_sec_lock;
#if CONFIG_SPI_FLASH_NO_FAST_READ
	stm->flash.read = spi_flash_cmd_read_slow;
#else
	stm->flash.read = spi_flash_cmd_read_fast;
#endif
	stm->flash.sector_size = (1 << stm->params->l2_page_size) *
		stm->params->pages_per_sector;
	stm->flash.size = page_size * params->pages_per_sector
				* params->sectors_per_block
				* params->nr_blocks;

	return &stm->flash;
}
