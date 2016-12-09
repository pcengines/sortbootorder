Sortbootorder
-------------

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
