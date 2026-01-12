#ifndef TTY_H
#define TTY_H

/* 字符缓冲队列 */
class TTy_Queue
{
public:
	/* 
	 * TTY_BUF_SIZE取值必须为2的n次幂，这样才可保证
	 * TTY_BUF_SIZE - 1的二进制表示全为1，从而CharNum()
	 * 函数中的&运算结果正确。
	 */
	static const unsigned int TTY_BUF_SIZE = 512;

	/* Functions */
public:
	/* Constructors */
	TTy_Queue();
	/* Destructors */
	~TTy_Queue();

	/* 从字符缓冲队列取出字符 */
	char GetChar();

	/* 将一个字符放到字符缓冲队列 */
	void PutChar(char ch);

	/* 返回未取出的字符数 */
	int CharNum();

	/* 返回缓冲中即将取出的字符的地址 */
	char* CurrentChar();

public:
	unsigned int m_Head;	/* 指向字符缓冲队列中下一个可以存放进字符的位置 */
	unsigned int m_Tail;	/* 指向字符缓冲队列中下一个要取出的字符的位置 */
	char m_CharBuf[TTY_BUF_SIZE];	/* 字符缓冲数组 */
};


class TTy
{
	/* Static Members */
public:
	static const unsigned int CANBSIZ = 256;

	/* 字符缓冲队列水位线常量 */
	static const int TTHIWAT = 512;
	static const int TTLOWAT = 30;
	static const int TTYHOG = 256;

	static const char CERASE = '\b';	/* 退格键 */
	static const char CEOT = 0x04;		/* 文件结束符 */
	static const char CKILL = 0x15;
	static const char CINTR = 0x7f;
	static const char CDEL = 0x7f;      /* Delete键 */
	static const char GET_ERROR = -1;
	static const char KEY_LEFT = 0x11;
	static const char KEY_RIGHT = 0x12;
	static const char KEY_UP = 0x13;
	static const char KEY_DOWN = 0x14;

	/* History */
	static const int HISTORY_SIZE = 10;

	/* modes (t_flags定义) */
	static const int HUPCL = 0x1;
	static const int XTABS = 0x2;
	static const int LCASE = 0x4;
	static const int ECHO = 0x8;
	static const int CRMOD = 0x10;
	static const int RAW =  0x20;

	/* Internal state bits (t_state定义) */
	static const int ISOPEN = 0x1;
	static const int CARR_ON = 0x2;


	/* Functions */
public:
	/* Constructors */
	TTy();
	/* Destructors */
	~TTy();

	/* tty设备通用读函数，由各字符设备的读写函数调用 */
	void TTRead();

	/* tty设备通用写函数，由各字符设备的读写函数调用 */
	void TTWrite();

	/* 将字符放入原始输入队列 */
	void TTyInput(char ch);

	/* 将字符放入输出队列 */
	void TTyOutput(char ch);

	/* tty设备开始输出，在我们的系统中，只是为了保证与Unix结构一致而保留 */
	void TTStart();

	/* 清空TTY所有缓冲区 */
	void FlushTTy();


	/* 行规程处理，对原始输入字符进行处理，如删除行或backspace */
	int Canon();

	int PassC(char ch);

	char CPass();

public:
	TTy_Queue t_rawq;	/* 原始输入字符缓冲队列 */
	TTy_Queue t_canq;	/* 标准输入字符缓冲队列 */
	TTy_Queue t_outq;	/* 输出字符缓冲队列 */

	int t_flags;	/* 字符设备模式标志位 */
	int t_delct;	/* 原始输入字符队列中的定界符数 */

	char t_erase;	/* 退格字符 */
	char t_kill;	/* 删除行字符 */

	int t_state;	/* 设备状态位 */
	short dev;		/* 设备号 */


	char Canonb[CANBSIZ];	/* 规范输入字符处理的工作缓冲区 */

	/* Command History */
	char m_History[HISTORY_SIZE][CANBSIZ];
	int m_HistoryHead;      /* 下一个写入位置 */
	int m_HistoryCount;     /* 当前历史记录数量 */
	int m_HistoryViewIndex; /* 当前查看的历史记录索引 */
	char m_DraftBuffer[CANBSIZ]; /* 暂存当前正在输入但未提交的行 */
};

#endif
