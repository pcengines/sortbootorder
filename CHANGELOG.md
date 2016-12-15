Change log for PC Engines sortbootorder
=======================================

## [Unreleased]
## [v4.0.2] - 2016-12-09
### Changed
- versioning scheme to compatible with coreboot releases

### Fixed
- libpayload compilation procedure for legacy firmware

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
