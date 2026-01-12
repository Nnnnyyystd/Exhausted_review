include Makefile.inc

.PHONY : all build deploy run $(DIRS)

PROGRAMS =	cat	\
			cp	\
			ls	\
			mkdir \
			rm	\
			date \
			perf \
			rm

all: build deploy 

.PHONY: qemug
qemug: all
	qemu-system-i386 -m 32M -audiodev pa,id=hda -machine pcspk-audiodev=hda -rtc base=localtime \
	-boot c -drive file=../targets/UNIXV6++/c.img,if=ide,index=0,media=disk,format=raw \
	-s -S

.PHONY: qemu
qemu: all
	qemu-system-i386 -m 32M -audiodev pa,id=hda -machine pcspk-audiodev=hda -rtc base=localtime \
	-boot c -drive file=../targets/UNIXV6++/c.img,if=ide,index=0,media=disk,format=raw 

#no program directory
$(DIRS) : 
	$(MAKE) --directory=$@
	
build	: $(DIRS)	
	$(LD) $(LDFLAGS) $(OBJS) -o $(TARGET)\kernel.exe	
	$(OBJCOPY) -O binary $(TARGET)\kernel.exe $(TARGET)\kernel.bin
	$(NM) $(TARGET)\kernel.exe > $(TARGET)\kernel.sym
	$(OBJDUMP) -d $(TARGET)\kernel.exe  > $(TARGET)\kernel.asm

deploy	:

	copy $(TARGET)\boot.bin $(IMGTARGET)\boot.bin
	copy $(TARGET)\kernel.bin $(IMGTARGET)\kernel.bin
	copy $(TARGET)\kernel.sym  $(IMGTARGET)\kernel.sym
	copy $(TARGET)\kernel.asm  $(IMGTARGET)\kernel.asm

#	cd $(IMGTARGET) && partcopy boot.bin 0 200 c.img 0
#	cd $(IMGTARGET) && partcopy kernel.bin 0 13000 c.img 200

#	copy $(TARGET)\boot.bin $(MakeImageDebug)\boot.bin
#	copy $(TARGET)\kernel.bin $(MakeImageDebug)\kernel.bin
#	copy $(IMGTARGET)\c.img $(MakeImageDebug)\c.img

#	cd $(MakeImageDebug) && build.exe c.img boot.bin kernel.bin programs
#	copy $(MakeImageDebug)\$(IMG) $(UNIXV6PPTARGET)\$(IMG)

	copy $(TARGET)\boot.bin $(V6PP_FS_EDIT_WORKSPACE)\boot.bin
	copy $(TARGET)\kernel.bin $(V6PP_FS_EDIT_WORKSPACE)\kernel.bin

	cd $(V6PP_FS_EDIT_WORKSPACE) && filescanner.exe | fsedit.exe $(IMG) c
	copy $(V6PP_FS_EDIT_WORKSPACE)\c.img $(UNIXV6PPTARGET)\$(IMG)

clean	:	
	del     $(TARGET)\*.o
	del     $(TARGET)\*.exe
	del     $(TARGET)\*.bin
	del     $(TARGET)\*.sym
	del     $(TARGET)\*.asm
	
	del     $(IMGTARGET)\*.bin
	del     $(IMGTARGET)\*.sym
	del     $(IMGTARGET)\*.asm
	
	del     $(UNIXV6PPTARGET)\c.img
