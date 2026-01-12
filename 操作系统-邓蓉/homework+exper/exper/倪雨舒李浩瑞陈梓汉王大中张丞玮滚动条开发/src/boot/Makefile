include ..\Makefile.inc

TARGET = ..\..\targets\objs

all		:	$(TARGET)\boot.bin $(TARGET)\sector2.bin $(TARGET)\support.o
		
$(TARGET)\boot.bin	:	boot.s
	$(ASM) -f bin boot.s -o $(TARGET)\boot.bin

$(TARGET)\sector2.bin : sector2.s
	$(ASM) -f elf sector2.s -o $(TARGET)\sector2.bin	
	
$(TARGET)\support.o : support.c
	$(CC) $(CFLAGS) -c $< -o $@