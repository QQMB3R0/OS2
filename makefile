CC = i686-elf-g++
LD = ld
ASM = nasm
# test -O2 flag
CFLAGS = -c -m32 -Wall -ffreestanding -nostdinc -nostdlib -lgcc
AFLAGS = -f elf32
LDFLAGS = -m elf_i386
CFLAGS =  -c  -m32 -Wall -ffreestanding -nostdinc -nostdlib -lgcc
AFLAGS =  -f  elf32
LDFLAGS = -m elf_i386

CFILES = $(shell find ./ -type f \( -name \*.cpp -o -name \*.c \))
AFILES = $(shell find ./ -type f \( -iname \*.s -o -name \*.asm \))
LDFILE = $(shell find ./ -type f -name *.ld)
SOURCE_FILES = $(CFILES) $(AFILES)
OBJ_FILES = $(addprefix $(BDIR)/, $(addsuffix .o, $(basename $(SOURCE_FILES))))

ifeq ($(DEBUG), true)
	CFLAGS := -g3 $(CFLAGS)
	AFLAGS := -g $(AFLAGS)
	QEMUFLAGS := -s -S $(QEMUFLAGS)
endif

OS_BINARY = $(BDIR)/kernel.bin
BDIR = ./build

.PHONY: install $(SOURCE_FILES)

build: startbuild $(SOURCE_FILES)
	echo $(AFLAGS)
	echo $(CFLAGS)
OS_BINARY = $(BDIR)/kernel.bin
BDIR = ./build

.PHONY: install $(SOURCE_FILES)

build: startbuild $(SOURCE_FILES)
	@echo "Link object files..."
	$(LD) $(LDFLAGS) -o $(OS_BINARY) -T $(LDFILE) $(OBJ_FILES)
	@echo "Project was built"

$(CFILES):
	mkdir -p $(BDIR)/$(@D)
	$(CC) $(CFLAGS) -o $(BDIR)/$(addsuffix .o, $(basename $@)) $@

$(AFILES):
	mkdir -p $(BDIR)/$(@D)
	$(ASM) $(AFLAGS) -o $(BDIR)/$(addsuffix .o, $(basename $@)) $@

startbuild:
	@echo $(OBJ_FILES)
	mkdir $(BDIR)
	@echo "Compile source files..."

install:
	@echo "Creating an iso image of OS"
	mkdir -p ./iso/os/
	cp ./$(BDIR)/kernel.bin ./iso/os/
	grub-mkrescue -o ./$(BDIR)/kernel.iso ./iso

clean:
	rm -rf $(BDIR)
	rm -rf ./iso/os/kernel.bin

run:
	 qemu-system-i386 $(QEMUFLAGS) -cdrom ./$(BDIR)/kernel.iso

all: clean build install run
