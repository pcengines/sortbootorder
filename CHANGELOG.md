Change log for PC Engines sortbootorder
=======================================

Releases 4.0.x are based on PC Engines 20160304 release.

Releases 4.5.x and 4.6.x are based on mainline support submitted in
[this gerrit ref](https://review.coreboot.org/#/c/14138/).

## [Unreleased]
## [v4.6.24] - 2022-06-21
### Added
- Hide non-working iPXE option on apu7

## [v4.6.23] - 2022-05-19
### Added
- RTC clock menu for setting the clock after a CMOS reset

## [v4.6.22] - 2021-09-06
### Added
- guard against unsafe values for the watchdog timeout

## [v4.6.21] - 2021-05-27
### Added
- support for bootorder in FMAP region for persistent settings

## Changed
- The description of UART C/D toggles now shows the current state: GPIO or UART

## [v4.6.20] - 2020-08-27
### Fixed
- minor build fix for mainline coreboot

## [v4.6.19] - 2020-07-29
### Added
- new option in menu to allow reverse PCI addressing order

### Fixed
- SPI unlock when updating runtime configuration by adding prints to
 delay SPI status register bits change

## [v4.6.18] - 2020-04-26
### Added
- PCI Express power management features runtime configuration

## [v4.6.17] - 2020-01-28
### Changed
- disabled PS2 keyboard and mouse by default, not supported on apu platforms
  caused an error with keyboard initialization

## [v4.6.16] - 2019-12-30
### Added
- IOMMU runtime configuration

### Fixed
- top banner displays the correct board name

## [v4.6.15] - 2019-07-05
### Fixed
- incorrect IF conditions in SPI lock menu

## [v4.6.14] - 2019-06-04
### Added
- SD 3.0 mode runtime configuration
- watchdog runtime configuration with configurable timeout
- additional SPI flash lockdown hidden menu
- basic support for Adesto chips

### Changed
- all USB bootorder options have been unified and now affect all USB ports and
  headers

## [v4.6.13] - 2019-02-26
### Added
- CPU boost runtime configuration

## [v4.6.12] - 2018-12-03
### Fixed
- BIOS WP feature for different SPI parts

### Changed
- BIOS write protection is applied after saving other changes

### Added
- COM2 redirection runtime configuration

## [v4.6.11] - 2018-09-28
### Fixed
- printing serial number when security registers are erased

## [v4.6.10] - 2018-09-24
### Changed
- rewrite CBFS access using existing libpayload API

## [v4.6.9] - 2018-06-08
### Added
- erase security registers content option in hidden menu

## [v4.6.8] - 2018-04-06
### Added
- Support for APU1 target

### Fixed
- Error message when bootorder{,_map,_def} not found

## [v4.6.5] - 2017-12-29
### Added
- Support for restoring EHCI, PXE, UARTc/d, USB boot, serial console, mPCIe2 CLK
default values

## [v4.6.4] - 2017-11-30
### Added
- hidden menu with security registers access for writing the custom serial
  number
- `Legacy serial console redirection` option removed
- automated versioning

### Changed
- some code refactoring

## [v4.6.3] - 2017-10-30
### Added
- Enable runtime configuration of UARTc/d, EHCI and mPCIe2 CLK in mainline

## [v4.5.7] - 2017-07-21
### Added
- Add option to force CLK for mPCIe2 slot (GPP3 PCIe CLK)

## [v4.5.6] - 2017-06-29
### Added
- Add option to enable/disable BIOS WP jumper operation

### Changed
- Use the same version for legacy and mainline builds

## [v4.5.5] - 2017-05-30
### Added
- Add option in setup for setting `mPCIe1 SATA` priority (ASM106X cards)

### Changed
- Letters reserved for device sorting: from `a-m` to `a-j`
- Increase number of significant characters written to `bootorder` file from
  256 to 512 (`flash_write` function)

### Fixed
- Change `ehci enable` letter from `e` to `h` (conflict with `ipxe` priority)

## [v4.0.6] - 2017-05-30
### Added
- Add option in setup for setting `mPCIe1 SATA` priority (ASM106X cards)

### Changed
- Letters reserved for device sorting: from `a-m` to `a-j`

### Fixed
- Change `ehci enable` letter from `e` to `h` (conflict with `ipxe` priority)

## [v4.0.5.1] - 2017-03-31
### Changed
- changed the name to `PC Engines apu setup` in welcome string
- changed `Serial console redirection` to `Legacy console redirection`

## [v4.0.5] - 2017-03-30
### Added
- EHCI0 controller disable/enable option

## [v4.0.4] - 2017-02-28
### Added
- serial console redirection option (`SgaBIOS` enable) - by default enabled

### Fixed
- fixed writing bootorder files with sizes bigger than 255 bytes

## [v4.5.4] - 2017-02-23
### Changed
- adds new option tag when there isn't one in bootorder file
- removed Serial console option - not used in mainline release
- added Serial console redirection option - controls loading the sgabios oprom

## [v4.5.3] - 2017-01-12
### Fixed
- bootorder file alignment

## [v4.0.3] - 2017-01-03
### Added
- UART C and D toggling

## [v4.0.2] - 2016-12-09
### Changed
- versioning scheme to compatible with coreboot releases

### Fixed
- libpayload compilation procedure for legacy firmware

## [v4.5.2] - 2016-11-22

### Added
- support for mainline compilation

## [v4.0.1] - 2016-08-14
initial commit based on [coreboot_140908](http://pcengines.ch/tmp/coreboot_140908.tar.gz)

### Added
- `(disabled)` tag in menu
- `PXE` and `USB` enable options in menu
- `PXE` to bootorder menu
- support for `yangtzee fch spi controller`
- `PC Engines` header
- `README` and `Makefile`

### Changed
- merged all `USB` entries into one
- interface modification
- version bump to v1.2
- version bump to v1.3
- interface to lower case
- letter for save and exit to `S`
- user interface improvements

### Fixed
- used proper way to access extended SPI registers

[Unreleased]: https://github.com/pcengines/sortbootorder/compare/v4.6.24...master
[v4.6.24]: https://github.com/pcengines/sortbootorder/compare/v4.6.23...v4.6.24
[v4.6.23]: https://github.com/pcengines/sortbootorder/compare/v4.6.22...v4.6.23
[v4.6.22]: https://github.com/pcengines/sortbootorder/compare/v4.6.21...v4.6.22
[v4.6.21]: https://github.com/pcengines/sortbootorder/compare/v4.6.20...v4.6.21
[v4.6.20]: https://github.com/pcengines/sortbootorder/compare/v4.6.19...v4.6.20
[v4.6.19]: https://github.com/pcengines/sortbootorder/compare/v4.6.18...v4.6.19
[v4.6.18]: https://github.com/pcengines/sortbootorder/compare/v4.6.17...v4.6.18
[v4.6.17]: https://github.com/pcengines/sortbootorder/compare/v4.6.16...v4.6.17
[v4.6.16]: https://github.com/pcengines/sortbootorder/compare/v4.6.15...v4.6.16
[v4.6.15]: https://github.com/pcengines/sortbootorder/compare/v4.6.14...v4.6.15
[v4.6.14]: https://github.com/pcengines/sortbootorder/compare/v4.6.13...v4.6.14
[v4.6.13]: https://github.com/pcengines/sortbootorder/compare/v4.6.12...v4.6.13
[v4.6.12]: https://github.com/pcengines/sortbootorder/compare/v4.6.11...v4.6.12
[v4.6.11]: https://github.com/pcengines/sortbootorder/compare/v4.6.10...v4.6.11
[v4.6.10]: https://github.com/pcengines/sortbootorder/compare/v4.6.9...v4.6.10
[v4.6.9]: https://github.com/pcengines/sortbootorder/compare/v4.6.8...v4.6.9
[v4.6.8]: https://github.com/pcengines/sortbootorder/compare/v4.6.5...v4.6.8
[v4.6.5]: https://github.com/pcengines/sortbootorder/compare/v4.6.4...v4.6.5
[v4.6.4]: https://github.com/pcengines/sortbootorder/compare/v4.6.3...v4.6.4
[v4.6.3]: https://github.com/pcengines/sortbootorder/compare/v4.5.7...v4.6.3
[v4.5.7]: https://github.com/pcengines/sortbootorder/compare/v4.5.6...v4.5.7
[v4.5.6]: https://github.com/pcengines/sortbootorder/compare/v4.5.5...v4.5.6
[v4.5.5]: https://github.com/pcengines/sortbootorder/compare/v4.5.4...v4.5.5
[v4.5.4]: https://github.com/pcengines/sortbootorder/compare/v4.5.3...v4.5.4
[v4.5.3]: https://github.com/pcengines/sortbootorder/compare/v4.5.2...v4.5.3
[v4.0.6]: https://github.com/pcengines/sortbootorder/compare/v4.0.5.1...v4.0.6
[v4.0.5.1]: https://github.com/pcengines/sortbootorder/compare/v4.0.5...v4.0.5.1
[v4.0.5]: https://github.com/pcengines/sortbootorder/compare/v4.0.4...v4.0.5
[v4.0.4]: https://github.com/pcengines/sortbootorder/compare/v4.0.3...v4.0.4
[v4.0.3]: https://github.com/pcengines/sortbootorder/compare/v4.0.2...v4.0.3
[v4.0.2]: https://github.com/pcengines/sortbootorder/compare/v4.0.1...v4.0.2
[v4.5.2]: https://github.com/pcengines/sortbootorder/compare/v4.0.1...v4.5.2
[v4.0.1]: https://github.com/pcengines/sortbootorder/compare/f652a858ff905f17688d841f866c2dedb371fb24...v4.0.1
