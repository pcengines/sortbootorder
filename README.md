Sortbootorder
-------------

This repository contains source code of `sortbootorder` payload that sorts and
saves boot order in flash.

## Contents

- [Sortbootorder](#sortbootorder)
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
  - [Hidden flash lockdown menu](#hidden-flash-lockdown-menu)
    - [Example](#example-1)
- [Building](#building)
  - [Manual build](#manual-build)
  - [Adding sortbootorder to coreboot.rom file](#adding-sortbootorder-to-corebootrom-file)
  - [Recent automated building process](#recent-automated-building-process)
  - [Issues](#issues)

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
  o UART C - Currently Enabled - Toggle UART C / GPIO
  p UART D - Currently Enabled - Toggle UART D / GPIO
  m Force mPCIe2 slot CLK (GPP3 PCIe) - Currently Disabled
  h EHCI0 controller - Currently Disabled
  l Core Performance Boost - Currently Enabled
  v IOMMU - Currently Disabled
  u PCIe power management features - Currently Disabled
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
* `o UART C` - enables/disables UART C on GPIO header. Disabled UART C means
  enabled GPIO[0..7].
* `p UART D` - enables/disables UART D on GPIO header. Disabled UART D means
     enabled GPIO[10..17].
* `m Force mPCIe2 slot CLK (GPP3 PCIe)` - enabling this option forces GPP3 PCIe
  clock (which is attached to mPCIe2 slot) to be always on. This helps in some
  cases, one example could be
  [mPCIe Ethernet extensioncard](https://github.com/pcengines/apu2-documentation/blob/master/docs/debug/mpcie_ethernet.md).
  It is advised to set to `Disable` if no extension card is attached to mPCIe2
  slot.
* `h EHCI0 controller` - enables/disables EHCI0 controller (used in apu3)
* `l Core Performance Boost` - enables/disables CPU boost.
* `i Watchdog` - enables/disables hardware watchdog. Each time toggled, the
  prompt will pop out asking to provide a timeout value in seconds (0 to
  disable). **Set reasonably high value so it will be possible to disable the**
  **watchdog later. Too low value may result in a reset loop!**
* `j SD 3.0 mode` - enable SD controller in 3.0 mode to allow achieving full
  speeds with UHS-I SD cards
* `g Reverse order of PCI addresses` - When enabled, the PCIe devices are
ordered as follows: NICs, mPCIe1, mPCIe2. In such way the PCI enumeration is
permanent for NICs regardless of mPCIe WiFi module presence or faulty detection.
More information in [documentation](https://github.com/pcengines/apu2-documentation/blob/master/docs/order_of_PCI_addresses.md)
* `v IOMMU` - enables/disables input–output memory management unit
* `u PCIe power management features` - enables/disables PCI Express power
  management features like: ASPM, Common Clock, Clock Power Management (if
  supported by PCI Express endpoints). **Enabling this option will reduce the**
  **power consumption at the cost of performance drop of Ethenet controllers**
  **and WiFi cards**
* `w Enable BIOS write protect` - enables/disables BIOS WP functionality. For
  details, see descritption in [BIOS WP option](#bios-wp-option).
* `x Exit setup without save` - exits setup menu without saving the settings
* `s Save configuration and exit` - exits setup menu saving the settings

### bootorder file

Sortbootorder manages `bootorder` file which is initially shipped from
[coreboot repository](https://github.com/pcengines/coreboot/blob/coreboot-4.5.x/src/mainboard/pcengines/apu2/bootorder)
This is binary file that has to be 4096 bytes long in order to entirely fill
one FLASH sector.

In pre-v4.14 coreboot this file was part of main CBFS. Since v4.14.0.1 it is
located in a dedicated FMAP region (`BOOTORDER`), which allows for firmware
upgrades without loss of settings. Refer to [flashing instructions](https://github.com/pcengines/apu2-documentation/blob/master/docs/firmware_flashing.md#corebootrom-flashing)
for proper command. Files `bootorder_def` and `bootorder_map` (see below) are
still located in CBFS.

Relevant content of this file may look like this:

```
/pci@i0cf8/usb@10/usb-*@1
/pci@i0cf8/usb@10/usb-*@2
/pci@i0cf8/usb@10/usb-*@3
/pci@i0cf8/usb@10/usb-*@4
/pci@i0cf8/usb@12/usb-*@1
/pci@i0cf8/usb@12/usb-*@2
/pci@i0cf8/usb@12/usb-*@3
/pci@i0cf8/usb@12/usb-*@4
/pci@i0cf8/usb@13/usb-*@1
/pci@i0cf8/usb@13/usb-*@2
/pci@i0cf8/usb@13/usb-*@3
/pci@i0cf8/usb@13/usb-*@4
/pci@i0cf8/*@14,7
/pci@i0cf8/*@11/drive@0/disk@0
/pci@i0cf8/*@11/drive@1/disk@0
/pci@i0cf8/pci-bridge@2,5/*@0/drive@0/disk@0
/pci@i0cf8/pci-bridge@2,5/*@0/drive@1/disk@0
/rom@genroms/pxe.rom
pxen0
scon1
usben1
...
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
  Serial console - Enabled
  USB boot - Enabled
  Force mPCIe2 slot CLK (GPP3 PCIe) - Disabled
  UART C - Enabled
  UART D - Enabled
  EHCI0 controller - Disabled
  Core Performance Boost - Enabled
  Watchdog - Disabled
  SD 3.0 mode - Disabled
  IOMMU - Disabled
  PCIe power management features - Disabled
  Redirect console output to COM2 - Disabled
  BIOS write protect - Disabled
  ```

Default bootorder list settings are taken from
[bootorder_def file](https://github.com/pcengines/coreboot/blob/develop/src/mainboard/pcengines/apu2/bootorder_def).
They can be restored by hitting `r` key. It only restores to default boot
list order, not other specific settings such as `USB enable` or `serial console
enable`.

### BIOS WP option

`Enable BIOS write protect` option (`w`) enables or disables flash write
protection feature. When enabled, then BIOS WP jumper (1-2 pins of J2) controls
the possibility of writing to flash. When BIOS WP is shorted and option is
enabled no writes to flash are possible, including disabling the write protect
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

### Hidden flash lockdown menu

Experimental menu containing options to write and read serial number to
security registers of the SPI flash chip. To enter press `Q` (`q + shift`)
in the main menu. Option description:

* `p` - prints all the protected ranges of SPI flash for CMP bit equal to 0,
        all protected ranges have its corresponding number that is used in
        other commands
* `r` - prints all the protected ranges of SPI flash for CMP bit equal to 1,
        all protected ranges have its corresponding number that is used in
        other commands
* `b {block_no}` - sets the desired protection range to enabled, takes the
                   protection range number as parameters; for correct number,
                   please refer to commands that print protected range statuses
* `c` - clears the block protection by setting the protected range to
        000000h - 000000h
* `s` - shows the current status register protection, each protection type has
        its corresponding number which is used in other commands;  due to the
        design limitations the WP pin state detection works only if SRP0 bit in
        status register is set
* `l {lock_type}` - set the desired status register protection, takes the
                    protection type number as a parameter; for the correct
                    number please refer to the command that prints the status
                    register protection status
* `q` - return to main menu

#### Example

```
> p
 1) Protected range 000000h – 000000h (currently enabled)
 2) Protected range 7E0000h – 7FFFFFh
 3) Protected range 7C0000h – 7FFFFFh
 4) Protected range 780000h – 7FFFFFh
 5) Protected range 700000h – 7FFFFFh
 6) Protected range 600000h – 7FFFFFh
 7) Protected range 400000h – 7FFFFFh
 8) Protected range  000000h – 01FFFFh
 9) Protected range  000000h – 03FFFFh
10) Protected range  000000h – 07FFFFh
11) Protected range  000000h – 0FFFFFh
12) Protected range  000000h – 1FFFFFh
13) Protected range  000000h – 3FFFFFh
14) Protected range  000000h – 7FFFFFh
15) Protected range  7FF000h – 7FFFFFh
16) Protected range  7FE000h – 7FFFFFh
17) Protected range  7FC000h – 7FFFFFh
18) Protected range  7F8000h – 7FFFFFh
19) Protected range  000000h – 000FFFh
20) Protected range  000000h – 001FFFh
21) Protected range  000000h – 003FFFh
22) Protected range  000000h – 007FFFh

...
> s
SRP0=0 , SRP1=0, WP=?
1) Status register is in Software Protected mode. WP pin may be active.
2) Status register is NOT in Hardware Protected mode
3) Status register is NOT in Hardware Unprotected mode
4) Status register is NOT in Power Supply Lock-Down mode
5) Status register is NOT in One Time Program mode

...

> b 5
Setting block protection success!
> p
 1) Protected range 000000h – 000000h
 2) Protected range 7E0000h – 7FFFFFh
 3) Protected range 7C0000h – 7FFFFFh
 4) Protected range 780000h – 7FFFFFh
 5) Protected range 700000h – 7FFFFFh (currently enabled)
 6) Protected range 600000h – 7FFFFFh
 7) Protected range 400000h – 7FFFFFh
 8) Protected range  000000h – 01FFFFh
 9) Protected range  000000h – 03FFFFh
10) Protected range  000000h – 07FFFFh
11) Protected range  000000h – 0FFFFFh
12) Protected range  000000h – 1FFFFFh
13) Protected range  000000h – 3FFFFFh
14) Protected range  000000h – 7FFFFFh
15) Protected range  7FF000h – 7FFFFFh
16) Protected range  7FE000h – 7FFFFFh
17) Protected range  7FC000h – 7FFFFFh
18) Protected range  7F8000h – 7FFFFFh
19) Protected range  000000h – 000FFFh
20) Protected range  000000h – 001FFFh
21) Protected range  000000h – 003FFFh
22) Protected range  000000h – 007FFFh

...

> l 4
Setting status register protection success!

> s
SRP0=0 , SRP1=1, WP=?
1) Status register is NOT in Software Protected mode.
2) Status register is NOT in Hardware Protected mode
3) Status register is NOT in Hardware Unprotected mode
4) Status register is in Power Supply Lock-Down mode
5) Status register is NOT in One Time Program mode
```

> For more verbose details about the protection modes and protection mechanisms
> please refer to Winbond W25Q64FV datasheet. W25Q64FV is currently the only
> supported chip by the hidden flash lockdown menu.

Be aware of the One Time Program mode. it will permanently lock the status
register. If You have left some block/range protection in place when locking,
You will not be able to erase/program that part of SPI flash. That means
**Your BIOS won't be upgraded anymore unless You solder a new unlocked chip.**
Handle with care. But, if You choose to set the permanent lock, You will be additionally
prompted for confirmation:

```
> l 5
WARNING: You are going to permanently lock status register
Are You sure? (y/n) n
Aborting...
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

### Issues

If you have any trouble or find any bug, please report an issue in 
[this location](https://github.com/pcengines/apu2-documentation/issues). To 
create the issue, choose the option `New issue', from the list of available 
templates select the one, that fits best the nature of the issue (bug, 
feature, question or task) and fill it.