#ifndef TTY_H
#define TTY_H

/* 字符缓冲队列 */
class TTy_Queue
{
public:
	/*
	 * TTY_BUF_SIZE取值必须为2的n次幂，这样才可保证
	 * TTY_BUF_SIZE - 1二进制比特全部为1，否则CharNum()
	 * 函数中的&运算结果不正确。
	 */
	static const unsigned int TTY_BUF_SIZE = 512;

	/* Functions */
public:
	/* Constructors */
	TTy_Queue();
	/* Destructors */
	~TTy_Queue();

	/* 从字符缓存中取出字符 */
	char GetChar();

	/* 将输入字符放到字符缓存中 */
	void PutChar(char ch);

	/* 缓存中未取出的字符数 */
	int CharNum();

	/* 返回缓存中即将取出字符的地址 */
	char *CurrentChar();

public:
	unsigned int m_Head;		  /* 指向字符缓存数组中下一个用于存放接收字符的位置 */
	unsigned int m_Tail;		  /* 指向字符缓存数组中下一个要取出字符的位置 */
	char m_CharBuf[TTY_BUF_SIZE]; /* 字符缓存数组 */
};

class TTy
{
	/* Static Members */
public:
	static const unsigned int CANBSIZ = 256;

	/* 字符缓存队列字符数限制 */
	static const int TTHIWAT = 512;
	static const int TTLOWAT = 30;
	static const int TTYHOG = 256;

	static const char CERASE = '\b'; /* 定义擦除键 */
	static const char CEOT = 0x04;	 /* 文件结束符 */
	static const char CKILL = 0x15;
	static const char CINTR = 0x7f;
	static const char GET_ERROR = -1;

	/* modes (t_flags设置) */
	static const int HUPCL = 0x1;
	static const int XTABS = 0x2;
	static const int LCASE = 0x4;
	static const int ECHO = 0x8;
	static const int CRMOD = 0x10;
	static const int RAW = 0x20;

	/* Internal state bits (t_state设置) */
	static const int ISOPEN = 0x1;
	static const int CARR_ON = 0x2;

	/* Functions */
public:
	/* Constructors */
	TTy();
	/* Destructors */
	~TTy();

	/* tty设备的通用读函数，由各个字符设备的读写函数调用 */
	void TTRead();

	/* tty设备的通用写函数，由各个字符设备的读写函数调用 */
	void TTWrite();

	/* 输入字符初步处理程序 */
	void TTyInput(char ch);

	/* 输出字符处理程序 */
	void TTyOutput(char ch);

	/* tty设备的启动函数，不过在这个操作系统中，只是为了保证移植结构的完整性 */
	void TTStart();

	/* 清空TTY所有缓存内容 */
	void FlushTTy();

	/* 行规则程序，对输入的字符进行处理，比如删除行或者backspace */
	int Canon();

	int PassC(char ch);

	char CPass();

public:
	TTy_Queue t_rawq; /* 原始输入字符缓存队列 */
	TTy_Queue t_canq; /* 标准输入字符缓存队列 */
	TTy_Queue t_outq; /* 输出字符缓存队列 */

	int t_flags; /* 字符设备工作标志字 */
	int t_delct; /* 原始输入字符队列中的定界符数 */

	char t_erase; /* 擦除键字符 */
	char t_kill;  /* 删除整行字符 */

	int t_state; /* 设备状态字 */
	short dev;	 /* 设备号 */

	char Canonb[CANBSIZ]; /* 对输入字符处理的工作缓存 */
};

#endif
