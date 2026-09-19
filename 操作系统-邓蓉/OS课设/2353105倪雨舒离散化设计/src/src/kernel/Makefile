include ..\Makefile.inc

TARGET = ..\..\targets\objs

all		:	$(TARGET)\main.o $(TARGET)\kernel.o $(TARGET)\video.o $(TARGET)\utility.o

$(TARGET)\main.o	:	main.cpp
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $< -o $@
	
$(TARGET)\kernel.o	:	Kernel.cpp $(INCLUDE)\Kernel.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $< -o $@
			
$(TARGET)\video.o : Video.cpp $(INCLUDE)\Video.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $< -o $@

$(TARGET)\utility.o : Utility.cpp $(INCLUDE)\Utility.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $< -o $@