![Ghidra with Apache NuttX RTOS for Arm Cortex-A53](https://lupyuen.github.io/images/arm-title.png)

# Apache NuttX RTOS on PinePhone

Read the article...

-   ["Apache NuttX RTOS on Arm Cortex-A53: How it might run on PinePhone"](https://lupyuen.github.io/articles/arm)

[Apache NuttX RTOS](https://nuttx.apache.org/docs/latest/) now runs on Arm Cortex-A53 with Multi-Core SMP...

-   [nuttx/boards/arm64/qemu/qemu-a53](https://github.com/apache/incubator-nuttx/tree/master/boards/arm64/qemu/qemu-a53)

PinePhone is based on [Allwinner A64 SoC](https://linux-sunxi.org/A64) with 4 Cores of Arm Cortex-A53...

-   [PinePhone Wiki](https://wiki.pine64.org/index.php/PinePhone)

Will NuttX run on PinePhone? Let's find out!

_Why NuttX?_

NuttX is tiny and might be a fun way to teach more people about the internals of Phone Operating Systems. (Without digging deep into the entire Linux Stack)

Someday we might have a cheap, fast, responsive and tweakable phone running on NuttX!

Many thanks to [qinwei2004](https://github.com/qinwei2004) and the NuttX Team for implementing [Cortex-A53 support](https://github.com/apache/incubator-nuttx/pull/6478)!

# Download NuttX

Download the Source Code for NuttX Mainline, which supports Arm Cortex-A53...

```bash
## Create NuttX Directory
mkdir nuttx
cd nuttx

## Download NuttX OS
git clone \
    --recursive \
    --branch arm64 \
    https://github.com/lupyuen/incubator-nuttx \
    nuttx

## Download NuttX Apps
git clone \
    --recursive \
    --branch arm64 \
    https://github.com/lupyuen/incubator-nuttx-apps \
    apps

## We'll build NuttX inside nuttx/nuttx
cd nuttx
```

Install the Build Prerequisites, skip the RISC-V Toolchain...

-   ["Install Prerequisites"](https://lupyuen.github.io/articles/nuttx#install-prerequisites)

# Download Toolchain

Download the Arm Toolchain for AArch64 ELF Bare-Metal Target (`aarch64-none-elf`)...

-   [Arm GNU Toolchain Downloads](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)

For Linux x64 and WSL:

-   [gcc-arm-11.2-2022.02-x86_64-aarch64-none-elf.tar.xz](https://developer.arm.com/-/media/Files/downloads/gnu/11.2-2022.02/binrel/gcc-arm-11.2-2022.02-x86_64-aarch64-none-elf.tar.xz)

For macOS:

-   [arm-gnu-toolchain-11.3.rel1-darwin-x86_64-aarch64-none-elf.pkg](https://developer.arm.com/-/media/Files/downloads/gnu/11.3.rel1/binrel/arm-gnu-toolchain-11.3.rel1-darwin-x86_64-aarch64-none-elf.pkg)

(I don't recommend building NuttX on Plain Old Windows CMD, please use WSL instead)

Add it to the `PATH`...

```bash
## For Linux x64 and WSL:
export PATH="$PATH:$HOME/gcc-arm-11.2-2022.02-x86_64-aarch64-none-elf/bin"

## For macOS:
export PATH="$PATH:/Applications/ArmGNUToolchain/11.3.rel1/aarch64-none-elf/bin"
```

Check the toolchain...

```bash
aarch64-none-elf-gcc -v
```

[(Based on the instructions here)](https://github.com/apache/incubator-nuttx/tree/master/boards/arm64/qemu/qemu-a53)

# Download QEMU

Download and install QEMU...

-   [Download QEMU](https://www.qemu.org/download/)

For macOS we may use `brew`...

```bash
brew install qemu
```

# Build NuttX: Single Core

First we build NuttX for a Single Core of Arm Cortex-A53...

```bash
## Configure NuttX for Single Core
./tools/configure.sh -l qemu-a53:nsh

## Build NuttX
make

## Dump the disassembly to nuttx.S
aarch64-none-elf-objdump \
  -t -S --demangle --line-numbers --wide \
  nuttx \
  >nuttx.S \
  2>&1
```

The NuttX Output Files may be found here...

-   [NuttX for Arm Cortex-A53 Single Core](https://github.com/lupyuen/pinephone-nuttx/releases/tag/v1.0.1)

# Test NuttX with QEMU: Single Core

This is how we test NuttX on QEMU with a Single Core of Arm Cortex-A53...

```bash
## Start QEMU (Single Core) with NuttX
qemu-system-aarch64 \
  -cpu cortex-a53 \
  -nographic \
  -machine virt,virtualization=on,gic-version=3 \
  -net none \
  -chardev stdio,id=con,mux=on \
  -serial chardev:con \
  -mon chardev=con,mode=readline \
  -kernel ./nuttx
```

Here's NuttX with a Single Core running on QEMU...

```text
- Ready to Boot CPU
- Boot from EL2
- Boot from EL1
- Boot to C runtime for OS Initialize

nx_start: Entry
up_allocate_heap: heap_start=0x0x402c4000, heap_size=0x7d3c000
gic_validate_dist_version: GICv3 version detect
gic_validate_dist_version: GICD_TYPER = 0x37a0007
gic_validate_dist_version: 224 SPIs implemented
gic_validate_dist_version: 0 Extended SPIs implemented
gic_validate_dist_version: Distributor has no Range Selector support
gic_validate_redist_version: GICD_TYPER = 0x1000011
gic_validate_redist_version: 16 PPIs implemented
gic_validate_redist_version: no VLPI support, no direct LPI support
up_timer_initialize: up_timer_initialize: cp15 timer(s) running at 62.50MHz, cycle 62500
uart_register: Registering /dev/console
uart_register: Registering /dev/ttyS0
work_start_highpri: Starting high-priority kernel worker thread(s)
nx_start_application: Starting init thread
lib_cxx_initialize: _sinit: 0x402a7000 _einit: 0x402a7000 _stext: 0x40280000 _etext: 0x402a8000
nsh: sysinit: fopen failed: 2
nsh: mkfatfs: command not found

NuttShell (NSH) NuttX-10.3.0-RC2
nsh> nx_start: CPU0: Beginning Idle Loop

nsh> help
help usage:  help [-v] [<cmd>]

  .         cd        dmesg     help      mount     rmdir     true      xd        
  [         cp        echo      hexdump   mv        set       truncate  
  ?         cmp       exec      kill      printf    sleep     uname     
  basename  dirname   exit      ls        ps        source    umount    
  break     dd        false     mkdir     pwd       test      unset     
  cat       df        free      mkrd      rm        time      usleep    

Builtin Apps:
  getprime  hello     nsh       ostest    sh        

nsh> uname -a
NuttX 10.3.0-RC2 1e8f2a8 Aug 23 2022 07:04:54 arm64 qemu-a53

nsh> hello
task_spawn: name=hello entry=0x4029b594 file_actions=0x402c9580 attr=0x402c9588 argv=0x402c96d0
spawn_execattrs: Setting policy=2 priority=100 for pid=3
Hello, World!!

nsh> ls /
/:
 dev/
 etc/
 proc/

nsh> ls /dev
/dev:
 console
 null
 ram0
 ram2
 ttyS0
 zero

nsh> ls /proc
/proc:
 0/
 1/
 2/
 meminfo
 memdump
 fs/
 self/
 uptime
 version

nsh> ls /etc
/etc:
 init.d/

nsh> ls /etc/init.d
/etc/init.d:
 rcS

nsh> cat /etc/init.d/rcS
# Create a RAMDISK and mount it at /tmp

mkrd -m 2 -s 512 1024
mkfatfs /dev/ram2
mount -t vfat /dev/ram2 /tmp
```

NuttX is [POSIX Compliant](https://nuttx.apache.org/docs/latest/introduction/inviolables.html), so the developer experience feels very much like Linux. (But much smaller)

And NuttX everything runs in RAM, no File System needed. (For now)

# Build NuttX: Multi Core

From Single Core to Multi Core! Now we build NuttX for 4 Cores of Arm Cortex-A53...

```bash
## Configure NuttX for 4 Cores
./tools/configure.sh -l qemu-a53:nsh_smp

## Build NuttX
make

## Dump the disassembly to nuttx.S
aarch64-none-elf-objdump \
  -t -S --demangle --line-numbers --wide \
  nuttx \
  >nuttx.S \
  2>&1
```

The NuttX Output Files may be found here...

-   [NuttX for Arm Cortex-A53 Multi-Core](https://github.com/lupyuen/pinephone-nuttx/releases/tag/v1.0.0)

# Test NuttX with QEMU: Multi Core

And this is how we test NuttX on QEMU with 4 Cores of Arm Cortex-A53...

```bash
## Start QEMU (4 Cores) with NuttX
qemu-system-aarch64 \
  -smp 4 \
  -cpu cortex-a53 \
  -nographic \
  -machine virt,virtualization=on,gic-version=3 \
  -net none \
  -chardev stdio,id=con,mux=on \
  -serial chardev:con \
  -mon chardev=con,mode=readline \
  -kernel ./nuttx
```

Note that `smp` is set to 4. [(Symmetric Multi-Processing)](https://developer.arm.com/documentation/den0024/a/Multi-core-processors/Multi-processing-systems/Symmetric-multi-processing?lang=en)

Here's NuttX with 4 Cores running on QEMU...

```text
- Ready to Boot CPU
- Boot from EL2
- Boot from EL1
- Boot to C runtime for OS Initialize

[CPU0] psci_detect: Detected PSCI v1.1
[CPU0] nx_start: Entry
[CPU0] up_allocate_heap: heap_start=0x0x402db000, heap_size=0x7d25000
[CPU0] gic_validate_dist_version: GICv3 version detect
[CPU0] gic_validate_dist_version: GICD_TYPER = 0x37a0007
[CPU0] gic_validate_dist_version: 224 SPIs implemented
[CPU0] gic_validate_dist_version: 0 Extended SPIs implemented
[CPU0] gic_validate_dist_version: Distributor has no Range Selector support
[CPU0] gic_validate_redist_version: GICD_TYPER = 0x1000001
[CPU0] gic_validate_redist_version: 16 PPIs implemented
[CPU0] gic_validate_redist_version: no VLPI support, no direct LPI support
[CPU0] up_timer_initialize: up_timer_initialize: cp15 timer(s) running at 62.50MHz, cycle 62500
[CPU0] uart_register: Registering /dev/console
[CPU0] uart_register: Registering /dev/ttyS0

- Ready to Boot CPU
- Boot from EL2
- Boot from EL1
- Boot to C runtime for OS Initialize

[CPU1] gic_validate_redist_version: GICD_TYPER = 0x101000101
[CPU1] gic_validate_redist_version: 16 PPIs implemented
[CPU1] gic_validate_redist_version: no VLPI support, no direct LPI support
[CPU1] nx_idle_trampoline: CPU1: Beginning Idle Loop
[CPU0] arm64_start_cpu: Secondary CPU core 1 (MPID:0x1) is up

- Ready to Boot CPU
- Boot from EL2
- Boot from EL1
- Boot to C runtime for OS Initialize

[CPU2] gic_validate_redist_version: GICD_TYPER = 0x201000201
[CPU2] gic_validate_redist_version: 16 PPIs implemented
[CPU2] gic_validate_redist_version: no VLPI support, no direct LPI support
[CPU2] nx_idle_trampoline: CPU2: Beginning Idle Loop
[CPU0] arm64_start_cpu: Secondary CPU core 2 (MPID:0x2) is up

- Ready to Boot CPU
- Boot from EL2
- Boot from EL1
- Boot to C runtime for OS Initialize

[CPU3] gic_validate_redist_version: GICD_TYPER = 0x301000311
[CPU3] gic_validate_redist_version: 16 PPIs implemented
[CPU3] gic_validate_redist_version: no VLPI support, no direct LPI support
[CPU0] arm64_start_cpu: Secondary CPU core 3 (MPID:0x3) is up
[CPU0] work_start_highpri: Starting high-priority kernel worker thread(s)
[CPU0] nx_start_application: Starting init thread
[CPU3] nx_idle_trampoline: CPU3: Beginning Idle Loop
[CPU0] nx_start: CPU0: Beginning Idle Loop

nsh: sysinit: fopen failed: 2
nsh: mkfatfs: command not found

NuttShell (NSH) NuttX-10.3.0-RC2
nsh> help
help usage:  help [-v] [<cmd>]

  .         cd        dmesg     help      mount     rmdir     true      xd        
  [         cp        echo      hexdump   mv        set       truncate  
  ?         cmp       exec      kill      printf    sleep     uname     
  basename  dirname   exit      ls        ps        source    umount    
  break     dd        false     mkdir     pwd       test      unset     
  cat       df        free      mkrd      rm        time      usleep    

Builtin Apps:
  getprime  hello     nsh       ostest    sh        smp       taskset   

nsh> uname -a
NuttX 10.3.0-RC2 1e8f2a8 Aug 21 2022 15:57:35 arm64 qemu-a53

nsh> hello
[CPU0] task_spawn: name=hello entry=0x4029cee4 file_actions=0x402e52b0 attr=0x402e52b8 argv=0x402e5400
[CPU0] spawn_execattrs: Setting policy=2 priority=100 for pid=6
Hello, World!
```

We see each of the 4 Cores starting NuttX (CPU0 to CPU3). That's so cool!

(Can we use QEMU to partially emulate PinePhone? That would be extremely helpful!)

# Inside NuttX for Cortex-A53

Now we browse the Source Files for the implementation of Cortex-A53 on NuttX.

NuttX treats QEMU as a Target Board (as though it was a dev board). Here are the Source Files and Build Configuration for the QEMU Board...

-   [nuttx/boards/arm64/qemu/qemu-a53](https://github.com/apache/incubator-nuttx/tree/master/boards/arm64/qemu/qemu-a53)

(We'll clone this to create a Target Board for PinePhone)

The Board-Specific Drivers for QEMU are started in [qemu-a53/src/qemu_bringup.c](https://github.com/apache/incubator-nuttx/blob/master/boards/arm64/qemu/qemu-a53/src/qemu_bringup.c)

(We'll start the PinePhone Drivers here)

The QEMU Board calls the QEMU Architecture-Specific Drivers at...

-   [nuttx/arch/arm64/src/qemu](https://github.com/apache/incubator-nuttx/tree/master/arch/arm64/src/qemu)

The UART Driver is located at [qemu/qemu_serial.c](https://github.com/apache/incubator-nuttx/blob/master/arch/arm64/src/qemu/qemu_serial.c) and [qemu/qemu_lowputc.S](https://github.com/apache/incubator-nuttx/blob/master/arch/arm64/src/qemu/qemu_lowputc.S)

(For PinePhone we'll create a UART Driver for Allwinner A64 SoC. I2C, SPI and other Low-Level A64 Drivers will be located here too)

The QEMU Functions (Board and Architecture) call the Arm64 Architecture Functions at...

-   [nuttx/arch/arm64/src/common](https://github.com/apache/incubator-nuttx/tree/master/arch/arm64/src/common)

Which implements all kinds of Arm64 Features: [FPU](https://github.com/apache/incubator-nuttx/blob/master/arch/arm64/src/common/arm64_fpu.c), [Interrupts](https://github.com/apache/incubator-nuttx/blob/master/arch/arm64/src/common/arm64_gicv3.c), [MMU](https://github.com/apache/incubator-nuttx/blob/master/arch/arm64/src/common/arm64_mmu.c), [Tasks](https://github.com/apache/incubator-nuttx/blob/master/arch/arm64/src/common/arm64_task_sched.c), [Timers](https://github.com/apache/incubator-nuttx/blob/master/arch/arm64/src/common/arm64_arch_timer.c)...

(We'll reuse them for PinePhone)

# NuttX Image

Next we analyse the NuttX Image with [Ghidra](https://ghidra-sre.org/), to understand the NuttX Image Header and Startup Code.

Here's the [NuttX ELF Image `nuttx`](https://github.com/lupyuen/pinephone-nuttx/releases/download/v1.0.0/nuttx) analysed by Ghidra...

![Top Part of NuttX Image Header](https://lupyuen.github.io/images/Screenshot%202022-08-22%20at%204.09.55%20PM.png)

Note that the NuttX Image jumps to `real_start` (to skip the Image Header)...

```text
40280000 4d 5a 00 91     add        x13,x18,#0x16
40280004 0f 00 00 14     b          real_start
```

`real_start` is defined at 0x4028 0040 with the Startup Code...

![Bottom Part of NuttX Image Header](https://lupyuen.github.io/images/arm-title.png)

We see something interesting: The Magic Number `ARM\x64` appears at address 0x4028 0038.

Searching the net for this Magic Number reveals that it's actually an Arm64 Linux Kernel Header!

When we refer to the [NuttX Arm64 Disassembly `nuttx.S`](https://github.com/lupyuen/pinephone-nuttx/releases/download/v1.0.0/nuttx.S), we find happiness: [arch/arm64/src/common/arm64_head.S](https://github.com/lupyuen/incubator-nuttx/blob/pinephone/arch/arm64/src/common/arm64_head.S#L79-L117)

```text
    /* Kernel startup entry point.
     * ---------------------------
     *
     * The requirements are:
     *   MMU = off, D-cache = off, I-cache = on or off,
     *   x0 = physical address to the FDT blob.
     *       it will be used when NuttX support device tree in the future
     *
     * This must be the very first address in the loaded image.
     * It should be loaded at any 4K-aligned address.
     */
    .globl __start;
__start:

    /* DO NOT MODIFY. Image header expected by Linux boot-loaders.
     *
     * This add instruction has no meaningful effect except that
     * its opcode forms the magic "MZ" signature of a PE/COFF file
     * that is required for UEFI applications.
     *
     * Some bootloader (such imx8 uboot) checking the magic "MZ" to see
     * if the image is a valid Linux image. but modifying the bootLoader is
     * unnecessary unless we need to do a customize secure boot.
     * so just put the ''MZ" in the header to make bootloader happiness
     */

    add     x13, x18, #0x16      /* the magic "MZ" signature */
    b       real_start           /* branch to kernel start */
    .quad   0x480000              /* Image load offset from start of RAM */
    .quad   _e_initstack - __start         /* Effective size of kernel image, little-endian */
    .quad   __HEAD_FLAGS         /* Informative flags, little-endian */
    .quad   0                    /* reserved */
    .quad   0                    /* reserved */
    .quad   0                    /* reserved */
    .ascii  "ARM\x64"            /* Magic number, "ARM\x64" */
    .long   0                    /* reserved */

real_start:
    /* Disable all exceptions and interrupts */
```

NuttX Image actually follows the Arm64 Linux Kernel Image Format! As defined here...

-   ["Booting AArch64 Linux"](https://www.kernel.org/doc/html/latest/arm64/booting.html)

Arm64 Linux Kernel Image contains a 64-byte header...

```text
u32 code0;                    /* Executable code */
u32 code1;                    /* Executable code */
u64 text_offset;              /* Image load offset, little endian */
u64 image_size;               /* Effective Image size, little endian */
u64 flags;                    /* kernel flags, little endian */
u64 res2      = 0;            /* reserved */
u64 res3      = 0;            /* reserved */
u64 res4      = 0;            /* reserved */
u32 magic     = 0x644d5241;   /* Magic number, little endian, "ARM\x64" */
u32 res5;                     /* reserved (used for PE COFF offset) */
```

Start of RAM is 0x4000 0000. The Image Load Offset in our NuttX Image Header is 0x48 0000 according to [arch/arm64/src/common/arm64_head.S](https://github.com/lupyuen/incubator-nuttx/blob/pinephone/arch/arm64/src/common/arm64_head.S#L107)

```text
    .quad   0x480000              /* Image load offset from start of RAM */
```

This means that our NuttX Image will be loaded at 0x4048 0000.

I wonder if this Image Load Offset should have been 0x28 0000? (Instead of 0x48 0000)

Remember that Ghidra (and the Arm Disassembly) says that our NuttX Image is actually loaded at 0x4028 0000. (Instead of 0x4048 0000)

RAM Size and RAM Start are defined in the NuttX Configuration: [boards/arm64/qemu/qemu-a53/configs/nsh_smp/defconfig](https://github.com/lupyuen/incubator-nuttx/blob/pinephone/boards/arm64/qemu/qemu-a53/configs/nsh_smp/defconfig#L47-L48)

```text
CONFIG_RAM_SIZE=134217728
CONFIG_RAM_START=0x40000000
```

That's 128 MB RAM. Which should fit inside PinePhone's 2 GB RAM.

The NuttX Image was built with this Linker Command, based on `make --trace`...

```bash
aarch64-none-elf-ld \
  --entry=__start \
  -nostdlib \
  --cref \
  -Map=nuttx/nuttx/nuttx.map \
  -Tnuttx/nuttx/boards/arm64/qemu/qemu-a53/scripts/dramboot.ld  \
  -L nuttx/nuttx/staging \
  -L nuttx/nuttx/arch/arm64/src/board  \
  -o nuttx/nuttx/nuttx arm64_head.o  \
  --start-group \
  -lsched \
  -ldrivers \
  -lboards \
  -lc \
  -lmm \
  -larch \
  -lapps \
  -lfs \
  -lbinfmt \
  -lboard /Applications/ArmGNUToolchain/11.3.rel1/aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/11.3.1/libgcc.a /Applications/ArmGNUToolchain/11.3.rel1/aarch64-none-elf/bin/../lib/gcc/aarch64-none-elf/11.3.1/../../../../aarch64-none-elf/lib/libm.a \
  --end-group
```

NuttX Image begins at `__start`, which is defined as 0x4028 0000 in the NuttX Linker Script: [boards/arm64/qemu/qemu-a53/scripts/dramboot.ld](https://github.com/lupyuen/incubator-nuttx/blob/pinephone/boards/arm64/qemu/qemu-a53/scripts/dramboot.ld#L30-L33)

```text
SECTIONS
{
  . = 0x40280000;  /* uboot load address */
  _start = .;
```

We'll change this to 0x4000 0000 for PinePhone, since Start of RAM is 0x4000 0000 and Image Load Offset is 0. (See below)

We've seen the NuttX Image (which looks like a Linux Kernel Image), let's compare with a PinePhone Linux Kernel Image and see how NuttX needs to be tweaked...

# PinePhone Image

Will NuttX run on PinePhone? Let's analyse a PinePhone Linux Kernel Image with Ghidra, to look at the Linux Kernel Header and Startup Code.

We'll use the PinePhone Jumpdrive Image, since it's small...

https://github.com/dreemurrs-embedded/Jumpdrive

Download https://github.com/dreemurrs-embedded/Jumpdrive/releases/download/0.8/pine64-pinephone.img.xz

Expand `pine64-pinephone.img.xz`

Expand the files inside...

```bash
gunzip Image.gz
gunzip initramfs.gz
tar xvf initramfs
```

Import the uncompressed `Image` (Linux Kernel) into Ghidra.

For "Language" select AARCH64:LE:v8A:default...
-   Processor: AARCH64 
-   Variant: v8A 
-   Size: 64 
-   Endian: little 
-   Compiler: default

![For "Language" select AARCH64:LE:v8A:default](https://lupyuen.github.io/images/Screenshot%202022-08-22%20at%203.39.06%20PM.png)

Here's the Jumpdrive `Image` (Linux Kernel) in Ghidra...

![Jumpdrive Image in Ghidra](https://lupyuen.github.io/images/Screenshot%202022-08-22%20at%205.40.58%20PM.png)

According to the Linux Kernel Header...

-   ["Booting AArch64 Linux"](https://www.kernel.org/doc/html/latest/arm64/booting.html)

We see Linux Kernel Magic Number `ARM\x64` at offset 0x38.

Image Load Offset is 0, according to the header.

Start of RAM is 0x4000 0000 according to this Memory Map...

https://linux-sunxi.org/A64/Memory_map

So we shift `Image` in Ghidra to start at 0x4000 0000...

-   Click Window > Memory Map

-   Click "ram"

-   Click the 4-Arrows icon ("Move a block to another address")

-   Change "New Start Address" to 40000000

![Change Start Address to 40000000](https://lupyuen.github.io/images/Screenshot%202022-08-21%20at%207.07.15%20PM.png)

Note that the first instruction at 0x4000 0000 jumps to 0x4081 0000 (to skip the Linux Kernel Header)...

```text
40000000 00 40 20 14     b          FUN_40810000
```

(Note: The magic "MZ" signature is not needed)

The Linux Kernel Code actually begins at 0x4081 0000...

![Linux Kernel Code actually begins at 0x4081 0000](https://lupyuen.github.io/images/Screenshot%202022-08-22%20at%205.53.58%20PM.png)

# Will NuttX Boot On PinePhone?

_So will NuttX boot on PinePhone?_

It's highly plausible! We discovered (with happiness) that NuttX already generates an Arm64 Linux Kernel Header.

So NuttX could be a drop-in replacement for the PinePhone Linux Kernel! We just need to...

-   Write PinePhone Jumpdrive to a microSD Card (with Etcher, in FAT format)

-   Overwrite `Image.gz` by the (gzipped) NuttX Binary Image `nuttx.bin.gz`

-   Insert the microSD Card into PinePhone

-   Power on PinePhone

And NuttX should (theoretically) boot on PinePhone!

As mentioned earlier, we should rebuild NuttX so that `__start` is changed to 0x4000 0000 (from 0x4028 0000), as defined in the NuttX Linker Script: [boards/arm64/qemu/qemu-a53/scripts/dramboot.ld](https://github.com/lupyuen/incubator-nuttx/blob/pinephone/boards/arm64/qemu/qemu-a53/scripts/dramboot.ld#L30-L33)

```text
SECTIONS
{
  /* TODO: Change to 0x4000000 for PinePhone */
  . = 0x40280000;  /* uboot load address */
  _start = .;
```

Also the Image Load Offset in our NuttX Image Header should be changed to 0x0 (from 0x48 0000): [arch/arm64/src/common/arm64_head.S](https://github.com/lupyuen/incubator-nuttx/blob/pinephone/arch/arm64/src/common/arm64_head.S#L107)

```text
    /* TODO: Change to 0x0 for PinePhone */
    .quad   0x480000              /* Image load offset from start of RAM */
```

We'll increase the RAM Size to 2 GB (from 128 MB): [boards/arm64/qemu/qemu-a53/configs/nsh_smp/defconfig](https://github.com/lupyuen/incubator-nuttx/blob/pinephone/boards/arm64/qemu/qemu-a53/configs/nsh_smp/defconfig#L47-L48)

```text
/* TODO: Increase to 2 GB for PinePhone */
CONFIG_RAM_SIZE=134217728
CONFIG_RAM_START=0x40000000
```

_But will we see anything when NuttX boots on PinePhone?_

Not yet. We'll need to implement the UART Driver for NuttX...

# UART Driver for NuttX

We won't see any output from NuttX until we implement the UART Driver for NuttX.

These are the Source Files for the QEMU UART Driver (PL011)...

-   [arch/arm64/src/qemu/qemu_serial.c](https://github.com/apache/incubator-nuttx/blob/master/arch/arm64/src/qemu/qemu_serial.c)

-   [arch/arm64/src/qemu/qemu_lowputc.S](https://github.com/apache/incubator-nuttx/blob/master/arch/arm64/src/qemu/qemu_lowputc.S)

    [(More about PL011 UART)](https://krinkinmu.github.io/2020/11/29/PL011.html)

We'll replace the code above with the UART Driver for Allwinner A64 SoC...

-   [UART0 Memory Map](https://linux-sunxi.org/A64/Memory_map)

-   [Allwinner A64 UART](https://linux-sunxi.org/UART)

-   [Allwinner A64 User Manual](https://linux-sunxi.org/File:Allwinner_A64_User_Manual_V1.1.pdf)

-   [Allwinner A64 Info](https://linux-sunxi.org/A64)

To access the UART Port on PinePhone, we'll use this USB Serial Debug Cable...

-   [PinePhone Serial Debug Cable](https://wiki.pine64.org/index.php/PinePhone#Serial_console)

Which connects to the Headphone Port. Genius!

# TODO

TODO: Configure NuttX Memory Regions for Allwinner A64 SoC

TODO: Copy NuttX to microSD Card

A64 Boot ROM: https://linux-sunxi.org/BROM#A64

A64 U-Boot: https://linux-sunxi.org/U-Boot

A64 U-Boot SPL: https://linux-sunxi.org/BROM#U-Boot_SPL_limitations

SD Card Layout: https://linux-sunxi.org/Bootable_SD_card#SD_Card_Layout

TODO: Boot NuttX on PinePhone and test NuttX Shell

TODO: Build NuttX Drivers for PinePhone's LCD Display, Touch Panel, LTE Modem, WiFi, BLE, Power Mgmt, ...

TODO: From [Alan Carvalho de Assis](https://www.linkedin.com/in/acassis/)

-   I ran NuttX on PCDuino (ARM Cortex-A9 I think)

-   also NuttX on iMX6 and BeagleBoneBlack

-   nice to try evolve NuttX on Desktop direction

-   Tom Window Manager that Greg ported to NuttX

-   TODO: port NanoX (nxlib/microwindows) it could open doors to port X11 graphic applications from Linux

TODO: Boot Files for Manjaro Phosh on PinePhone:

```text
[manjaro@manjaro-arm ~]$ ls -l /boot
total 38568
-rw-r--r-- 1 root root     1476 Jun 22 08:36 boot.scr
-rw-r--r-- 1 root root     1404 Apr  6 11:51 boot.txt
drwxr-xr-x 3 root root     4096 Oct 16  2021 dtbs
-rw-r--r-- 1 root root 20160520 Jul  3 14:56 Image
-rw-r--r-- 1 root root  8359044 Jul  3 14:56 Image.gz
-rw-r--r-- 1 root root  7327835 Jul 24 14:33 initramfs-linux.img
-rw-r--r-- 1 root root   722223 Apr  6 11:51 u-boot-sunxi-with-spl-pinephone-492.bin
-rw-r--r-- 1 root root   722223 Apr  6 11:51 u-boot-sunxi-with-spl-pinephone-528.bin
-rw-r--r-- 1 root root   722223 Apr  6 11:51 u-boot-sunxi-with-spl-pinephone-552.bin
-rw-r--r-- 1 root root   722223 Apr  6 11:51 u-boot-sunxi-with-spl-pinephone-592.bin
-rw-r--r-- 1 root root   722223 Apr  6 11:51 u-boot-sunxi-with-spl-pinephone-624.bin

[manjaro@manjaro-arm ~]$ ls -l /boot/dtbs
total 8
drwxr-xr-x 2 root root 8192 Jul 24 14:30 allwinner

[manjaro@manjaro-arm ~]$ ls -l /boot/dtbs/allwinner
total 1504
-rw-r--r-- 1 root root 13440 Jul  3 14:56 sun50i-a100-allwinner-perf1.dtb
-rw-r--r-- 1 root root 41295 Jul  3 14:56 sun50i-a64-amarula-relic.dtb
-rw-r--r-- 1 root root 41648 Jul  3 14:56 sun50i-a64-bananapi-m64.dtb
-rw-r--r-- 1 root root 40512 Jul  3 14:56 sun50i-a64-nanopi-a64.dtb
-rw-r--r-- 1 root root 39951 Jul  3 14:56 sun50i-a64-oceanic-5205-5inmfd.dtb
-rw-r--r-- 1 root root 41268 Jul  3 14:56 sun50i-a64-olinuxino.dtb
-rw-r--r-- 1 root root 41397 Jul  3 14:56 sun50i-a64-olinuxino-emmc.dtb
-rw-r--r-- 1 root root 42295 Jul  3 14:56 sun50i-a64-orangepi-win.dtb
-rw-r--r-- 1 root root 40316 Jul  3 14:56 sun50i-a64-pine64.dtb
-rw-r--r-- 1 root root 40948 Jul  3 14:56 sun50i-a64-pine64-lts.dtb
-rw-r--r-- 1 root root 40438 Jul  3 14:56 sun50i-a64-pine64-plus.dtb
-rw-r--r-- 1 root root 42979 Jul  3 14:56 sun50i-a64-pinebook.dtb
-rw-r--r-- 1 root root 53726 Jul  3 14:56 sun50i-a64-pinephone-1.0.dtb
-rw-r--r-- 1 root root 53753 Jul  3 14:56 sun50i-a64-pinephone-1.1.dtb
-rw-r--r-- 1 root root 53718 Jul  3 14:56 sun50i-a64-pinephone-1.2.dtb
-rw-r--r-- 1 root root 44110 Jul  3 14:56 sun50i-a64-pinetab.dtb
-rw-r--r-- 1 root root 44150 Jul  3 14:56 sun50i-a64-pinetab-early-adopter.dtb
-rw-r--r-- 1 root root 40816 Jul  3 14:56 sun50i-a64-sopine-baseboard.dtb
-rw-r--r-- 1 root root 42234 Jul  3 14:56 sun50i-a64-teres-i.dtb
-rw-r--r-- 1 root root 31407 Jul  3 14:56 sun50i-h5-bananapi-m2-plus.dtb
-rw-r--r-- 1 root root 32846 Jul  3 14:56 sun50i-h5-bananapi-m2-plus-v1.2.dtb
-rw-r--r-- 1 root root 31056 Jul  3 14:56 sun50i-h5-emlid-neutis-n5-devboard.dtb
-rw-r--r-- 1 root root 31277 Jul  3 14:56 sun50i-h5-libretech-all-h3-cc.dtb
-rw-r--r-- 1 root root 29939 Jul  3 14:56 sun50i-h5-libretech-all-h3-it.dtb
-rw-r--r-- 1 root root 31872 Jul  3 14:56 sun50i-h5-libretech-all-h5-cc.dtb
-rw-r--r-- 1 root root 29013 Jul  3 14:56 sun50i-h5-nanopi-neo2.dtb
-rw-r--r-- 1 root root 29704 Jul  3 14:56 sun50i-h5-nanopi-neo-plus2.dtb
-rw-r--r-- 1 root root 31401 Jul  3 14:56 sun50i-h5-nanopi-r1s-h5.dtb
-rw-r--r-- 1 root root 31082 Jul  3 14:56 sun50i-h5-orangepi-pc2.dtb
-rw-r--r-- 1 root root 29806 Jul  3 14:56 sun50i-h5-orangepi-prime.dtb
-rw-r--r-- 1 root root 29044 Jul  3 14:56 sun50i-h5-orangepi-zero-plus2.dtb
-rw-r--r-- 1 root root 29131 Jul  3 14:56 sun50i-h5-orangepi-zero-plus.dtb
-rw-r--r-- 1 root root 31911 Jul  3 14:56 sun50i-h6-beelink-gs1.dtb
-rw-r--r-- 1 root root 33042 Jul  3 14:56 sun50i-h6-orangepi-3.dtb
-rw-r--r-- 1 root root 30504 Jul  3 14:56 sun50i-h6-orangepi-lite2.dtb
-rw-r--r-- 1 root root 30287 Jul  3 14:56 sun50i-h6-orangepi-one-plus.dtb
-rw-r--r-- 1 root root 32368 Jul  3 14:56 sun50i-h6-pine-h64.dtb
-rw-r--r-- 1 root root 32882 Jul  3 14:56 sun50i-h6-pine-h64-model-b.dtb
-rw-r--r-- 1 root root 29544 Jul  3 14:56 sun50i-h6-tanix-tx6.dtb
-rw-r--r-- 1 root root 29305 Jul  3 14:56 sun50i-h6-tanix-tx6-mini.dtb

[manjaro@manjaro-arm ~]$ cat /boot/boot.txt
#
# /boot/boot.txt
# After modifying, run "pp-uboot-mkscr" to re-generate the U-Boot boot script.
#

#
# This is the description of the GPIO lines used in this boot script:
#
# GPIO #98 is PD2, or A64 ball W19, which controls the vibrator motor
# GPIO #114 is PD18, or A64 ball AB13, which controls the red part of the multicolor LED
# GPIO #115 is PD19, or A64 ball AB12, which controls the green part of the multicolor LED
# GPIO #116 is PD20, or A64 ball AB11, which controls the blue part of the multicolor LED
#

gpio set 98
gpio set 114

# Set root partition to the second partition of boot device
part uuid ${devtype} ${devnum}:1 uuid_boot
part uuid ${devtype} ${devnum}:2 uuid_root

setenv bootargs loglevel=4 console=tty0 console=${console} earlycon=uart,mmio32,0x01c28000 consoleblank=0 boot=PARTUUID=${uuid_boot} root=PARTUUID=${uuid_root} rw rootwait quiet audit=0 bootsplash.bootfile=bootsplash-themes/manjaro/bootsplash

if load ${devtype} ${devnum}:${distro_bootpart} ${kernel_addr_r} /Image; then
  gpio clear 98
  if load ${devtype} ${devnum}:${distro_bootpart} ${fdt_addr_r} /dtbs/${fdtfile}; then
    if load ${devtype} ${devnum}:${distro_bootpart} ${ramdisk_addr_r} /initramfs-linux.img; then
      gpio set 115
      booti ${kernel_addr_r} ${ramdisk_addr_r}:${filesize} ${fdt_addr_r};
    else
      gpio set 116
      booti ${kernel_addr_r} - ${fdt_addr_r};
    fi;
  fi;
fi

# EOF
```
