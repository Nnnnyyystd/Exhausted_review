include ..\Makefile.inc

TARGET = ..\..\targets\objs

all		:	$(TARGET)\peparser.o
		
$(TARGET)\PEParser.o	:	PEParser.cpp $(INCLUDE)\PEParser.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $< -o $@

