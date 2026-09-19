include ..\Makefile.inc

TARGET = ..\..\targets\objs

all		:	$(TARGET)\process.o $(TARGET)\processmanager.o $(TARGET)\memorydescriptor.o $(TARGET)\user.o $(TARGET)\text.o			
			
$(TARGET)\process.o	:	Process.cpp $(INCLUDE)\Process.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $< -o $@

$(TARGET)\processmanager.o	:	ProcessManager.cpp $(INCLUDE)\ProcessManager.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $< -o $@

$(TARGET)\memorydescriptor.o	:	MemoryDescriptor.cpp $(INCLUDE)\MemoryDescriptor.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $< -o $@

$(TARGET)\user.o	:	User.cpp $(INCLUDE)\User.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $< -o $@
	
$(TARGET)\text.o	:	Text.cpp $(INCLUDE)\Text.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $< -o $@
