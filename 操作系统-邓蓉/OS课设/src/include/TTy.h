#ifndef TTY_H
#define TTY_H

/* �ַ�������� */
class TTy_Queue
{
public:
	/* 
	 * TTY_BUF_SIZEȡֵ����Ϊ2��n���ݣ������ſɱ�֤
	 * TTY_BUF_SIZE - 1�����Ʊ���ȫ��Ϊ1������CharNum()
	 * �����е�&����������ȷ��
	 */
	static const unsigned int TTY_BUF_SIZE = 512;

	/* Functions */
public:
	/* Constructors */
	TTy_Queue();
	/* Destructors */
	~TTy_Queue();

	/* ���ַ�������ȡ���ַ� */
	char GetChar();

	/* �������ַ��ŵ��ַ������� */
	void PutChar(char ch);

	/* ������δȡ�����ַ��� */
	int CharNum();

	/* ���ػ����м���ȡ���ַ��ĵ�ַ */
	char* CurrentChar();

public:
	unsigned int m_Head;	/* ָ���ַ�������������һ�����ڴ�Ž����ַ���λ�� */
	unsigned int m_Tail;	/* ָ���ַ�������������һ��Ҫȡ���ַ���λ�� */
	char m_CharBuf[TTY_BUF_SIZE];	/* �ַ��������� */
};


class TTy
{
	/* Static Members */
public:
	static const unsigned int CANBSIZ = 256;

	/* �ַ���������ַ������� */
	static const int TTHIWAT = 512;
	static const int TTLOWAT = 30;
	static const int TTYHOG = 256;

	static const char CERASE = '\b';	/* ��������� */
	static const char CEOT = 0x04;		/* �ļ������� */
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

	/* modes (t_flags����) */
	static const int HUPCL = 0x1;
	static const int XTABS = 0x2;
	static const int LCASE = 0x4;
	static const int ECHO = 0x8;
	static const int CRMOD = 0x10;
	static const int RAW =  0x20;

	/* Internal state bits (t_state����) */
	static const int ISOPEN = 0x1;
	static const int CARR_ON = 0x2;


	/* Functions */
public:
	/* Constructors */
	TTy();
	/* Destructors */
	~TTy();

	/* tty�豸��ͨ�ö��������ɸ����ַ��豸�Ķ�д�������� */
	void TTRead();

	/* tty�豸��ͨ��д�������ɸ����ַ��豸�Ķ�д�������� */
	void TTWrite();

	/* �����ַ������������� */
	void TTyInput(char ch);

	/* ����ַ��������� */
	void TTyOutput(char ch);

	/* tty�豸�������������������������ϵͳ�У�ֻ��Ϊ�˱�֤��ֲ�ṹ�������� */
	void TTStart();

	/* ���TTY���л������� */
	void FlushTTy();


	/* �й�����򣬶�������ַ����д���������ɾ���л���backspace */
	int Canon();

	int PassC(char ch);

	char CPass();

public:
	TTy_Queue t_rawq;	/* ԭʼ�����ַ�������� */
	TTy_Queue t_canq;	/* ��׼�����ַ�������� */
	TTy_Queue t_outq;	/* ����ַ�������� */

	int t_flags;	/* �ַ��豸������־�� */
	int t_delct;	/* ԭʼ�����ַ������еĶ������ */

	char t_erase;	/* �������ַ� */
	char t_kill;	/* ɾ�������ַ� */

	int t_state;	/* �豸״̬�� */
	short dev;		/* �豸�� */


	char Canonb[CANBSIZ];	/* �������ַ������Ĺ������� */

	/* Command History */
	char m_History[HISTORY_SIZE][CANBSIZ];
	int m_HistoryHead;      /* ��һ��д��λ�� */
	int m_HistoryCount;     /* ��ǰ��ʷ��¼���� */
	int m_HistoryViewIndex; /* ��ǰ�鿴����ʷ��¼���� */
	char m_DraftBuffer[CANBSIZ]; /* �ݴ浱ǰ���������δ�ύ���� */
};

#endif
