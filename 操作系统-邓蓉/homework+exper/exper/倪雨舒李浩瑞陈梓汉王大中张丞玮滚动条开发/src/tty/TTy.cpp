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
	/* 当Head < Tail时使用%运算会有问题！应该用'&'运算。
	 *  譬如Head = 5，Tail = 10， (5 - 10) %  TTY_BUF_SIZE 结果
	 *  会被当做0xFFFF FFFB  ( =4294967291) 去模，结果就错了。
	 */
	// unsigned int ans = this->m_Head - this->m_Tail;
	// ans = ans % TTY_BUF_SIZE;
	// return ans;
	
	int ans = (this->m_Head - this->m_Tail) & (TTy_Queue::TTY_BUF_SIZE - 1);
	return ans;
}

char* TTy_Queue::CurrentChar()
{
	/* 返回下一个要取出的字符的地址 */
	return &this->m_CharBuf[m_Tail];
}

/*==============================class TTy===============================*/
/* 控制台终端实例的定义 */
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
 * 从标准输入队列取字符，送给用户。
 * 直到队列为空，或者满足应用层读之请求（u.u_IOParam.m_Count 为  0）
 * 注意，这里有可能会睡眠，因为标准输入队列为空，而原始队列里可能还有东西
 * 如果没有回车，那么原始队列里的数据就不会被处理，
 * */
void TTy::TTRead()
{
	/* 设备没有开启，直接返回 */
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
 * 一一个字节地将用户缓冲区里的数据送到标准输出队列中。
 * 如果输出队列满了，则刷新显存。 否则就缓冲，不刷新。
 * 记录CRT::m_BeginChar，指向输出队列中写入的第一个字符的单元，BackSpace键不可以删除该指针之前的任何字符。
 */
void TTy::TTWrite()
{
	/* 
	 * 因为终端的输出设备是内存，响应速度相当快，所以
	 * 不需要在这里做中断，这里的代码非常简单。
	 * 原版unix v6在这里有些不同，因为输出设备是慢速的串口
	 * 有代码防止用户输入但可能被输出删除的bug,这里因为输出
	 * 不可能会导致时钟中断响应延迟，所以这里没有问题，
	 * 也就没有了那些代码。
	 */
	char ch;
	
	 /* 设备没有开启，直接返回 */
	if ( (this->t_state & TTy::CARR_ON) == 0 )
	{
		return;
	}

	while ( (ch = CPass()) > 0 )
	{
		/* 如果输出队列中字符数量超过高水位线 */
		if ( this->t_outq.CharNum() > TTy::TTHIWAT)
		{
			this->TTStart();
			/* 记录下BeginChar指向输出字符队列中，未确认部分的开始处
			 * 目的是为了BackSpace键删除写在标准输出上的内容，譬如输入密码之类。
			 */
			CRT::m_BeginChar = this->t_outq.CurrentChar();
		}
		this->TTyOutput(ch);
	}
	this->TTStart();
	CRT::m_BeginChar = this->t_outq.CurrentChar();
	/* 更新BeginChar为了防止被删除打印的字符，因为这里要打印提示符，而提示符前面的内容
	 * 是字符在被删除时，不可能被删除的，其实是已经打印删除了。
	 */
}

/* 键盘中断处理程序调用。将ch（扫描码转换成的ASCII码）
 * 可能，将ch放入原始输入队列中，如果回车，则唤醒标准输入队列（唤醒TTStart，显存）
 * 这里有粗糙的地方，this->t_rawq.PutChar(ch) 之前没有判断原始输入队列满，没有处理溢出的情况，原始队列满，
 * 则字符被删除。
 * 原始输入队列满只会在一种情况下：没有进程在等待输入，而输入过快。
 * 另外 文件系统的TTyInput这里没有判断，没有进程睡眠等待输入的时候，不要将ch放入原始队列。
 * */
void TTy::TTyInput(char ch)
{
	/* 将输入字符放入原始字符队列中 */
	this->t_rawq.PutChar(ch);

	//if ( this->t_flags & TTy::RAW || ch == '\n' || ch == TTy::CEOT )
	
	// 唤醒Canon进程处理原始队列，将数据放入规范队列中
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
		 * 历史记录切换(UP/DOWN)不需要回显，
		 * 否则会给CRT发送普通字符显示(如!!或0~9)，
		 * 而是等待显示逻辑在Canon进程中处理。
		 * LEFT/RIGHT按键也在Canon进程中处理，不需要在这里echo
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

	/* 将字符放入输出字符队列中 */
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
     
     // 进入历史记录查看模式，每行输入开始时默认为不查看历史
     this->m_HistoryViewIndex = -1;

     while ( true )
     {
         X86Assembly::CLI();
         // 等待输入：直到有定界符(t_delct>0) 或者 原始输入队列有字符(例如UP/DOWN/LEFT/RIGHT等)
         while ( this->t_delct == 0 && this->t_rawq.CharNum() == 0 )
         {
             if ( (this->t_state & TTy::CARR_ON) == 0 )
                 return 0;   // 设备没打开，返回
             u.u_procp->Sleep((unsigned long)&this->t_rawq, ProcessManager::TTIPRI);
         }
        X86Assembly::STI();

        // 处理原始队列中的字符
        while ( (ch = this->t_rawq.GetChar()) >= 0 )
        {
            if ( 0x7 == ch )		/* 是定界符 */
            {
                this->t_delct--;     //  原始队列定界符--
                goto EndOfLine;      //  跳出循环，提交当前行
            }

           if ( ch == this->t_erase )     	/* 是backspace */
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
                   // 保存当前草稿
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

               // 清除前半段 (注意：必须及时调用TTStart防止t_outq满)
               while ( pChar > &Canonb[0] ) { this->TTyOutput('\b'); this->TTStart(); pChar--; }
               int currentLen = pEnd - &Canonb[0];
               for ( int i = 0; i < currentLen; i++ ) { this->TTyOutput(' '); this->TTStart(); }
               for ( int i = 0; i < currentLen; i++ ) { this->TTyOutput('\b'); this->TTStart(); }

               // 拷贝历史
               char* hist = this->m_History[newIndex];
               char* dest = &Canonb[0];
               while ( *hist && dest < &Canonb[TTy::CANBSIZ - 1] ) *dest++ = *hist++;
               pEnd = dest;
               pChar = dest;

               // 回显
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
                   newIndex = -1; // 回到草稿
               }
               else
               {
                   newIndex = (this->m_HistoryViewIndex + 1) % TTy::HISTORY_SIZE;
               }
               
               this->m_HistoryViewIndex = newIndex;

               // 清除前半段
               while ( pChar > &Canonb[0] ) { this->TTyOutput('\b'); this->TTStart(); pChar--; }
               int currentLen = pEnd - &Canonb[0];
               for ( int i = 0; i < currentLen; i++ ) { this->TTyOutput(' '); this->TTStart(); }
               for ( int i = 0; i < currentLen; i++ ) { this->TTyOutput('\b'); this->TTStart(); }

               // 拷贝内容
               char* src;
               if ( newIndex == -1 ) src = this->m_DraftBuffer;
               else src = this->m_History[newIndex];

               char* dest = &Canonb[0];
               while ( *src && dest < &Canonb[TTy::CANBSIZ - 1] ) *dest++ = *src++;
               pEnd = dest;
               pChar = dest;

               // 回显
               for ( char* p = &Canonb[0]; p < pEnd; p++ ) { this->TTyOutput(*p); this->TTStart(); }

               continue;
           }

           if ( ch == TTy::CEOT )	/* CEOT == 0x4 (ctrl + d) */
               continue;       		/* 文件结束符，这里没做处理 */

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
               int len = pEnd - &Canonb[0] - 1; // 计算行长度
               if ( len > 0 )
               {
                   for(int i=0; i<len; i++) this->m_History[m_HistoryHead][i] = Canonb[i];
                   this->m_History[m_HistoryHead][len] = 0;

                   this->m_HistoryHead = (this->m_HistoryHead + 1) % TTy::HISTORY_SIZE;
                   if ( this->m_HistoryCount < TTy::HISTORY_SIZE ) this->m_HistoryCount++;
               }
           }

           if ( pChar >= Canonb + TTy::CANBSIZ )
               break;    			/* Canonb满，截断。剩余字符下次Canon执行时读取 */
        }
    }

EndOfLine:
    // 提交行：从开始到pEnd（注意前面功能键处理到了pChar）
    char* pSubmit = &Canonb[0];

    while ( pSubmit < pEnd )
        this->t_canq.PutChar(*pSubmit++);   /* 将Canonb缓冲区中的字符送入标准队列 */

    return 1;
}

int TTy::PassC(char ch)
{
	User& u = Kernel::Instance().GetUser();

	/* 将字符放入用户目标区 */
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



