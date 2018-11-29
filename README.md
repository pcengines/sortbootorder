Sortbootorder
-------------

This repository contains source code of `sortbootorder` payload that sorts and
saves boot order in flash.

## Contents

<!-- TOC -->

- [Contents](#contents)
- [Theory of operation](#theory-of-operation)
  - [Example menu view](#example-menu-view)
  - [Settings description](#settings-description)
  - [bootorder file](#bootorder-file)
  - [bootorder_map file](#bootorder_map-file)
  - [Default settings](#default-settings)
  - [BIOS WP option](#bios-wp-option)
  - [Hidden security registers menu](#hidden-security-registers-menu)
    - [Example](#example)
- [Building](#building)
  - [Manual build](#manual-build)
  - [Adding sortbootorder to coreboot.rom file](#adding-sortbootorder-to-corebootrom-file)
  - [Recent automated building process](#recent-automated-building-process)

<!-- /TOC -->

## Theory of operation

### Example menu view

> Exact list may be different, depending on BIOS release version.

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
  t Serial console - Currently Enabled
  k Redirect console output to COM2 - Currently Disabled
  o UART C - Currently Enabled
  p UART D - Currently Enabled
  m Force mPCIe2 slot CLK (GPP3 PCIe) - Currently Disabled
  h EHCI0 controller - Currently Disabled
  w Enable BIOS write protect - Currently Disabled
  x Exit setup without save
  s Save configuration and exit
```

First part of the list is used to set boot device priorities. Second part of
the list is used to enable/disable specific settings. Those information are
stored in `bootorder` file, which is written back to flash after hitting `s`
key.

### Settings description

* `r Restore boot order defaults` - restores boot order to default settings
* `n Network/PXE boot` - enables/disables the network boot (iPXE)
* `u USB boot` - enables/disables boot from USB drives
* `k Redirect console output to COM2` - enables/disables serial redirection to
  the COM2 port. Leaves COM1 entirely unused (except Memtest86+ still printing
  on COM1) for user needs.
* `t Serial console` - enables/disables output to the serial console
  Useful for legacy software, which is not using native serial port output, but
  uses standard PC text console instead (eg. FreeDOS).
* `o UART C` - enables/disables UART C on GPIO header. Disabled UART means
  enabled GPIO.
* `p UART D` - enables/disables UART D on GPIO header. Disabled UART means
     enabled GPIO.
* `m Force mPCIe2 slot CLK (GPP3 PCIe)` - enabling this option forces GPP3 PCIe
  clock (which is attached to mPCIe2 slot) to be always on. This helps in some
  cases, one example could be
  [mPCIe Ethernet extensioncard](https://github.com/pcengines/apu2-documentation/blob/master/docs/debug/mpcie_ethernet.md).
  It is advised to set to `Disable` if no extension card is attached to mPCIe2
  slot.
* `h EHCI0 controller` - enables/disables EHCI0 controller (used in apu3)
* `w Enable BIOS write protect` - enables/disables BIOS WP functionality. For
  details, see descritption in [BIOS WP option](#bios-wp-option).
* `x Exit setup without save` - exits setup menu without saving the settings
* `s Save configuration and exit` - exits setup menu saving the settings

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

When device is attached and detected by `SeaBIOS`, then `SeaBIOS` begins to
check if such device node is written into `bootorder` file. If it is, it gains
priority according to it's place on the list.
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

### Hidden security registers menu

Experimental menu containing options to write and read serial number to
security registers of the SPI flash chip. To enter press `Z` (`z + shift`)
in the main menu. Option description:

* `r` - reads the stored serial number
* `w {serial}` - writes the serial to register. Saves only first 10 characters.
* `s` - gets lock status of the security registers
* `l {register number}` - try to lock the specified register (1,2 or 3). Serial
  is stored in the register 1
* `q` - return to main menu

#### Example

```
> w 1234567890
serial written
> r
serial: 1234567890
> q
```

## Building

### Manual build

> coreboot is in `../coreboot-${BR_NAME}` directory

```sh
git clone https://github.com/pcengines/sortbootorder.git sortbootorder
cd sortbootorder
# for mainline coreboot (4.5.x, 4.6.x)
KDIR=../coreboot-${BR_NAME} make distclean
KDIR=../coreboot-${BR_NAME} make
# for legacy coreboot (4.0.x)
KDIR=../coreboot-${BR_NAME} make distclean
KDIR=../coreboot-${BR_NAME} COREBOOT_REL=legacy make
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
