#!/bin/sh

# Exit immediately if any command fails
set -e

# Toolchain Definitions
CC="aarch64-linux-gnu-gcc"
LD="aarch64-linux-gnu-ld"
OBJCOPY="aarch64-linux-gnu-objcopy"

# Compilation & Linker Flags
CFLAGS="-Wall -O2 -ffreestanding -nostdlib -nostartfiles -fno-stack-protector -fno-builtin"
LDFLAGS="-T linker-a06.ld -nostdlib"

echo "=== Cleaning old build artifacts ==="
rm -f *.o *.elf kernel8.img boot.img

echo "=== Compiling Source Files ==="

echo "Assembling entry.S..."
$CC $CFLAGS -c entry.S -o entry.o

# Compile C Files
echo "Compiling kernel.c..."
$CC $CFLAGS -c kernel.c -o kernel.o

echo "Compiling framebuffer-a06.c..."
$CC $CFLAGS -c framebuffer-a06.c -o framebuffer-a06.o

echo "Compiling font.c..."
$CC $CFLAGS -c font.c -o font.o

echo "Compiling shell.c..."
$CC $CFLAGS -c shell.c -o shell.o


echo "=== Linking Kernel Object Files ==="
$LD $LDFLAGS entry.o kernel.o framebuffer-a06.o font.o shell.o -o kernel8.elf

echo "=== Generating Raw Binary Flat Image ==="
$OBJCOPY -O binary kernel8.elf kernel8.img

echo "=== Build Successful! kernel8.img is ready ==="

echo "=== Packaging Raw Boot Image with mkbootimg ==="

# Validation checks for assets
if [ ! -f "dtba06" ]; then
    echo "⚠️ ERROR: Missing 'dtba06' file in this directory!"
    echo "Please extract the stock DTB from your A065FXXS7BYJ1 firmware and place it here."
    exit 1
fi

# Create a blank/dummy ramdisk because modern Android boot headers require the field populated
if [ ! -f "dummy_ramdisk.cpio" ]; then
    echo "Creating empty dummy ramdisk..."
    touch empty_file
    echo empty_file | cpio -o -H newc > dummy_ramdisk.cpio
    rm empty_file
fi

echo "Building Android boot image..." 
    mkbootimg \
    --kernel kernel8.img \
    --ramdisk dummy_ramdisk.cpio \
    --dtb dtba06 \
    --base 0x40000000 \
    --kernel_offset 0x00080000 \
    --ramdisk_offset 0x04000000 \
    --tags_offset 0x00000100 \
    --dtb_offset 0x07000000 \
    --pagesize 2048 \
    --header_version 2 \
    --cmdline "console=ttynull stack_depot_disable=on cgroup_disable=pressure kasan.stacktrace=off kvm-arm.mode=protected bootconfig ioremap_guard" \
    -o boot.img



echo "=== SUCCESS: Raw bare-metal boot.img is ready for Samsung A06! ==="
echo "=== DTB is from A065FXXS7BYJ1 ==="
