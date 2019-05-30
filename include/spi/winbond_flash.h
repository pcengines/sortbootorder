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

/* M25Pxx-specific commands */
#define CMD_W25_WREN       0x06	/* Write Enable */
#define CMD_W25_WRDI       0x04	/* Write Disable */
#define CMD_W25_RDSR1      0x05	/* Read 1st Status Register */
#define CMD_W25_RDSR2      0x35	/* Read 2nd Status Register */
#define CMD_W25_WRSR       0x01	/* Write Status Register */
#define CMD_W25_READ       0x03	/* Read Data Bytes */
#define CMD_W25_FAST_READ  0x0b	/* Read Data Bytes at Higher Speed */
#define CMD_W25_PP         0x02	/* Page Program */
#define CMD_W25_SE         0x20	/* Sector (4K) Erase */
#define CMD_W25_BE         0xd8	/* Block (64K) Erase */
#define CMD_W25_CE         0xc7	/* Chip Erase */
#define CMD_W25_DP         0xb9	/* Deep Power-down */
#define CMD_W25_RES        0xab	/* Release from DP, and Read Signature */
#define CMD_W25_ER_SEC     0x44	/* Erase security registers */
#define CMD_W25_WR_SEC     0x42	/* Write security registers */
#define CMD_W25_RD_SEC     0x48	/* Read security registers */

#define REG_W25_BP0        (1 << 2)
#define REG_W25_BP1        (1 << 3)
#define REG_W25_BP2        (1 << 4)
#define REG_W25_TB         (1 << 5)
#define REG_W25_SEC        (1 << 6)
#define REG_W25_SRP0       (1 << 7)
#define REG_W25_SRP1       (1 << 0)
#define REG_W25_CMP        (1 << 6)
#define REG_W25_WPS        (1 << 2)
#define REG_W25_LB1        (1 << 3)
#define REG_W25_LB2        (1 << 4)
#define REG_W25_LB3        (1 << 5)

#define ADDR_W25_SEC1      0x10
#define ADDR_W25_SEC2      0x20
#define ADDR_W25_SEC3      0x30
