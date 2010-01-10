
.DELETE_ON_ERROR:

# Prove we started from main Makefile
export STARTEDATTOP=true


#  -D CFG_SPI	 	Enable SPI support (disablee M25LC512 and uIP if not defined)
#  -D CFG_LCD           Enable LCD support (edit lcd/Makefile for 4 vs 8 bit LCDs)
#  -D CFG_FATFS         Enable SD/MMC FatFS support
#  -D CFG_UIP           Enable uIP w/ ENC28J60 on SPI0
#
#export LPC2148_OPTS=-D CFG_UART -D CFG_SPI -D CFG_LCD -D CFG_FATFS -D CFG_UIP
LPC2148_OPTS=-DUART0_DEBUG

SUBDIRS = arch/lpc21xx/uart arch/lpc21xx/enc28j60 arch/lpc21xx/spi_eth lib #arch/lpc21xx/lcd arch/lpc21xx/clock #arch/lpc21xx/timer arch/lpc21xx/mmc uip apps/hello-world
TARGET = main

SRC = $(TARGET).c syscalls.c
#SRC = $(TARGET).c
ASRC=startup.S
LDSCRIPT=lpc2148-flash

# Commands
export CC=arm-elf-gcc
export AR=arm-elf-ar
export OBJCOPY=arm-elf-objcopy
export OBJDUMP=arm-elf-objdump
export ELFSIZE=arm-elf-size

export ROOT=$(shell pwd)
export COMMON=$(ROOT)/common

INCLUDES = -I$(ROOT)/lib -I$(ROOT)/arch/lpc21xx/spi_eth -I$(ROOT)/arch/lpc21xx/uart -I$(ROOT)/arch/lpc21xx/lcd -I$(ROOT)/uip -I$(ROOT)/arch/lpc21xx/enc28j60 -I$(ROOT)/arch/lpc21xx/clock -I$(ROOT)/arch/lpc21xx/timer

export BASEINCLUDE=-I$(ROOT) -I$(ROOT)/include

# Define all object files.
COBJ      = $(SRC:.c=.o) 
AOBJ      = $(ASRC:.S=.o)

#
# Flags
#
WARNINGS = -Wall 
#WARNINGS += -Werror
#WARNINGS += -Wextra 
#WARNINGS += -Wshadow 
#WARNINGS += -Wpointer-arith 
#WARNINGS += -Wbad-function-cast 
#WARNINGS += -Wcast-align 
#WARNINGS += -Wsign-compare 
#WARNINGS += -Waggregate-return 
#WARNINGS += -Wstrict-prototypes 
#WARNINGS += -Wmissing-prototypes 
#WARNINGS += -Wmissing-declarations 
#WARNINGS += -Wunused
export WARNINGS

CFLAGS = $(WARNINGS)
CFLAGS += $(INCLUDES) 
CFLAGS += $(BASEINCLUDE)
CFLAGS += -mcpu=arm7tdmi 
CFLAGS += -gdwarf-2
CFLAGS += -Os
CFLAGS += -std=gnu99
CFLAGS += $(LPC2148_OPTS)
export CFLAGS

LDFLAGS = $(COMMON)/common.a -lc -lm -lgcc -nostartfiles -T$(LDSCRIPT).ld --relocatable
export LDFLAGS

# Assembly flags (to as via gcc)
ASFLAGS = -I. -x assembler-with-cpp
ASFLAGS += -mcpu=arm7tdmi
ASFLAGS += $(ADEFS) 
ASFLAGS += -Wa,--gdwarf-2

# Define all listing files.
#LST = $(ASRC:.S=.lst) $(SRC:.c=.lst) 

.PHONY: all build bin elf hex
all: build

build: bin
bin: subdirs hex
	@echo "Done bin"

elf: $(TARGET).elf
hex: $(TARGET).hex

.PHONY: subdirs $(SUBDIRS)
subdirs: $(SUBDIRS)
	@echo "Done subdirs"

# Make all SUBDIRS
$(SUBDIRS):
	$(MAKE) -C $@

#
# Program the device
#

# verbose output:
# LPC21ISP_DEBUG = -debug
LPC21ISP = lpc21isp
LPC21ISP_BAUD = 19200
LPC21ISP_XTAL = 12000
LPC21ISP_FLASHFILE = $(TARGET).hex
LPC21ISP_PORT = /dev/ttyUSB0
# enter bootloader via RS232 DTR/RTS (only if hardware supports this feature - see Philips AppNote):
LPC21ISP_CONTROL = -control

.PHONY: program
program: bin
	$(LPC21ISP) -term $(LPC21ISP_CONTROL) $(LPC21ISP_DEBUG) $(LPC21ISP_FLASHFILE) $(LPC21ISP_PORT) $(LPC21ISP_BAUD) $(LPC21ISP_XTAL)
	@echo "Download done."

# Create final output file (.hex) from ELF output file.
%.hex: %.elf
	$(OBJCOPY) --strip-debug -O ihex $< $@

# Link: create ELF output file from object files.
.SECONDARY : $(TARGET).elf
.PRECIOUS : $(AOBJ) $(COBJ)
%.elf: $(AOBJ) $(COBJ) $(LDSCRIPT).ld $(COMMON)/common.a
	$(CC) $(CFLAGS) $(AOBJ) $(COBJ) --output $@ $(LDFLAGS)
	$(ELFSIZE) -A $(TARGET).elf

# Compile: create object files from C source files.
$(COBJ) : %.o : %.c Makefile .depend
	$(CC) -c $(CFLAGS) $(GENDEPFLAGS) $< -o $@ 

# Assemble: create object files from assembler source files.
$(AOBJ) : %.o : %.S
	$(CC) -c $(ASFLAGS) $< -o $@

#
#  The .depend files contains the list of header files that the
#  various source files depend on.  By doing this, we'll only
#  rebuild the .o's that are affected by header files changing.
#
.depend:
	$(CC) $(CFLAGS) -M $(SRC) -o .depend

#
#  Utility targets
#
.PHONY: tags
tags :
	@rm -f tags ctags
	find . -name \*.c -exec ctags -a {} \;
	find . -name \*.h -exec ctags -a {} \;

.PHONY: clean
clean :
	find . -name \*.o -exec rm -f {} \;
	find . -name .depend -exec rm -f {} \;
	rm -f *.map *.lst *.elf *.hex .depend sizes.csv tags
	rm $(COMMON)/common.a

ifeq (.depend,$(wildcard .depend))
include .depend
endif
