/*
 * Copyright (c) 2014 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Ported from mosys project (http://code.google.com/p/mosys/).
 */

#ifndef __LIB_VPD_TABLES_H__
#define __LIB_VPD_TABLES_H__

#include <stdint.h>

#define VPD_ENTRY_MAGIC    "_SM_"
#define VPD_INFO_MAGIC     \
  "\xfe"      /* type: VPD header */       \
  "\x09"      /* key length, 9 = 1 + 8 */  \
  "\x01"      /* info version, 1 */        \
  "gVpdInfo"  /* signature, 8 bytes */ \
  "\x04"      /* value length */

/* Google specific VPD info */
struct google_vpd_info {
  union {
    struct {
      u8 type;
      u8 key_len;
      u8 info_ver;
      u8 signature[8];
      u8 value_len;
    } tlv;
    u8 magic[12];
  } header;
  u32 size;
} __attribute__((packed));

/* Entry */
struct vpd_entry {
  u8 anchor_string[4];
  u8 entry_cksum;
  u8 entry_length;
  u8 major_ver;
  u8 minor_ver;
  u16 max_size;
  u8 entry_rev;
  u8 format_area[5];
  u8 inter_anchor_string[5];
  u8 inter_anchor_cksum;
  u16 table_length;
  u32 table_address;
  u16 table_entry_count;
  u8 bcd_revision;
} __attribute__ ((packed));

/* Header */
struct vpd_header {
  u8 type;
  u8 length;
  u16 handle;
} __attribute__ ((packed));

/* Type 0 - firmware information */
struct vpd_table_firmware {
  u8 vendor;
  u8 version;
  u16 start_address;
  u8 release_date;
  u8 rom_size_64k_blocks;
  u32 characteristics;
  u8 extension[2];  /* v2.4+ */
  u8 major_ver;     /* v2.4+ */
  u8 minor_ver;     /* v2.4+ */
  u8 ec_major_ver;  /* v2.4+ */
  u8 ec_minor_ver;  /* v2.4+ */
} __attribute__ ((packed));

/* Type 1 - system information */
struct vpd_table_system {
  u8 manufacturer;
  u8 name;
  u8 version;
  u8 serial_number;
  u8 uuid[16];
  u8 wakeup_type;
  u8 sku_number;  /* v2.4+ */
  u8 family;      /* v2.4+ */
} __attribute__ ((packed));

/* Type 127 - end of table */
struct vpd_table_eot {
  struct vpd_header header;
} __attribute__ ((packed));

/* Type 241 - binary blob pointer */
struct vpd_table_binary_blob_pointer {
  u8 struct_major_version;
  u8 struct_minor_version;
  u8 vendor;
  u8 description;
  u8 major_version;
  u8 minor_version;
  u8 variant;
  u8 reserved[5];
  u8 uuid[16];
  u32 offset;
  u32 size;
} __attribute__ ((packed));

/* The length and number of strings defined here is not a limitation of VPD.
 * These numbers were deemed good enough during development. */
#define VPD_MAX_STRINGS 10
#define VPD_MAX_STRING_LENGTH 64

#endif /* __LIB_VPD_TABLES_H__ */
