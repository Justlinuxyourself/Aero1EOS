# --- AliOS 4 Master Build System ---
SRCDIR = src
OBJDIR = obj
BIN = alios4.bin
ISO = alios4.iso
PAYLOAD_H = $(SRCDIR)/section4_shell/grub_payload.h

C_SOURCES = $(shell find $(SRCDIR) -name '*.c')
ASM_SOURCES = $(shell find $(SRCDIR) -name '*.asm')

# Filter out the payload header from source lists if accidentally caught
OBJ = $(C_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
OBJ += $(ASM_SOURCES:$(SRCDIR)/%.asm=$(OBJDIR)/%.o)

CFLAGS = -m64 -c -ffreestanding -fno-stack-protector -Iinclude -Wall -Wextra
LDFLAGS = -n -T linker.ld --build-id=none -z max-page-size=0x1000 --no-warn-rwx-segments -Map=link.map

# The default rule now ensures the GRUB payload is generated BEFORE compiling C files
all: $(PAYLOAD_H) $(ISO)

# --- GRUB Payload Generation ---
$(PAYLOAD_H):
	@echo "--- GENERATING AUTO-BOOT GRUB PAYLOAD ---"
	@# -c embeds the config, multiboot2 adds the command support
	grub-mkimage -o grub_core.img -O i386-pc \
		-c early_grub.cfg \
		-p /boot/grub \
		biosdisk part_msdos fat normal multiboot multiboot2
	
	@# Copy the standard MBR boot image
	cp /usr/lib/grub/i386-pc/boot.img .
	
	@# Convert to C header
	@echo "/* Auto-generated GRUB payload for AliOS Installer */" > $(PAYLOAD_H)
	xxd -i boot.img >> $(PAYLOAD_H)
	xxd -i grub_core.img >> $(PAYLOAD_H)
	
	rm boot.img grub_core.img
	@echo "--- PAYLOAD HEADER GENERATED ---"


# --- Main Build Rules ---
$(BIN): $(OBJ)
	@echo "--- LINKING ALIOS BINARY ---"
	ld $(LDFLAGS) -o $(BIN) $(OBJDIR)/section1_cpu/boot.o $(filter-out $(OBJDIR)/section1_cpu/boot.o, $(OBJ))

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(PAYLOAD_H)
	@mkdir -p $(@D)
	gcc $(CFLAGS) $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.asm
	@mkdir -p $(@D)
	nasm -f elf64 $< -o $@

# --- ISO Creation (For the initial installation media) ---
$(ISO): $(BIN)
	@echo "--- CREATING BOOTABLE ISO ---"
	@mkdir -p isodir/boot/grub
	cp $(BIN) isodir/boot/
	@# Ensure a basic grub.cfg exists for the ISO
	@if [ ! -f grub.cfg ]; then \
		echo 'set timeout=5' > isodir/boot/grub/grub.cfg; \
		echo 'set default=0' >> isodir/boot/grub/grub.cfg; \
		echo 'menuentry "AliOS 4.0 (Installer Mode)" {' >> isodir/boot/grub/grub.cfg; \
		echo '    multiboot2 /boot/alios4.bin' >> isodir/boot/grub/grub.cfg; \
		echo '    boot' >> isodir/boot/grub/grub.cfg; \
		echo '}' >> isodir/boot/grub/grub.cfg; \
	else \
		cp grub.cfg isodir/boot/grub/; \
	fi
	grub-mkrescue -o $(ISO) isodir

clean:
	@echo "--- CLEANING PROJECT ---"
	rm -rf $(OBJDIR) $(BIN) $(ISO) isodir $(PAYLOAD_H) link.map

.PHONY: all clean grub_payload
