Sortbootorder
-------------

This repository contain source code of `sortbootorder` payload that sorts and
save boot order in flash.

Below instructions assume you use APU2 image builder according to instructions
provided [here](https://github.com/pcengines/apu2-documentation).

## Building

```
cd coreboot-${REL}
git clone https://github.com/pcengines/sortbootorder.git -b v${REL} payloads/pcengines/sortbootorder
cd payloads/libpayload
make defconfig
cp /xgcc/.xcompile-libpayload .xcompile
make
make install
cd ../pcengines/sortbootorder
make
```

For adding `sortbootorder` to `coreboot.rom` image please follow
[README.md](https://github.com/pcengines/apu2-documentation)
