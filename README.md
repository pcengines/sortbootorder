Sortbootorder
=============

This repository contain source code of `sortbootorder` payload that sorts and
save boot order in flash.

Below instructions assume you use APU2 image builder according to instructions
provided [here](https://github.com/pcengines/apu2-documentation).

## Building

```
cd && cd coreboot-${BR_NAME}
git clone https://github.com/pcengines/sortbootorder.git -b ${BR_NAME} payloads/pcengines/sortbootorder
cd payloads/libpayload
make clean && make defconfig
wget https://raw.githubusercontent.com/pcengines/apu2-documentation/master/xcompile/.apu2-builder-xcompile-libpayload -O .xcompile
make -j$(nproc)
make install
cd ../pcengines/sortbootorder
make -j$(nproc)
```

For adding `sortbootorder` to `coreboot.rom` image please follow
[README.md](https://github.com/pcengines/apu2-documentation)

## Options description

* `r Restore boot order defaults` - restores boot order to default settings
* `n Network/PXE boot` - enables/disables the network boot (iPXE)
* `t Serial console` - enables/disables output to the serial console
* `l Legacy console redirection` - enables/disables serial redirection to the sgabios (serial text console emulation).
  Useful for legacy software, which is not using native serial port output, but uses standard PC text console instead (eg. FreeDOS).
* `u USB boot` - enables/disables boot from USB drives
* `e EHCI0 controller` - enables/disables EHCI0 controller (used in apu3)
* `o UART C` - enables/disables UART C on GPIO header. Disabled UART means enabled GPIO.
* `p UART D` - enables/disables UART D on GPIO header. Disabled UART means enabled GPIO.
* `x Exit setup without save` - exits setup menu without saving the settings
* `s Save configuration and exit` - exits setup menu saving the settings
