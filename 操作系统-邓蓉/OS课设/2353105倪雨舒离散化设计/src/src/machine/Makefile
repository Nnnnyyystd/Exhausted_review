include ..\Makefile.inc

TARGET = ..\..\targets\objs

all		:	$(TARGET)\chip8253.o $(TARGET)\chip8259A.o $(TARGET)\idt.o $(TARGET)\gdt.o \
			$(TARGET)\machine.o	$(TARGET)\pagedirectory.o $(TARGET)\taskstatesegment.o \
			$(TARGET)\cmostime.o
		
$(TARGET)\chip8253.o :	Chip8253.cpp $(INCLUDE)\Chip8253.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $< -o $@

$(TARGET)\chip8259A.o : Chip8259A.cpp $(INCLUDE)\Chip8259A.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $< -o $@
	
$(TARGET)\idt.o	:	IDT.cpp $(INCLUDE)\IDT.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $< -o $@
	
$(TARGET)\gdt.o :	GDT.cpp $(INCLUDE)\GDT.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $< -o $@
	
$(TARGET)\machine.o : Machine.cpp $(INCLUDE)\Machine.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $< -o $@	

$(TARGET)\pagedirectory.o : PageDirectory.cpp $(INCLUDE)\PageDirectory.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $< -o $@

$(TARGET)\taskstatesegment.o : TaskStateSegment.cpp $(INCLUDE)\TaskStateSegment.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $< -o $@

$(TARGET)\cmostime.o : CMOSTime.cpp $(INCLUDE)\CMOSTime.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $< -o $@
