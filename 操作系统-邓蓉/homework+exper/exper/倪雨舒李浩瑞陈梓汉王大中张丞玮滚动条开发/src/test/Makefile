include ../Makefile.inc

#SRC/test下各个模块的测试代码子目录定义
TESTDIR = dev fs lib mm

#专门用于存放OBJS的目录
TARGET = ..\..\targets\objs
#SRC/test子目录中同时包括*.h和*.cpp文件，不再将*.h单独放到test/include目录下
INCLUDE = .

.PHONY	:	all $(TESTDIR)

all	:	$(TESTDIR) $(TARGET)\testutility.o

#遍历SRC/test/xx子目录，由这些xx子目录将生成的OBJS文件存放到target\objs
$(TESTDIR)	:
	$(MAKE) --directory=$@

$(TARGET)\testutility.o	:	TestUtility.cpp $(INCLUDE)\TestUtility.h
	$(CC) $(CFLAGS) -I"$(INCLUDE)" -c $< -o $@
