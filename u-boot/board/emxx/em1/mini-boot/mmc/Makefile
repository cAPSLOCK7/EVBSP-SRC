# EM1 MMC MINI BootLoader Makefile

include $(TOPDIR)/config.mk
include $(TOPDIR)/include/config.mk
include ../../config.mk

MINI_TEXTADDR = 0xa0000000

U_BOOT_SIZE = $(shell ls -l $(TOPDIR)/u-boot.bin | awk '{print $$5}')

DEF_FLAGS = -DEMXX_MINIBOOT -DU_BOOT_TEXTADDR=$(TEXT_BASE) -DU_BOOT_SIZE=$(U_BOOT_SIZE)
DEF_FLAGS += -DDEBUG

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OC = $(CROSS_COMPILE)objcopy
OD = $(CROSS_COMPILE)objdump
NM = $(CROSS_COMPILE)nm

# subdir
MAIN_DIR    = ../main
UBOOT_DIR   = ../..

MINI_BOOT = mini-boot
MINI_EMMC = u-boot-emmc
MINI_ESD  = u-boot-esd
MINI_SD   = sdboot

# sources
MAIN_C_SRCS    = etc.c
MAIN_S_SRCS    = mini_start.S
MMC_C_SRCS     = mmc.c sdc.c
UBOOT_C_SRCS   = auto_calibration.c
UBOOT_S_SRCS   = lowlevel_init.S

MAIN_C_SRCS2    = $(addprefix $(MAIN_DIR)/, $(MAIN_C_SRCS))
MAIN_S_SRCS2    = $(addprefix $(MAIN_DIR)/, $(MAIN_S_SRCS))
UBOOT_C_SRCS2   = $(addprefix $(UBOOT_DIR)/, $(UBOOT_C_SRCS))
UBOOT_S_SRCS2   = $(addprefix $(UBOOT_DIR)/, $(UBOOT_S_SRCS))

# MINI-Boot object
MINI_LD_BASE = ./mini_ld.base
MINI_LD_SCRIPT = mini_ld.script
MINI_LFLAGS = -T $(MINI_LD_SCRIPT)

MMC_C_OBJS = $(MMC_C_SRCS:.c=.o)

MAIN_C_OBJS = $(MAIN_C_SRCS2:.c=.o)
MAIN_S_OBJS = $(MAIN_S_SRCS2:.S=.o)

UBOOT_C_OBJS = $(UBOOT_C_SRCS:.c=.o)
UBOOT_S_OBJS = $(UBOOT_S_SRCS:.S=.o)

# flags
INCLUDE_DIR = -I../include -I$(TOPDIR)/include

CFLAGS = -Os -nostdinc -fsigned-char $(INCLUDE_DIR) -msoft-float -mabi=apcs-gnu -Wall $(DEF_FLAGS)
SFLAGS = -Os -nostdinc $(INCLUDE_DIR) -msoft-float -mabi=apcs-gnu -Wall -D__ASSEMBLY__ $(DEF_FLAGS)

# export
export MINI_BOOT
export CROSS CC LD OC OD NM
export CFLAGS SFLAGS DEBUG_FLAGS
export MAIN_C_SRCS MAIN_S_SRCS MMC_C_SRCS

# make Rules
#all : clean $(MINI_EMMC).bin $(MINI_ESD).bin $(MINI_SD).bin
ifeq ($(CONFIG_EMMC_U_BOOT),y)
all : clean  $(MINI_EMMC).bin
endif
ifeq ($(CONFIG_ESD_U_BOOT),y)
all : clean  $(MINI_ESD).bin
endif
ifeq ($(CONFIG_SD_U_BOOT),y)
all : clean  $(MINI_SD).bin
endif

$(MINI_EMMC).bin : $(MINI_BOOT).bin
	@echo "u-boot start=$(TEXT_BASE) size=$(U_BOOT_SIZE)"
	cat $^ $(TOPDIR)/u-boot.bin > $(TOPDIR)/$(MINI_EMMC).bin
	rm -rf $(MMC_C_OBJS)

$(MINI_ESD).bin : $(MINI_BOOT).bin
	@echo "u-boot start=$(TEXT_BASE) size=$(U_BOOT_SIZE)"
	cat $^ $(TOPDIR)/u-boot.bin > $(TOPDIR)/$(MINI_ESD).bin
	rm -rf $(MMC_C_OBJS)

$(MINI_SD).bin : $(MINI_BOOT).bin
	cp -f $(MINI_BOOT).bin $(TOPDIR)/$(MINI_SD).bin
	rm -rf $(MMC_C_OBJS)

$(MINI_BOOT).bin : $(MINI_BOOT) $(MINI_BOOT).map
	$(OC) -S -O binary --gap-fill=0 --pad-to=0xa0001000 $(MINI_BOOT) $(MINI_BOOT).bin
	$(OD) -D $(MINI_BOOT) > $(MINI_BOOT).dis

$(MINI_BOOT) : $(MAIN_C_OBJS) $(MAIN_S_OBJS) $(MMC_C_OBJS) $(MINI_LD_SCRIPT) $(UBOOT_C_OBJS) $(UBOOT_S_OBJS)
	$(LD) $(MINI_LFLAGS) $(MAIN_S_OBJS) $(MAIN_C_OBJS) $(MMC_C_OBJS) $(UBOOT_C_OBJS)  $(UBOOT_S_OBJS) -o $@

$(MAIN_C_OBJS) $(MAIN_S_OBJS) :
	make -C $(MAIN_DIR)

.c.o :
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -c $<

$(MINI_BOOT).map : $(MINI_BOOT)
	@$(NM) $< | \
	grep -v '\(compiled\)\|\(\.o$$\)\|\( [aUw] \)\|\(\.\.ng$$\)\|\(LASH[RL]DI\)' | \
	sort > $@

$(MINI_LD_SCRIPT) : $(MINI_LD_BASE)
	@sed 's/TEXTADDR/$(MINI_TEXTADDR)/' $^ > $@

$(UBOOT_C_OBJS) : $(UBOOT_C_SRCS2)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) -c $<

$(UBOOT_S_OBJS) : $(UBOOT_S_SRCS2)
	$(CC) $(SFLAGS) $(DEBUG_FLAGS) -c $<

# clean
clean :
	make -C $(MAIN_DIR) clean
	make -C $(UBOOT_DIR) clean
	rm -f *.o *.dis *.map
	rm -f $(MINI_LD_SCRIPT)
	rm -f $(MINI_BOOT)*
