#include "TTy.h"
#include "Assembly.h"
#include "Kernel.h"
#include "CRT.h"

/*==============================class TTy_Queue===============================*/
TTy_Queue::TTy_Queue()
{
	this->m_Head = 0;
	this->m_Tail = 0;
}

TTy_Queue::~TTy_Queue()
{
	//nothing to do here
}

char TTy_Queue::GetChar()
{
	char ch = TTy::GET_ERROR;
	{
		if ( this->m_Head == this->m_Tail )
		{
			//Buffer Empty
			return ch;
		}
	}

	ch = this->m_CharBuf[m_Tail];
	this->m_Tail = ( this->m_Tail + 1 ) % TTY_BUF_SIZE;

	return ch;
}

void TTy_Queue::PutChar(char ch)
{
	this->m_CharBuf[m_Head] = ch;
	this->m_Head = ( this->m_Head + 1 ) % TTY_BUF_SIZE;
}

int TTy_Queue::CharNum()
{
	/* ��Head < Tailʱʹ��%���������⣡������'&'���㡣
	 *  Ʃ���±�Head = 5��Tail = 10�� (5 - 10) %  TTY_BUF_SIZE ��
	 *  �ᱻ����0xFFFF FFFB  ( =4294967291) ȥģ����TTY_BUF_SIZE������ʹ��ˡ�
	 */
	// unsigned int ans = this->m_Head - this->m_Tail;
	// ans = ans % TTY_BUF_SIZE;
	// return ans;
	
	int ans = (this->m_Head - this->m_Tail) & (TTy_Queue::TTY_BUF_SIZE - 1);
	return ans;
}

char* TTy_Queue::CurrentChar()
{
	/* ������һ����Ҫȡ���ַ��ĵ�ַ */
	return &this->m_CharBuf[m_Tail];
}

/*==============================class TTy===============================*/
/* ����̨����ʵ���Ķ��� */
TTy g_TTy;

TTy::TTy()
{
	this->m_HistoryHead = 0;
	this->m_HistoryCount = 0;
	this->m_HistoryViewIndex = -1;
}

TTy::~TTy()
{

}

/*
 * �ӱ�׼�������ȡ�ַ������û���
 * ֱ������Ϊ�գ�������Ӧ�ó���֮���裨u.u_IOParam.m_Count Ϊ  0��
 * ��䣬�����п�����˯����һ���� ��Ϊ��׼�������Ϊ�գ���ԭʼ���������û�ж����
 * ���û�û������س��������ܶ�ԭʼ�����е��������ݽ����޸ģ�
 * */
void TTy::TTRead()
{
	/* �豸û�п�ʼ���������� */
	if ( (this->t_state & TTy::CARR_ON) == 0 )
	{
		return;
	}

	if ( this->t_canq.CharNum() || this->Canon() )
	{
		while ( this->t_canq.CharNum() && (this->PassC(this->t_canq.GetChar()) >= 0) );
	}
}

/*
 * һ��һ���ֽڵؽ��û����еȴ�����������ͱ�׼������С�
 * ��������������������ˢ�����Դ档 ������̻��������ˢ�¡�
 * ����CRT::m_BeginChar����ָ����������д����һ���ַ��ĵ�Ԫ��BackSpace�����Բ�����ָ��֮ǰ���κ��ַ���
 */
void TTy::TTWrite()
{
	/* 
	 * ��Ϊ���ڵ�����豸���ڴ棬����Ӧ�ٶ��൱�죬����
	 * ����Ҫ�ڽ������Ժ����жϣ������Ĵ��۷ǳ������
	 * ԭ��unix v6����Щ��ͬ����������������ַ��Ĺ����й�
	 * �ж�����ֹ�û����뵫���ܱ�����ɾ����bug,����Ϊ����
	 * ���ܻᵼ��ʱ���ж���Ӧ���ӳ٣��������������أ���
	 * ��û����������
	 */
	char ch;
	
	 /* �豸û�п�ʼ���������� */
	if ( (this->t_state & TTy::CARR_ON) == 0 )
	{
		return;
	}

	while ( (ch = CPass()) > 0 )
	{
		/*��������г����涨�ַ�������Ҫ�Ͽ���ʾ */
		if ( this->t_outq.CharNum() > TTy::TTHIWAT)
		{
			this->TTStart();
			/* ��������BeginCharָ������ַ���������У�δȷ�ϲ��ֵ���ʼ����
			 * Ŀ�����ڲ�����Backspace��ɾ��д�ڱ�׼����ϵ����ݣ�Ʃ��������ʾ��֮�ࡣ
			 */
			CRT::m_BeginChar = this->t_outq.CurrentChar();
		}
		this->TTyOutput(ch);
	}
	this->TTStart();
	CRT::m_BeginChar = this->t_outq.CurrentChar();
	/* ����BeginCharΪ�˷�ֹ����ɾ����ӡ���ַ���������Ҫ�����ʾ���棬�������ǰ���������
	 * ���ַ��ڱ�ɾ��ʱ�����ܱ�ɾ����������ʵ�����Ѿ���ɾ���ˡ�
	 */
}

/* ���жϴ���������á�����ch��ɨ����ת���ɵ�ASCII�롣
 * ���ܣ���ch����ԭʼ������У���������л��� ���ͱ�׼������У�֮������TTStart���Դ棩
 * �����дֲڵĵط���this->t_rawq.PutChar(ch) ֮ǰû���ж�ԭʼ���������û��������ɵĽ����ԭʼ������ԭ������
 * ���ַ���ɾ����
 * ԭʼ���������ֻ��˵��һ�����⣺û�н����ڵȴ��������롣
 * ���� �Ľ�ϵͳ��TTyInput���������жϣ�û�н���˯�ߵȴ����������ʱ�򣬲�Ҫ��ch����ԭʼ���С�
 * */
void TTy::TTyInput(char ch)
{
//	if ( (ch &= 0xFF) == '\r' && (this->t_flags & TTy::CRMOD) )
//	{
//		ch = '\n';
//	}

	/* �����Сд�ն� */
//	if ( (this->t_flags & TTy::LCASE) && ch >= 'A' && ch <= 'Z' )
//	{
//		ch += 'a' - 'A';
//	}

	/* �������ַ�����ԭʼ�ַ�������� */
	this->t_rawq.PutChar(ch);

	//if ( this->t_flags & TTy::RAW || ch == '\n' || ch == TTy::CEOT )
	
	// ����Canon���̴����������������ʷ��¼�͹���ƶ���
	if ( ch == '\n' || ch == TTy::CEOT || 
		 ch == TTy::KEY_UP || ch == TTy::KEY_DOWN || 
		 ch == TTy::KEY_LEFT || ch == TTy::KEY_RIGHT )
	{
		Kernel::Instance().GetProcessManager().WakeUpAll((unsigned long)&this->t_rawq);
	}

	if ( ch == '\n' || ch == TTy::CEOT )
	{
		this->t_rawq.PutChar(0x7);
		this->t_delct++;
	}

	if ( this->t_flags & TTy::ECHO )
	{
		/* 
		 * ��ʷ��¼�л���(UP/DOWN)����Ҫ���ԣ�
		 * ����ᱻCRT������ͨ�ַ���ʾ(��!!��0�9)��
		 * ���ǵ���ʾ������Canon�����е��߼�������
		 * LEFT/RIGHT����Ҳ��Canon���д�������Ҫ������echo
		 */
		if ( ch != TTy::KEY_UP && ch != TTy::KEY_DOWN && 
		     ch != TTy::KEY_LEFT && ch != TTy::KEY_RIGHT )
		{
			this->TTyOutput(ch);
			this->TTStart();
		}
	}
}

void TTy::TTyOutput(char ch)
{
	/* ��������ַ�Ϊ�ļ��������������ն˹�����ԭʼ��ʽ�£��򷵻� */
	 /*if ( (ch &= 0xFF) == TTy::CEOT && (this->t_flags & TTy::RAW) == 0 )
	{
		return;
	}

	if ( '\n' == ch && (this->t_flags & TTy::CRMOD) )
	{
		this->TTyOutput('\r');
	} */

	/* ���ַ���������ַ���������� */
	if (ch)
	{
		this->t_outq.PutChar(ch);
	}
}

void TTy::TTStart()
{
	CRT::CRTStart(this);
}

void TTy::FlushTTy()
{
	while ( this->t_canq.GetChar() >= 0 );
	while ( this->t_outq.GetChar() >= 0 );
	Kernel::Instance().GetProcessManager().WakeUpAll((unsigned long)&this->t_canq);
	Kernel::Instance().GetProcessManager().WakeUpAll((unsigned long)&this->t_outq);
	
	X86Assembly::CLI();
	while ( this->t_rawq.GetChar() >= 0 );
	this->t_delct = 0;
	X86Assembly::STI();
}

int TTy::Canon()
{
     char* pChar = &Canonb[0];
     char* pEnd = &Canonb[0];
     char ch;
     User& u = Kernel::Instance().GetUser();
     
     // ������ʷ��¼�鿴������ÿ�����п�ʼʱĬ��Ϊ����������
     this->m_HistoryViewIndex = -1;

     while ( true )
     {
         X86Assembly::CLI();
         // �ȴ����룺ֱ���ж����(t_delct>0) ���� ԭʼ���������ַ�(����UP/DOWN/LEFT/RIGHT��)
         while ( this->t_delct == 0 && this->t_rawq.CharNum() == 0 )
         {
             if ( (this->t_state & TTy::CARR_ON) == 0 )
                 return 0;   // �豸û�򿪣�����
             u.u_procp->Sleep((unsigned long)&this->t_rawq, ProcessManager::TTIPRI);
         }
        X86Assembly::STI();

        // ����ԭʼ�����е��ַ�
        while ( (ch = this->t_rawq.GetChar()) >= 0 )
        {
            if ( 0x7 == ch )		/* �Ƕ����*/
            {
                this->t_delct--;     //  ԭʼ��������--
                goto EndOfLine;      //  ����ѭ�����ύ��ǰ��
            }

           if ( ch == this->t_erase )     	/* 即backspace */
           {
               if ( pChar > &Canonb[0] )
               {
                   pChar--;

                   char* readPos = pChar + 1;
                   char* writePos = pChar;
                   while ( readPos < pEnd )
                   {
                       *writePos++ = *readPos++;
                   }
                   pEnd--;

                   this->TTyOutput('\b');
                   this->TTStart();

                   int tailLen = pEnd - pChar;
                   for ( char* p = pChar; p < pEnd; ++p )
                   {
                       this->TTyOutput(*p);
                       this->TTStart();
                   }

                   this->TTyOutput(' ');
                   this->TTStart();

                   for ( int i = 0; i < tailLen + 1; ++i )
                   {
                       this->TTyOutput('\b');
                       this->TTStart();
                   }
               }
               continue;
           }

           if ( ch == TTy::CDEL )         /* Delete键 */
           {
               if ( pChar < pEnd )
               {
                   char* readPos = pChar + 1;
                   char* writePos = pChar;
                   while ( readPos < pEnd )
                   {
                       *writePos++ = *readPos++;
                   }
                   pEnd--;

                   int tailLen = pEnd - pChar;
                   for ( char* p = pChar; p < pEnd; ++p )
                   {
                       this->TTyOutput(*p);
                       this->TTStart();
                   }

                   this->TTyOutput(' ');
                   this->TTStart();

                   for ( int i = 0; i < tailLen + 1; ++i )
                   {
                       this->TTyOutput(TTy::KEY_LEFT);
                       this->TTStart();
                   }
               }
               continue;
           }

           if ( ch == TTy::KEY_LEFT )
           {
               if ( pChar > &Canonb[0] )
               {
                   pChar--;
                   // 发送LEFT键到输出队列，让CRT移动光标
                   this->TTyOutput(TTy::KEY_LEFT);
                   this->TTStart();
               }
               continue;
           }

           if ( ch == TTy::KEY_RIGHT )
           {
               if ( pChar < pEnd )
               {
                   pChar++;
                   // 发送RIGHT键到输出队列，让CRT移动光标
                   this->TTyOutput(TTy::KEY_RIGHT);
                   this->TTStart();
               }
               continue;
           }

           // ================== History UP ==================
           if ( ch == TTy::KEY_UP )
           {
               if ( this->m_HistoryCount == 0 ) continue;

               int newIndex;
               if ( this->m_HistoryViewIndex == -1 )
               {
                   // ���浱ǰ�ݸ�
                   int len = pEnd - &Canonb[0];
                   for(int i=0; i<len; i++) this->m_DraftBuffer[i] = Canonb[i];
                   this->m_DraftBuffer[len] = 0;

                   newIndex = (this->m_HistoryHead - 1 + TTy::HISTORY_SIZE) % TTy::HISTORY_SIZE;
               }
               else
               {
                   int oldest = (this->m_HistoryCount < TTy::HISTORY_SIZE) ? 0 : this->m_HistoryHead;
                   if ( this->m_HistoryViewIndex == oldest ) continue; 

                   newIndex = (this->m_HistoryViewIndex - 1 + TTy::HISTORY_SIZE) % TTy::HISTORY_SIZE;
               }

               this->m_HistoryViewIndex = newIndex;

               // �����ǰ�� (ע�⣺���뼰ʱ����TTStart��ֹt_outq���)
               while ( pChar > &Canonb[0] ) { this->TTyOutput('\b'); this->TTStart(); pChar--; }
               int currentLen = pEnd - &Canonb[0];
               for ( int i = 0; i < currentLen; i++ ) { this->TTyOutput(' '); this->TTStart(); }
               for ( int i = 0; i < currentLen; i++ ) { this->TTyOutput('\b'); this->TTStart(); }

               // ������ʷ
               char* hist = this->m_History[newIndex];
               char* dest = &Canonb[0];
               while ( *hist && dest < &Canonb[TTy::CANBSIZ - 1] ) *dest++ = *hist++;
               pEnd = dest;
               pChar = dest;

               // ���
               for ( char* p = &Canonb[0]; p < pEnd; p++ ) { this->TTyOutput(*p); this->TTStart(); }
               
               continue;
           }

           // ================== History DOWN ==================
           if ( ch == TTy::KEY_DOWN )
           {
               if ( this->m_HistoryViewIndex == -1 ) continue;

               int newIndex;
               int latest = (this->m_HistoryHead - 1 + TTy::HISTORY_SIZE) % TTy::HISTORY_SIZE;
               
               if ( this->m_HistoryViewIndex == latest )
               {
                   newIndex = -1; // �ص��ݸ�
               }
               else
               {
                   newIndex = (this->m_HistoryViewIndex + 1) % TTy::HISTORY_SIZE;
               }
               
               this->m_HistoryViewIndex = newIndex;

               // �����ǰ��
               while ( pChar > &Canonb[0] ) { this->TTyOutput('\b'); this->TTStart(); pChar--; }
               int currentLen = pEnd - &Canonb[0];
               for ( int i = 0; i < currentLen; i++ ) { this->TTyOutput(' '); this->TTStart(); }
               for ( int i = 0; i < currentLen; i++ ) { this->TTyOutput('\b'); this->TTStart(); }

               // ��������
               char* src;
               if ( newIndex == -1 ) src = this->m_DraftBuffer;
               else src = this->m_History[newIndex];

               char* dest = &Canonb[0];
               while ( *src && dest < &Canonb[TTy::CANBSIZ - 1] ) *dest++ = *src++;
               pEnd = dest;
               pChar = dest;

               // ���
               for ( char* p = &Canonb[0]; p < pEnd; p++ ) { this->TTyOutput(*p); this->TTStart(); }

               continue;
           }

           if ( ch == TTy::CEOT )	/* CEOT == 0x4������ ctrl + d�� */
               continue;       		/* ���ļ���������û������������ */

           if ( ch == '\n' && pChar < pEnd )
           {
               pChar = pEnd;    // 逻辑上把光标移动到行尾，避免额外输出
           }

           // 如果光标在中间位置，需要将后续字符后移，腾出空间插入新字符
           if ( pChar < pEnd )
           {
               // 将pChar到pEnd之间的字符向后移动一位
               char* src = pEnd - 1;
               char* dst = pEnd;
               while ( src >= pChar )
               {
                   *dst-- = *src--;
               }
               *pChar = ch;  // 插入新字符
               pChar++;
               pEnd++;
               
               // 重新显示从插入位置到行尾的所有字符
               for ( char* p = pChar - 1; p < pEnd; p++ )
               {
                   this->TTyOutput(*p);
                   this->TTStart();
               }
               
               // 将光标移回到插入位置之后
               int moveBack = pEnd - pChar;
               for ( int i = 0; i < moveBack; i++ )
               {
                   this->TTyOutput(TTy::KEY_LEFT);
                   this->TTStart();
               }
           }
           else
           {
               // 在行尾追加字符
               *pChar++ = ch;
               if (pChar > pEnd)
                   pEnd = pChar;
           }

           // ================== Save History on Enter ==================
           if ( ch == '\n' )
           {
               int len = pEnd - &Canonb[0] - 1; // ���������з�
               if ( len > 0 )
               {
                   for(int i=0; i<len; i++) this->m_History[m_HistoryHead][i] = Canonb[i];
                   this->m_History[m_HistoryHead][len] = 0;

                   this->m_HistoryHead = (this->m_HistoryHead + 1) % TTy::HISTORY_SIZE;
                   if ( this->m_HistoryCount < TTy::HISTORY_SIZE ) this->m_HistoryCount++;
               }
           }

           if ( pChar >= Canonb + TTy::CANBSIZ )
               break;    			/* Canonb���������ء�����ʣ���ַ����´�Canon����ִ��ʱ��ȡ */
        }
    }

EndOfLine:
    // �ύ����������ӿ�ʼ��pEnd������ǰ�⹦���е�pChar��
    char* pSubmit = &Canonb[0];

    while ( pSubmit < pEnd )
        this->t_canq.PutChar(*pSubmit++);   /* ��Cannonb�����д��������ַ��ͱ�׼���� */

    return 1;
}int TTy::PassC(char ch)
{
	User& u = Kernel::Instance().GetUser();

	/* ���ַ������û�Ŀ���� */
	if ( u.u_IOParam.m_Count > 0 )
	{
		*(u.u_IOParam.m_Base++) = ch;
		//u.u_IOParam.m_Offset++;
		u.u_IOParam.m_Count--;
		return 0;
	}
	return -1;
}

char TTy::CPass()
{
	char ch;
	User& u = Kernel::Instance().GetUser();

	ch = *(u.u_IOParam.m_Base++);
	if ( u.u_IOParam.m_Count > 0 )
	{
		u.u_IOParam.m_Count--;
		//u.u_IOParam.m_Offset++;
		return ch;
	}
	else
	{
		return -1;
	}
}



