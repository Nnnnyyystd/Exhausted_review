include ..\..\Makefile.inc

#专门用于存放OBJS的目录
TARGET = ..\..\..\targets\objs
#SRC/test/xx子目录中同时包括*.h和*.cpp文件，不再将*.h单独放到test/include目录下
INCLUDE = .

all		:	$(TARGET)\testswappermanager.o

$(TARGET)\testswappermanager.o	:	TestSwapperManager.cpp $(INCLUDE)\TestSwapperManager.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $< -o $@
