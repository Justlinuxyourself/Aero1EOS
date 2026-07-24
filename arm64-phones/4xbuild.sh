#!/bin/sh

set -e

CC="arm-linux-gnueabihf-gcc"
LD="arm-linux-gnueabihf-ld"
OBJCOPY="arm-linux-gnueabihf-objcopy"

# Compilation & Linker Flags
CFLAGS="-Wall -O2 -ffreestanding -nostdlib -nostartfiles -fno-stack-protector -fno-builtin"
LDFLAGS="-T linker-4x.ld -nostdlib"

echo "=== Cleaning old build artifacts ==="
rm -f *.o *.elf kernel8.img boot.img

echo "=== Compiling Source Files ==="

echo "Assembling entry.S..."
$CC $CFLAGS -c entry-4x.S -o entry-4x.o

# Compile C Files
echo "Compiling kernel.c..."
$CC $CFLAGS -c kernel.c -o kernel.o

echo "Compiling framebuffer-4x.c..."
$CC $CFLAGS -c framebuffer-4x.c -o framebuffer-4x.o

echo "Compiling font.c..."
$CC $CFLAGS -c font.c -o font.o

echo "Compiling shell.c..."
$CC $CFLAGS -c shell.c -o shell.o


echo "=== Linking Kernel Object Files ==="
$LD $LDFLAGS entry-4x.o kernel.o framebuffer-4x.o font.o shell.o -o kernel8.elf

echo "=== Generating Raw Binary Flat Image ==="
$OBJCOPY -O binary kernel8.elf kernel8.img

echo "=== Build Successful! kernel8.img is ready ==="

echo "=== Packaging Raw Boot Image with mkbootimg ==="

# Validation checks for assets
if [ ! -f "dtb4x" ]; then
    echo "⚠️ ERROR: Missing 'dtb4x' file in this directory!"
    echo "Please extract the stock DTB from your Redmi firmware and place it here."
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
    --dtb dtb4x \
    --base 0x80000000 \
    --kernel_offset 0x00008000 \
    --ramdisk_offset 0x02000000 \
    --tags_offset 0x00000100 \
    --dtb_offset 0x01e00000 \
    --pagesize 2048 \
    --header_version 2 \
    --cmdline "console=ttyMSM0,115200n8 earlycon=msm_hsl_serial,0x78af000 androidboot.selinux=permissive" \
    -o boot.img


echo "=== SUCCESS: Raw bare-metal boot.img is ready for Redmi 4X! ==="
echo "=== DTB is from  ==="
