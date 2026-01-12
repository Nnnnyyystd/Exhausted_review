#
# Makefile for all Unix V6++ programs
#

.DEFAULT_GOAL := all

# 编译器
CC = gcc
LD = ld
OBJDUMP = objdump

# 编译参数
CFLAGS = -w -O0 -g -nostdlib -nostartfiles -nostdinc -fno-builtin -fno-exceptions

# 目录
TARGET = .\objs
V6++LIB = "..\lib\Lib_V6++.a"
INCLUDE = .
LIB_INCLUDE = ..\lib\include

MAKEIMAGEPATH = ..\..\tools\v6pp-fs-edit-2022\workspace\programs
BIN = bin

.PHONY : all

# Shell 程序列表（新增 kill.exe 和 loop.exe）
SHELL_OBJS =$(TARGET)\cat.exe	\
			$(TARGET)\cp.exe	\
			$(TARGET)\ls.exe	\
			$(TARGET)\mkdir.exe	\
			$(TARGET)\rm.exe	\
			$(TARGET)\perf.exe	\
			$(TARGET)\sig.exe \
			$(TARGET)\copyfile.exe	\
			$(TARGET)\shutdown.exe	\
			$(TARGET)\test.exe	\
			$(TARGET)\forks.exe	\
			$(TARGET)\trace.exe	\
			$(TARGET)\echo.exe	\
			$(TARGET)\date.exe \
			$(TARGET)\newsig.exe \
			$(TARGET)\sigTest.exe \
			$(TARGET)\stack.exe	\
			$(TARGET)\malloc.exe \
			$(TARGET)\testSTDOUT.exe \
			$(TARGET)\fork.exe \
			$(TARGET)\showStack.exe \
			$(TARGET)\trivialProg.exe \
			$(TARGET)\helloworld.exe \
			$(TARGET)\getppid.exe \
			$(TARGET)\getproc.exe \
			$(TARGET)\getpids.exe \
			$(TARGET)\tryExec.exe \
			$(TARGET)\myShell.exe \
			$(TARGET)\kill.exe \
			$(TARGET)\loop.exe \
			$(TARGET)\diagspam.exe \
		
		


build : $(SHELL_OBJS)

#------------ 原有程序编译规则（保持不动） ---------------#

$(TARGET)\cat.exe :	cat.c
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -I"$(LIB_INCLUDE)"  $< -e _main1 $(V6++LIB) -o $@
	copy $(TARGET)\cat.exe $(MAKEIMAGEPATH)\$(BIN)\cat

$(TARGET)\cp.exe :	cp.c
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -I"$(LIB_INCLUDE)"  $< -e _main1 $(V6++LIB) -o $@
	copy $(TARGET)\cp.exe $(MAKEIMAGEPATH)\$(BIN)\cp

$(TARGET)\ls.exe :	ls.c
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -I"$(LIB_INCLUDE)"  $< -e _main1 $(V6++LIB) -o $@
	copy $(TARGET)\ls.exe $(MAKEIMAGEPATH)\$(BIN)\ls

$(TARGET)\mkdir.exe :	mkdir.c
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -I"$(LIB_INCLUDE)"  $< -e _main1 $(V6++LIB) -o $@
	copy $(TARGET)\mkdir.exe $(MAKEIMAGEPATH)\$(BIN)\mkdir

$(TARGET)\perf.exe :	performance.c
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -I"$(LIB_INCLUDE)"  $< -e _main1 $(V6++LIB) -o $@
	copy $(TARGET)\perf.exe $(MAKEIMAGEPATH)\$(BIN)\perf

$(TARGET)\rm.exe :	rm.c
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -I"$(LIB_INCLUDE)"  $< -e _main1 $(V6++LIB) -o $@
	copy $(TARGET)\rm.exe $(MAKEIMAGEPATH)\$(BIN)\rm

$(TARGET)\copyfile.exe :	copyfile.c
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -I"$(LIB_INCLUDE)"  $< -e _main1 $(V6++LIB) -o $@
	copy $(TARGET)\copyfile.exe $(MAKEIMAGEPATH)\$(BIN)\copyfile

$(TARGET)\shutdown.exe :	shutdown.c
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -I"$(LIB_INCLUDE)"  $< -e _main1 $(V6++LIB) -o $@
	copy $(TARGET)\shutdown.exe $(MAKEIMAGEPATH)\$(BIN)\shutdown

$(TARGET)\test.exe :	test.c
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -I"$(LIB_INCLUDE)"  $< -e _main1 $(V6++LIB) -o $@
	copy $(TARGET)\test.exe $(MAKEIMAGEPATH)\$(BIN)\test

$(TARGET)\forks.exe :	forks.c
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -I"$(LIB_INCLUDE)"  $< -e _main1 $(V6++LIB) -o $@
	copy $(TARGET)\forks.exe $(MAKEIMAGEPATH)\$(BIN)\forks

$(TARGET)\sig.exe :	sig.c
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -I"$(LIB_INCLUDE)"  $< -e _main1 $(V6++LIB) -o $@
	copy $(TARGET)\sig.exe $(MAKEIMAGEPATH)\$(BIN)\sig

$(TARGET)\sigTest.exe :	sigTest.c
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -I"$(LIB_INCLUDE)"  $< -e _main1 $(V6++LIB) -o $@
	copy $(TARGET)\sigTest.exe $(MAKEIMAGEPATH)\$(BIN)\sigTest

$(TARGET)\trace.exe :	trace.c
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -I"$(LIB_INCLUDE)"  $< -e _main1 $(V6++LIB) -o $@
	copy $(TARGET)\trace.exe $(MAKEIMAGEPATH)\$(BIN)\trace

$(TARGET)\echo.exe :	echo.c
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -I"$(LIB_INCLUDE)"  $< -e _main1 $(V6++LIB) -o $@
	copy $(TARGET)\echo.exe $(MAKEIMAGEPATH)\$(BIN)\echo

$(TARGET)\date.exe :	date.c
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -I"$(LIB_INCLUDE)"  $< -e _main1 $(V6++LIB) -o $@
	copy $(TARGET)\date.exe $(MAKEIMAGEPATH)\$(BIN)\date

$(TARGET)\newsig.exe :	newsig.c
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -I"$(LIB_INCLUDE)"  $< -e _main1 $(V6++LIB) -o $@
	copy $(TARGET)\newsig.exe $(MAKEIMAGEPATH)\$(BIN)\newsig

$(TARGET)\stack.exe :	stack.c
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -I"$(LIB_INCLUDE)"  $< -e _main1 $(V6++LIB) -o $@
	copy $(TARGET)\stack.exe $(MAKEIMAGEPATH)\$(BIN)\stack

$(TARGET)\malloc.exe :	malloc.c
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -I"$(LIB_INCLUDE)"  $< -e _main1 $(V6++LIB) -o $@
	copy $(TARGET)\malloc.exe $(MAKEIMAGEPATH)\$(BIN)\malloc

$(TARGET)\fork.exe :	fork.c
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -I"$(LIB_INCLUDE)"  $< -e _main1 $(V6++LIB) -o $@
	copy $(TARGET)\fork.exe $(MAKEIMAGEPATH)\$(BIN)\fork

$(TARGET)\testSTDOUT.exe :	testSTDOUT.c
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -I"$(LIB_INCLUDE)"  $< -e _main1 $(V6++LIB) -o $@
	copy $(TARGET)\testSTDOUT.exe $(MAKEIMAGEPATH)\$(BIN)\testSTDOUT

$(TARGET)\showStack.exe :	showStack.c
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -I"$(LIB_INCLUDE)"  $< -e _main1 $(V6++LIB) -o $@
	copy $(TARGET)\showStack.exe $(MAKEIMAGEPATH)\$(BIN)\showStack

$(TARGET)\trivialProg.exe :	trivialProg.c
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -I"$(LIB_INCLUDE)"  $< -e _main1 $(V6++LIB) -o $@
	copy $(TARGET)\trivialProg.exe $(MAKEIMAGEPATH)\$(BIN)\trivialProg

$(TARGET)\helloworld.exe :	helloworld.c
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -I"$(LIB_INCLUDE)"  $< -e _main1 $(V6++LIB) -o $@
	copy $(TARGET)\helloworld.exe $(MAKEIMAGEPATH)\$(BIN)\helloworld

$(TARGET)\getppid.exe : getppid.c
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -I"$(LIB_INCLUDE)" $< -e _main1 $(V6++LIB) -o $@
	if not exist $(MAKEIMAGEPATH)\$(BIN)\ mkdir $(MAKEIMAGEPATH)\$(BIN)
	copy $@ $(MAKEIMAGEPATH)\$(BIN)\getppid

$(TARGET)\getproc.exe : getproc.c
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -I"$(LIB_INCLUDE)" $< -e _main1 "$(V6++LIB)" -o $@
	if not exist $(MAKEIMAGEPATH)\$(BIN)\ mkdir $(MAKEIMAGEPATH)\$(BIN)
	copy $@ $(MAKEIMAGEPATH)\$(BIN)\getproc

$(TARGET)\getpids.exe : getpids.c
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -I"$(LIB_INCLUDE)" $< -e _main1 $(V6++LIB) -o $@
	if not exist $(MAKEIMAGEPATH)\$(BIN)\ mkdir $(MAKEIMAGEPATH)\$(BIN)
	copy $@ $(MAKEIMAGEPATH)\$(BIN)\getpids

$(TARGET)\tryExec.exe : tryExec.c
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -I"$(LIB_INCLUDE)" $< -e _main1 $(V6++LIB) -o $@
	if not exist $(MAKEIMAGEPATH)\$(BIN)\ mkdir $(MAKEIMAGEPATH)\$(BIN)
	copy $@ $(MAKEIMAGEPATH)\$(BIN)\tryExec

$(TARGET)\myShell.exe : myShell.c
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -I"$(LIB_INCLUDE)" $< -e _main1 $(V6++LIB) -o $@
	if not exist $(MAKEIMAGEPATH)\$(BIN)\ mkdir $(MAKEIMAGEPATH)\$(BIN)
	copy $@ $(MAKEIMAGEPATH)\$(BIN)\myShell


#==================== 新增：kill.exe ====================#

$(TARGET)\kill.exe : kill.c
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -I"$(LIB_INCLUDE)" kill.c -e _main1 $(V6++LIB) -o $@
	if not exist $(MAKEIMAGEPATH)\$(BIN)\ mkdir $(MAKEIMAGEPATH)\$(BIN)
	copy $@ $(MAKEIMAGEPATH)\$(BIN)\kill

#==================== 新增：loop.exe ====================#

$(TARGET)\loop.exe : loop.c
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -I"$(LIB_INCLUDE)" loop.c -e _main1 $(V6++LIB) -o $@
	if not exist $(MAKEIMAGEPATH)\$(BIN)\ mkdir $(MAKEIMAGEPATH)\$(BIN)
	copy $@ $(MAKEIMAGEPATH)\$(BIN)\loop

$(TARGET)\diagspam.exe : diagspam.c
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -I"$(LIB_INCLUDE)" diagspam.c -e _main1 $(V6++LIB) -o $@
	if not exist $(MAKEIMAGEPATH)\$(BIN)\ mkdir $(MAKEIMAGEPATH)\$(BIN)
	copy $@ $(MAKEIMAGEPATH)\$(BIN)\diagspam


#------------ 清理 ---------------#

clean:
	del $(TARGET)\*.exe
	del /Q $(MAKEIMAGEPATH)\$(BIN)\*

all : clean build
