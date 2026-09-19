include ..\Makefile.inc

TARGET = ..\..\targets\objs

all		:	$(TARGET)\filesystem.o $(TARGET)\openfilemanager.o $(TARGET)\inode.o \
			$(TARGET)\file.o $(TARGET)\filemanager.o
			
$(TARGET)\filesystem.o	:	FileSystem.cpp $(INCLUDE)\FileSystem.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $< -o $@
	
$(TARGET)\openfilemanager.o	:	OpenFileManager.cpp $(INCLUDE)\OpenFileManager.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $< -o $@
	
$(TARGET)\inode.o	:	INode.cpp $(INCLUDE)\INode.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $< -o $@
	
$(TARGET)\file.o	:	File.cpp $(INCLUDE)\File.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $< -o $@

$(TARGET)\filemanager.o	:	FileManager.cpp $(INCLUDE)\FileManager.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $< -o $@	
	