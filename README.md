# Apache NuttX RTOS on PinePhone

[Apache NuttX RTOS](https://nuttx.apache.org/docs/latest/) now runs on Arm Cortex-A53 with Multi-Core SMP...

https://github.com/apache/incubator-nuttx/tree/master/boards/arm64/qemu/qemu-a53

Will NuttX run on PinePhone? PinePhone is based on Allwinner A64 SoC with 4 Cores of Arm Cortex-A53...

https://wiki.pine64.org/index.php/PinePhone

NuttX might be a fun way to teach more people about Phone Operating Systems. And someday we might have a cheap, fast and responsive phone running on NuttX!

Many thanks to [qinwei2004](https://github.com/qinwei2004) and the NuttX Team for implementing [Cortex-A53 support](https://github.com/apache/incubator-nuttx/pull/6478)!

# Download NuttX

TODO

```bash
mkdir nuttx
cd nuttx
git clone --recursive --branch arm64 \
    https://github.com/lupyuen/incubator-nuttx \
    nuttx
git clone --recursive --branch arm64 \
    https://github.com/lupyuen/incubator-nuttx-apps \
    apps
cd nuttx
```

Install prerequisites, skip the RISC-V Toolchain...

https://lupyuen.github.io/articles/nuttx#install-prerequisites

# Download Toolchain

TODO

Instructions:

https://github.com/apache/incubator-nuttx/tree/master/boards/arm64/qemu/qemu-a53

Download toolchain for AArch64 ELF bare-metal target (aarch64-none-elf)

https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads

For macOS:

https://developer.arm.com/-/media/Files/downloads/gnu/11.3.rel1/binrel/arm-gnu-toolchain-11.3.rel1-darwin-x86_64-aarch64-none-elf.pkg

For x64 Linux:

https://developer.arm.com/-/media/Files/downloads/gnu/11.2-2022.02/binrel/gcc-arm-11.2-2022.02-x86_64-aarch64-none-elf.tar.xz

Add to PATH: /Applications/ArmGNUToolchain/11.3.rel1/aarch64-none-elf/bin

```bash
export PATH="$PATH:/Applications/ArmGNUToolchain/11.3.rel1/aarch64-none-elf/bin"
```

# Download QEMU

TODO

Download QEMU: https://www.qemu.org/download/

```bash
brew install qemu
```

# Build NuttX: Single Core

TODO

Configure NuttX and compile...

```bash
./tools/configure.sh -l qemu-a53:nsh
make
```

Test with qemu...

```bash
qemu-system-aarch64 -cpu cortex-a53 -nographic \
    -machine virt,virtualization=on,gic-version=3 \
    -net none -chardev stdio,id=con,mux=on -serial chardev:con \
    -mon chardev=con,mode=readline -kernel ./nuttx
```

# Build NuttX: Multi Core

TODO

Configure NuttX and compile...

```bash
./tools/configure.sh -l qemu-a53:nsh_smp
make
```

Test with qemu...

```bash
qemu-system-aarch64 -cpu cortex-a53 -smp 4 -nographic \
    -machine virt,virtualization=on,gic-version=3 \
    -net none -chardev stdio,id=con,mux=on -serial chardev:con \
    -mon chardev=con,mode=readline -kernel ./nuttx
```

# Analyse Image with Ghidra

TODO: Disassemble a PinePhone Image with Ghidra to look at the Startup Code

https://github.com/dreemurrs-embedded/Jumpdrive

Download https://github.com/dreemurrs-embedded/Jumpdrive/releases/download/0.8/pine64-pinephone.img.xz

Expand `pine64-pinephone.img.xz`

Expand the files inside...

```bash
gunzip Image.gz
gunzip initramfs.gz
tar xvf initramfs
```

Import `Image` as AARCH64:LE:v8A:default...
-   Processor: AARCH64 
-   Variant: v8A 
-   Size: 64 
-   Endian: little 
-   Compiler: default

`Image` seems to start at 0x40000000, as suggested by this Memory Map...

https://linux-sunxi.org/A64/Memory_map

Click Window > Memory Map

Click "ram"

Click the 4-Arrows icon (Move a block to another address)

Change Start Address to 40000000

# TODO

TODO: Verify that NuttX uses similar Startup Code

TODO: Build UART Driver in NuttX for Allwinner A64 SoC

UART0 Memory Map: https://linux-sunxi.org/A64/Memory_map

More about A64 UART: https://linux-sunxi.org/UART

TODO: Configure NuttX Memory Regions for Allwinner A64 SoC

TODO: Copy NuttX to microSD Card

A64 Boot ROM: https://linux-sunxi.org/BROM#A64

A64 U-Boot SPL: https://linux-sunxi.org/BROM#U-Boot_SPL_limitations

SD Card Layout: https://linux-sunxi.org/Bootable_SD_card#SD_Card_Layout

TODO: Boot NuttX on PinePhone and test NuttX Shell
