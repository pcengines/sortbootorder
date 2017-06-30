Sortbootorder
-------------

This repository contains source code of `sortbootorder` payload that sorts and
saves boot order in flash.

## Contents

<!-- TOC -->

- [Contents](#contents)
- [Theory of operation](#theory-of-operation)
    - [Example menu view](#example-menu-view)
    - [bootorder file](#bootorder-file)
    - [bootorder_map file](#bootorder_map-file)
    - [Default settings](#default-settings)
    - [BIOS WP option](#bios-wp-option)
- [Building](#building)
    - [Manual build](#manual-build)
    - [Adding sortbootorder to coreboot.rom file](#adding-sortbootorder-to-corebootrom-file)
    - [Recent automated building process](#recent-automated-building-process)

<!-- /TOC -->

## Theory of operation

### Example menu view

> Exact list may be different, depending on BIOS release version.

For coreboot mainline (4.5.x) version:

  ```
  a USB 1 / USB 2 SS and HS
  b SDCARD
  c mSATA
  d SATA
  e mPCIe1 SATA1 and SATA2
  f iPXE (disabled)

  r Restore boot order defaults
  n Network/PXE boot - Currently Disabled
  u USB boot - Currently Enabled
  l Legacy console redirection - Currently Disabled
  w Enable BIOS write protect - Currently Disabled
  x Exit setup without save
  s Save configuration and exit
  ```

For coreboot legacy (4.0.x) version:

  ```
  a USB 1 / USB 2 SS and HS
  b SDCARD
  c mSATA
  d SATA
  e mPCIe1 SATA1 and SATA2
  f iPXE (disabled)


  r Restore boot order defaults
  n Network/PXE boot - Currently Disabled
  u USB boot - Currently Enabled
  l Legacy console redirection - Currently Enabled
  t Serial console - Currently Enabled
  o UART C - Currently Enabled
  p UART D - Currently Enabled
  h EHCI0 controller - Currently Disabled
  w Enable BIOS write protect - Currently Disabled
  x Exit setup without save
  s Save configuration and exit
  ```

First part of the list is used to set boot device priorities.  Second part of
the list is used to enable/disable specific settings. Those information are
stored in `bootorder` file, which is written back to flash after hitting `s`
key.

### bootorder file

Sortbootorder manages `bootorder` file which is initially shipped from
[coreboot repository](https://github.com/pcengines/coreboot/blob/coreboot-4.5.x/src/mainboard/pcengines/apu2/bootorder)
This is binary file that has to be 4096 bytes long in order to entirely fill
one FLASH sector.

Relevant content of this file may look like this:

  ```
  /pci@i0cf8/usb@10/usb-*@1
  /pci@i0cf8/usb@10/usb-*@2
  /pci@i0cf8/usb@10/usb-*@3
  /pci@i0cf8/usb@10/usb-*@4
  /pci@i0cf8/*@14,7
  /pci@i0cf8/*@11/drive@0/disk@0
  /pci@i0cf8/*@11/drive@1/disk@0
  /pci@i0cf8/pci-bridge@2,5/*@0/drive@0/disk@0
  /pci@i0cf8/pci-bridge@2,5/*@0/drive@1/disk@0
  /rom@genroms/pxe.rom
  pxen0
  scon1
  usben1
  ```

Rest of this file is filled with characters to meet that 4096 bytes
requirement.

When device is attached and detected by `SeaBIOS`, then `SeaBIOS` begins to check
if such device node is written into `bootorder` file. If it is, it gains priority
according to it's place on the list.
You can refer to
[SeaBIOS](https://github.com/pcengines/seabios/blob/coreboot-4.0.x/docs/Runtime_config.md#configuring-boot-order)
documentation for more insight.

### bootorder_map file

[bootorder_map](https://github.com/pcengines/coreboot/blob/coreboot-4.5.x/src/mainboard/pcengines/apu2/bootorder_map)
file is used to match device letter and description with corresponding node from
`bootorder` file.

### Default settings

* Bootorder list default settings:

  ```
  a USB 1 / USB 2 SS and HS
  b SDCARD
  c mSATA
  d SATA
  e mPCIe1 SATA1 and SATA2
  f iPXE
  ```

* Rest of default settings:

  ```
  Network/PXE boot - Disabled
  Serial console redirection - Disabled
  USB boot - Enabled
  ```

Default bootorder list settings are taken from
[bootorder_def file](https://github.com/pcengines/coreboot/blob/coreboot-4.5.x/src/mainboard/pcengines/apu2/bootorder_def).
They can be restored by hitting `r` key. It only restores to default boot
list order, not other specific settings such as `USB enable` or `serial console
enable`.

### BIOS WP option

`Enable BIOS write protect` option (`w`) enables or disables flash write
protection feature. When enabled, then BIOS WP jumper (1-2 pins of J2) controls
the possibility of writing to flash. When BIOS WP is shorted and option is
enabled no writes to flash is possible, including disabling the write protect
option itself and updating the BIOS is also not possible (using e.g. `flashrom`
tool).

## Building

### Manual build

> coreboot is in `./coreboot-${BR_NAME}` directory

```sh
git clone https://github.com/pcengines/sortbootorder.git sortbootorder
cd sortbootorder
# for mainline coreboot (4.5.x)
COREBOOT_ROOT=../coreboot-${BR_NAME} make distclean
COREBOOT_ROOT=../coreboot-${BR_NAME} make
# for legacy coreboot (4.0.x)
COREBOOT_ROOT=../coreboot-${BR_NAME} make distclean
COREBOOT_ROOT=../coreboot-${BR_NAME} COREBOOT_REL=legacy make
```

### Adding sortbootorder to coreboot.rom file

```sh
cd && cd coreboot-${BR_NAME}
./build/cbfstool ./build/coreboot.rom remove -n img/setup
./build/cbfstool ./build/coreboot.rom add-payload -f payloads/pcengines/sortbootorder/sortbootorder.elf -n img/setup -t payload
```

Above commands first remove already existing `img/setup` from CBFS and then add
`sortbootorder.elf` as payload under the name `img/setup` to `coreboot.rom`.

### Recent automated building process

Please follow
[release process](https://github.com/pcengines/apu2-documentation/blob/master/docs/release_process.md)
document to build complete `coreboot` binary, already including `SeaBIOS` and
other payloads (such us `sortbootorder`).
