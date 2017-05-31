## Sortbootorder

This repository contains source code of `sortbootorder` payload that sorts and
saves boot order in flash.

## Theory of operation

### Example menu view

Exact list may be different, depending on BIOS release version.

  ```
  a USB 1 / USB 2 SS and HS
  b SDCARD
  c mSATA
  d SATA
  e mPCIe1 SATA1 and SATA2
  f iPXE (disabled)

  r Restore boot order defaults
  n Network/PXE boot - Currently Disabled
  t Serial console - Currently Enabled
  l Legacy console redirection - Currently Enabled
  u USB boot - Currently Enabled
  h EHCI0 controller - Currently Disabled
  o UART C - Currently Enabled
  p UART D - Currently Enabled
  x Exit setup without save
  s Save configuration and exit
  ```

First part of the list is used to set boot device priorities.  Second part of
the list is used to enable/disable specific settings. Those information are
stored in `bootorder` file, which is written back to flash after hitting `s`
key.

### bootorder file

Sortbootorder manages `bootorder` file which is initially shipped from
[coreboot repository](https://github.com/pcengines/coreboot/blob/coreboot-4.0.x/src/mainboard/pcengines/apu2/bootorder)
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

[bootorder_map](https://github.com/pcengines/coreboot/blob/coreboot-4.0.x/src/mainboard/pcengines/apu2/bootorder_map)
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
  Serial console - Enabled
  Legacy console redirection - Enabled
  USB boot - Enabled
  EHCI0 controller - Disabled
  UART C - Enabled
  UART D - Enabled
  ```

Default bootorder list settings are taken from
[bootorder_def file](https://github.com/pcengines/coreboot/blob/coreboot-4.0.x/src/mainboard/pcengines/apu2/bootorder_def).
They can be restored by hitting `r` key. It only restores to default boot
list order, not other specific settings such as `USB enable` or `serial console
enable`.

## Building

### Recent automated building process

Please follow
[release process](https://github.com/pcengines/apu2-documentation/blob/master/docs/release_process.md)
document to build complete `coreboot` binary, already including `SeaBIOS` and
other payloads (such us `sortbootorder`).
