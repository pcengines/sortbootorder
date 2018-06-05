/*
 * Interface to SPI flash
 *
 * Copyright (C) 2008 Atmel Corporation
 * Copyright (c) 2014 Sage Electronic Engineering, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
#ifndef _SPI_FLASH_H_
#define _SPI_FLASH_H_

#include <stdint.h>
#include <stddef.h>
#include <spi/spi.h>

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})

#define min(a, b) ((a)<(b)?(a):(b))
#define sec_addr(offset, address) ((((uint32_t)offset) << 12) | (address))

#define CONFIG_ICH_SPI
#ifdef CONFIG_ICH_SPI
#define CONTROLLER_PAGE_LIMIT	64
#else
/* any number larger than 4K would do, actually */
#define CONTROLLER_PAGE_LIMIT	((int)(~0U>>1))
#endif

struct spi_flash {
	struct spi_slave *spi;
	const char	*name;
	u32		size;
	u32		sector_size;
	int		(*read)(struct spi_flash *flash, u32 offset, size_t len, void *buf);
	int		(*write)(struct spi_flash *flash, u32 offset, size_t len,
			const void *buf);
	int		(*spi_erase)(struct spi_flash *flash, u32 offset, size_t len);
	int		(*lock)(struct spi_flash *flash);
	int		(*unlock)(struct spi_flash *flash);
	int		(*is_locked)(struct spi_flash *flash);
	int		(*sec_sts)(struct spi_flash *flash);
	int		(*sec_read)(struct spi_flash *flash, u32 offset, size_t len, void *buf);
	int		(*sec_prog)(struct spi_flash *flash, u32 offset, size_t len,
			const void *buf);
	int		(*sec_erase)(struct spi_flash *flash, u32 offset, size_t len);
	int		(*sec_lock)(struct spi_flash *flash, u8 reg);
};

struct spi_flash *spi_flash_probe(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int spi_mode);

static inline int spi_flash_read(struct spi_flash *flash, u32 offset, size_t len, void *buf)
{
	return flash->read(flash, offset, len, buf);
}

static inline int spi_flash_write(struct spi_flash *flash, u32 offset,
		size_t len, const void *buf)
{
	return flash->write(flash, offset, len, buf);
}

static inline int spi_flash_erase(struct spi_flash *flash, u32 offset,
		size_t len)
{
	return flash->spi_erase(flash, offset, len);
}

static inline int spi_flash_lock(struct spi_flash *flash)
{
	return flash->lock(flash);
}

static inline int spi_flash_unlock(struct spi_flash *flash)
{
	return flash->unlock(flash);
}

static inline int spi_flash_is_locked(struct spi_flash *flash)
{
	return flash->is_locked(flash);
}

static inline int spi_flash_sec_sts(struct spi_flash *flash)
{
	return flash->sec_sts(flash);
}

static inline int spi_flash_sec_read(struct spi_flash *flash, u32 offset, size_t len,
		void *buf)
{
	return flash->sec_read(flash, offset, len, buf);
}

static inline int spi_flash_sec_prog(struct spi_flash *flash, u32 offset, size_t len,
		const void *buf)
{
	return flash->sec_prog(flash, offset, len, buf);
}

static inline int spi_flash_sec_erase(struct spi_flash *flash, u32 offset, size_t len)
{
	return flash->sec_erase(flash, offset, len);
}

static inline int spi_flash_sec_lock(struct spi_flash *flash, u8 reg)
{
	return flash->sec_lock(flash, reg);
}

#endif /* _SPI_FLASH_H_ */
