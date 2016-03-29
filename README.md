Sortbootorder
-------------

This repository contain source code of `sortbootorder` payload that sorts and
save boot order in flash.

Below instructions assume Docker-based build environment described [here](https://github.com/pcengines/apu2-documentation/blob/master/docs/building_env.md).

## Building

Inside container:

```
git clone git@github.com:pcengines/sortbootorder.git payloads/pcengines/sortbootorder
cd payloads/pcengines/sortbootorder
make
```

## Adding sortbootorder to coreboot.rom

```
cd /
./coreboot/build/cbfstool ./coreboot/build/coreboot.rom add-payload -f payloads/pcengines/sortbootorder/sortbootorder.elf -n img/setup -t payload
```

## Logs

Correct compilation log should look like that:

```
CC="/coreboot/payloads/libpayload/../../util/crossgcc/xgcc/bin/i386-elf-gcc  -Wno-unused-but-set-variable  -fuse-ld=bfd -Wa,--divide -fno-stack-protector -Wl,--build-id=none" ../../libpayload/install/libpayload/bin/lpgcc -Wall -Werror -Os -c -o eon.o eon.c
CC="/coreboot/payloads/libpayload/../../util/crossgcc/xgcc/bin/i386-elf-gcc  -Wno-unused-but-set-variable  -fuse-ld=bfd -Wa,--divide -fno-stack-protector -Wl,--build-id=none" ../../libpayload/install/libpayload/bin/lpgcc -Wall -Werror -Os -c -o gigadevice.o gigadevice.c
CC="/coreboot/payloads/libpayload/../../util/crossgcc/xgcc/bin/i386-elf-gcc  -Wno-unused-but-set-variable  -fuse-ld=bfd -Wa,--divide -fno-stack-protector -Wl,--build-id=none" ../../libpayload/install/libpayload/bin/lpgcc -Wall -Werror -Os -c -o macronix.o macronix.c
CC="/coreboot/payloads/libpayload/../../util/crossgcc/xgcc/bin/i386-elf-gcc  -Wno-unused-but-set-variable  -fuse-ld=bfd -Wa,--divide -fno-stack-protector -Wl,--build-id=none" ../../libpayload/install/libpayload/bin/lpgcc -Wall -Werror -Os -c -o sortbootorder.o sortbootorder.c
CC="/coreboot/payloads/libpayload/../../util/crossgcc/xgcc/bin/i386-elf-gcc  -Wno-unused-but-set-variable  -fuse-ld=bfd -Wa,--divide -fno-stack-protector -Wl,--build-id=none" ../../libpayload/install/libpayload/bin/lpgcc -Wall -Werror -Os -c -o spansion.o spansion.c
CC="/coreboot/payloads/libpayload/../../util/crossgcc/xgcc/bin/i386-elf-gcc  -Wno-unused-but-set-variable  -fuse-ld=bfd -Wa,--divide -fno-stack-protector -Wl,--build-id=none" ../../libpayload/install/libpayload/bin/lpgcc -Wall -Werror -Os -c -o spi.o spi.c
CC="/coreboot/payloads/libpayload/../../util/crossgcc/xgcc/bin/i386-elf-gcc  -Wno-unused-but-set-variable  -fuse-ld=bfd -Wa,--divide -fno-stack-protector -Wl,--build-id=none" ../../libpayload/install/libpayload/bin/lpgcc -Wall -Werror -Os -c -o spi_flash.o spi_flash.c
CC="/coreboot/payloads/libpayload/../../util/crossgcc/xgcc/bin/i386-elf-gcc  -Wno-unused-but-set-variable  -fuse-ld=bfd -Wa,--divide -fno-stack-protector -Wl,--build-id=none" ../../libpayload/install/libpayload/bin/lpgcc -Wall -Werror -Os -c -o sst.o sst.c
CC="/coreboot/payloads/libpayload/../../util/crossgcc/xgcc/bin/i386-elf-gcc  -Wno-unused-but-set-variable  -fuse-ld=bfd -Wa,--divide -fno-stack-protector -Wl,--build-id=none" ../../libpayload/install/libpayload/bin/lpgcc -Wall -Werror -Os -c -o stmicro.o stmicro.c
CC="/coreboot/payloads/libpayload/../../util/crossgcc/xgcc/bin/i386-elf-gcc  -Wno-unused-but-set-variable  -fuse-ld=bfd -Wa,--divide -fno-stack-protector -Wl,--build-id=none" ../../libpayload/install/libpayload/bin/lpgcc -Wall -Werror -Os -c -o winbond.o winbond.c
CC="/coreboot/payloads/libpayload/../../util/crossgcc/xgcc/bin/i386-elf-gcc  -Wno-unused-but-set-variable  -fuse-ld=bfd -Wa,--divide -fno-stack-protector -Wl,--build-id=none" ../../libpayload/install/libpayload/bin/lpgcc -o sortbootorder.elf eon.o gigadevice.o macronix.o sortbootorder.o spansion.o spi.o spi_flash.o sst.o stmicro.o winbond.o
```

## Known issues

* If command inside container return something similar to this message:

    ```
    CC="/apu2b-20160304/payloads/libpayload/../../util/crossgcc/xgcc/bin/i386-elf-gcc  -Wno-unused-but-set-variable  -fuse-ld=bfd -Wa,--divide -fno-stack-protector -Wl,--build-id=none" ../../libpayload/install/libpayload/bin/lpgcc -Wall -Werror -Os -c -o eon.o eon.c
    ../../libpayload/install/libpayload/bin/lpgcc: 98: ../../libpayload/install/libpayload/bin/lpgcc: /apu2b-20160304/payloads/libpayload/../../util/crossgcc/xgcc/bin/i386-elf-gcc: not found
    ../../libpayload/install/libpayload/bin/lpgcc: 1: ../../libpayload/install/libpayload/bin/lpgcc: /apu2b-20160304/payloads/libpayload/../../util/crossgcc/xgcc/bin/i386-elf-gcc: not found
    ../../libpayload/install/libpayload/bin/lpgcc: 165: ../../libpayload/install/libpayload/bin/lpgcc: /apu2b-20160304/payloads/libpayload/../../util/crossgcc/xgcc/bin/i386-elf-gcc: not found
    make: *** [eon.o] Error 127
    root@aad4794250e9:/coreboot/payloads/pcengines/sortbootorder# ls ../../libpayload/install/libpayload/bin/lpgcc
    ```

    to fix that problem remove `../../libpayload/.xcompile` and recompile `libpayload`:

    ```
    cd ../../libpayload/
    rm -rf .xcompile
    make
    ```

* When given file name exist you can get:

    ```
    E: 'img/setup' already in ROM image.
    ```

    to fix that remove existing `img/setup`:

    ```
    ./coreboot/build/cbfstool ./coreboot/build/coreboot.rom remove -n img/setup
    ```
