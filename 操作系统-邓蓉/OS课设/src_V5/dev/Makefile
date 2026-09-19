include ..\Makefile.inc

TARGET = ..\..\targets\objs

all		:	$(TARGET)\buffermanager.o $(TARGET)\blockdevice.o $(TARGET)\devicemanager.o \
			$(TARGET)\atadriver.o $(TARGET)\dma.o $(TARGET)\chardevice.o
			
$(TARGET)\buffermanager.o	:	BufferManager.cpp $(INCLUDE)\BufferManager.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $< -o $@
	
$(TARGET)\blockdevice.o	:	BlockDevice.cpp $(INCLUDE)\BlockDevice.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $< -o $@
	
$(TARGET)\devicemanager.o	:	DeviceManager.cpp $(INCLUDE)\DeviceManager.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $< -o $@
	
$(TARGET)\atadriver.o	:	ATADriver.cpp $(INCLUDE)\ATADriver.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $< -o $@
	
$(TARGET)\dma.o	:	DMA.cpp $(INCLUDE)\DMA.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $< -o $@

$(TARGET)\chardevice.o	:	CharDevice.cpp $(INCLUDE)\CharDevice.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $< -o $@	