# this is main makefile
.PHONY:all
.PHONY:clean

SOC_NAME := f1c100s
ARCH := arm32

include common.mk

APPDIR := app
ARCHDIR := arch/$(ARCH)
BOOTDIR := boot/$(SOC_NAME)
BUILDDIR := build
CLIBDIR := clib
DRIVERDIR := driver/$(SOC_NAME)
TOOLSDIR := tools/$(SOC_NAME)
USBPHYDIR := usb_phy/$(SOC_NAME)
USBCLASSSDIR := usb_class
LDSDIR :=lds

SRC_C :=  
SRC_S := 
SRC_CPP := 

LIBS 		:= -lgcc 
MCFLAGS := -march=armv5te -mtune=arm926ej-s -mfloat-abi=soft -marm -mno-thumb-interwork
INCDIRS := include
MKBIN :=   $(ECHO)
#every directory has *.mk add source file to SRC_C , SRC_S ,SRC_CPP , and add value to 

include $(APPDIR)/app.mk
include $(ARCHDIR)/arch.mk
include $(BOOTDIR)/boot.mk
include $(CLIBDIR)/clib.mk
include $(DRIVERDIR)/driver.mk
include $(USBPHYDIR)/usb_phy.mk
include $(USBCLASSSDIR)/usb_class.mk
include $(TOOLSDIR)/tools.mk
INC_FLAGS := $(addprefix -I,$(INCDIRS))

OBJ = $(addprefix $(BUILDDIR)/, $(SRC_C:.c=.o)) $(addprefix $(BUILDDIR)/, $(SRC_S:.S=.o))

all:$(BUILDDIR)/easyusb.bin



LDFLAGS		:= -T $(LDSDIR)/$(SOC_NAME).ld -nostdlib

$(BUILDDIR)/easyusb.bin: $(BUILDDIR)/easyusb.elf
	$(OBJCOPY) -v -O binary $^ $@
	@echo Make header information for brom booting
	@$(MKBIN) $@

$(BUILDDIR)/easyusb.elf: $(OBJ)
	$(ECHO) "LINK $@"
	$(CC) $(LDFLAGS) -Wl,--cref,-Map=$@.map -o $@ $^ $(LIBS)
	$(SIZE) $@


$(BUILDDIR)/%.o: %.S
	$(ECHO) "AS $<"
	$(AS) $(MCFLAGS) $(ASFLAGS) -c -o $@ $<
	
$(BUILDDIR)/%.o: %.s
	$(ECHO) "AS $<"
	$(AS) $(MCFLAGS) $(CFLAGS) -o $@ $<

define compile_c
$(ECHO) "CC $<"
$(CC) $(INC_FLAGS) $(MCFLAGS) $(CFLAGS) -c -MD -o $@ $<
@# The following fixes the dependency file.
@# See http://make.paulandlesley.org/autodep.html for details.
@# Regex adjusted from the above to play better with Windows paths, etc.
@$(CP) $(@:.o=.d) $(@:.o=.P); \
  $(SED) -e 's/#.*//' -e 's/^.*:  *//' -e 's/ *\\$$//' \
      -e '/^$$/ d' -e 's/$$/ :/' < $(@:.o=.d) >> $(@:.o=.P); \
  $(RM) -f $(@:.o=.d)
endef

$(BUILDDIR)/%.o: %.c
	$(call compile_c)

OBJ_DIRS = $(sort $(dir $(OBJ)))
$(OBJ): | $(OBJ_DIRS)
$(OBJ_DIRS):
	$(MKDIR) -p $@

clean:
	find . -name "*.o"  | xargs rm -f
	find . -name "*.bin"  | xargs rm -f
	find . -name "*.elf"  | xargs rm -f
	find . -name "*.P"  | xargs rm -f
#rm -rf *.o *.bin 
#rm -rf $(BUILD)/*.o $(BUILD)/*.bin

write:
	sudo sunxi-fel -p spiflash-write 0 $(BUILDDIR)/easyusb.bin

mktool:
	cd tools/mksunxiboot && make
	cd tools/mksunxi && make

dump:
	$(OBJDUMP) -S myboot.o | less


