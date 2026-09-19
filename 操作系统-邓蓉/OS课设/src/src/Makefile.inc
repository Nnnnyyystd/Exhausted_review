#命令定义
CC = g++
LD = ld
OBJCOPY = objcopy
OBJDUMP = objdump
ASM = nasm
NM = nm
MAKE = make

#参数定义
#g++参数
CFLAGS = -Wall -O0 -g -nostartfiles -nostdlib -fno-builtin -fno-rtti -fno-exceptions -nostdinc
#ld参数
LDFLAGS = -T $(LINKSCRIPT)

#目录定义
OBJFILES = ..\objs
TARGET = ..\targets\objs
DIRS = boot kernel machine interrupt mm proc dev fs tty test pe lib shell program
INCLUDE = ..\include
IMGTARGET = ..\targets\img
#"++"号是特殊符号，因此需要加引号
UNIXV6PPTARGET = "..\targets\UNIXV6++"
MakeImageDebug = ..\tools\MakeImage\bin\Debug
V6PP_FS_EDIT_WORKSPACE := ..\tools\v6pp-fs-edit-2022\workspace

#文件定义
LINKSCRIPT = Link.ld
SECTOR2_OBJ = sector2.bin
CPP_OBJS = $(foreach OBJFILE, \
	$(shell dir $(TARGET)\*.o /B), \
	$(TARGET)\$(OBJFILE) \
	)
OBJS = $(TARGET)\$(SECTOR2_OBJ) $(CPP_OBJS)
IMG = c.img

#编译规则
.c.s:
	$(CC) $(CFLAGS) \
	-S -o $*.s $<
.s.o:
	$(AS) -o $*.o $<
.c.o:
	$(CC) $(CFLAGS) \
	-c -o $*.o $<