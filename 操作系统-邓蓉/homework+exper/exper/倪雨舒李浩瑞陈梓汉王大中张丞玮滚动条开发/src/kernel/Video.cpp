#include "Video.h"

// ========================= 基本静态变量 =========================
unsigned short* Diagnose::m_VideoMemory = (unsigned short *)(0xB8000 + 0xC0000000);
unsigned int    Diagnose::m_Row    = 0;      // 仅作为“下半屏起始行”使用，会在 InitScrollBuffer() 里设为 SCREEN_ROWS-ROWS
unsigned int    Diagnose::m_Column = 0;
unsigned int    Diagnose::ROWS     = 10;     // 下半屏可视行数
bool            Diagnose::trace_on = true;

// ========================= 滚动缓冲状态 =========================
const unsigned int Diagnose::BUFFER_LINES;        // 在 Video.h 中声明为 const，定义由编译器处理
unsigned short*   Diagnose::m_ScrollBuffer = 0;   // 环形缓冲：BUFFER_LINES * COLUMNS
unsigned int      Diagnose::m_CurrentLine  = 0;   // 已写入的当前行号（从 0 开始递增）
unsigned int      Diagnose::m_TotalLinesWritten = 0; // 历史总行数（<=BUFFER_LINES）
unsigned int      Diagnose::m_ScrollOffset = 0;   // 0=显示最新；>0 为向上翻的行数
bool              Diagnose::m_IsScrolling  = false;

Diagnose::Diagnose() {}
Diagnose::~Diagnose() {}

void Diagnose::TraceOn()  { trace_on = true;  }
void Diagnose::TraceOff() { trace_on = false; }

// ========================= 核心：初始化/刷新/滚动 =========================
// 初始化滚动缓冲：不清屏，只刷新自己的窗口
void Diagnose::InitScrollBuffer()
{
    static unsigned short kStaticBuffer[BUFFER_LINES * COLUMNS];
    m_ScrollBuffer = kStaticBuffer;

    // 设置下半屏诊断窗口的起始行
    m_Row    = SCREEN_ROWS - ROWS;
    m_Column = 0;

    // 清空历史缓冲
    for (unsigned int i = 0; i < BUFFER_LINES * COLUMNS; ++i)
        m_ScrollBuffer[i] = (unsigned char)' ' | COLOR;

    m_CurrentLine        = 0;
    m_TotalLinesWritten  = 0;
    m_ScrollOffset       = 0;
    m_IsScrolling        = false;

    // 仅根据当前偏移把窗口画一遍（不清整屏）
    RefreshScreen();
}

// 按 m_ScrollOffset 把历史缓冲中的 ROWS 行拷贝到“下半屏”窗口
void Diagnose::RefreshScreen()
{
    if (!m_ScrollBuffer) return;

    // 下半屏窗口在 VRAM 的起点
    const unsigned int base = (SCREEN_ROWS - ROWS) * COLUMNS;

    // 计算历史起始行
    unsigned int startLine = 0;
    if (m_CurrentLine + 1 >= ROWS + m_ScrollOffset)
        startLine = (m_CurrentLine + 1) - ROWS - m_ScrollOffset;

    for (unsigned int row = 0; row < ROWS; ++row)
    {
        unsigned int line    = startLine + row;
        unsigned int srcLine = (line >= BUFFER_LINES) ? (line % BUFFER_LINES) : line;
        unsigned int src     = srcLine * COLUMNS;
        unsigned int dst     = base + row * COLUMNS;

        for (unsigned int col = 0; col < COLUMNS; ++col)
            m_VideoMemory[dst + col] = m_ScrollBuffer[src + col];
    }
}

// 向上滚动一行
void Diagnose::ScrollUp()
{
    if (!m_ScrollBuffer) return;

    unsigned int written = (m_TotalLinesWritten < BUFFER_LINES) ? m_TotalLinesWritten : BUFFER_LINES;
    if (written <= ROWS) return;

    unsigned int maxOffset = written - ROWS;
    if (m_ScrollOffset < maxOffset)
    {
        ++m_ScrollOffset;
        m_IsScrolling = true;
        RefreshScreen();
    }
}

// 向下滚动一行
void Diagnose::ScrollDown()
{
    if (!m_ScrollBuffer) return;
    if (m_ScrollOffset == 0) return;

    --m_ScrollOffset;
    if (m_ScrollOffset == 0) m_IsScrolling = false;
    RefreshScreen();
}

// 回到最新
void Diagnose::GoToLatestLine()
{
    if (!m_ScrollBuffer) return;
    if (m_ScrollOffset == 0) return;

    m_ScrollOffset = 0;
    m_IsScrolling  = false;
    RefreshScreen();
}

// 仅清“下半屏窗口”自身（一般不会自动调用；保留以备手工使用）
void Diagnose::ClearScreen()
{
    const unsigned short blank = (unsigned char)' ' | COLOR;
    const unsigned int base = (SCREEN_ROWS - ROWS) * COLUMNS;

    m_Row    = SCREEN_ROWS - ROWS;
    m_Column = 0;

    for (unsigned int row = 0; row < ROWS; ++row)
    {
        unsigned int dst = base + row * COLUMNS;
        for (unsigned int col = 0; col < COLUMNS; ++col)
            m_VideoMemory[dst + col] = blank;
    }
}

// ========================= 输出实现（写缓冲而非直接写显存） =========================
/* printf 风格（%d/%x/%s/\n），保持你原有逻辑不变 */
void Diagnose::Write(const char* fmt, ...)
{
    if (!trace_on) return;

    unsigned int * va_arg = (unsigned int *)&fmt + 1;
    const char * ch = fmt;

    while (1)
    {
        while (*ch != '%' && *ch != '\n')
        {
            if (*ch == '\0') return;
            if (*ch == '\n') break;
            WriteChar(*ch++);
        }

        ch++;   // skip '%' or '\n'

        if (*ch == 'd' || *ch == 'x')
        {
            int value = (int)(*va_arg);
            va_arg++;
            if (*ch == 'x') Write("0x");
            PrintInt(value, *ch == 'd' ? 10 : 16);
            ch++;   // skip 'd' or 'x'
        }
        else if (*ch == 's')
        {
            ch++;   // skip 's'
            char *str = (char *)(*va_arg);
            va_arg++;
            while (char tmp = *str++) WriteChar(tmp);
        }
        else  /* '\n' */
        {
            NextLine();
        }
    }
}

// 以 base 进制打印整数（递归）
void Diagnose::PrintInt(unsigned int value, int base)
{
    static char Digits[] = "0123456789ABCDEF";
    int i;
    if ((i = value / base) != 0) PrintInt(i, base);
    WriteChar(Digits[value % base]);
}

// 换行：推进到下一历史行（写缓冲，不改 VRAM）
void Diagnose::NextLine()
{
    if (!m_ScrollBuffer) InitScrollBuffer();

    // 把本行剩余位置填空格
    unsigned int bufferLine = m_CurrentLine % BUFFER_LINES;
    for (unsigned int col = m_Column; col < COLUMNS; ++col)
        m_ScrollBuffer[bufferLine * COLUMNS + col] = (unsigned char)' ' | COLOR;

    m_Column = 0;

    // 推进到下一行
    ++m_CurrentLine;
    if (m_TotalLinesWritten < BUFFER_LINES) ++m_TotalLinesWritten;

    RefreshScreen();
}

// 单字符输出：写入缓冲并刷新（不直接写显存，不自动清屏）
void Diagnose::WriteChar(const char ch)
{
    if (!m_ScrollBuffer) InitScrollBuffer();

    if (ch == '\n') { NextLine(); return; }
    if (ch == '\r') { m_Column = 0; RefreshScreen(); return; }
    if (ch == '\t') { for (int i = 0; i < 4; ++i) WriteChar(' '); return; }
    if (ch == '\b') { if (m_Column > 0) --m_Column; RefreshScreen(); return; }

    // 可见字符：写入当前行
    unsigned int bufferLine = m_CurrentLine % BUFFER_LINES;
    m_ScrollBuffer[bufferLine * COLUMNS + m_Column] = (unsigned char)ch | COLOR;

    if (++m_Column >= COLUMNS)
    {
        m_Column = 0;
        ++m_CurrentLine;
        if (m_TotalLinesWritten < BUFFER_LINES) ++m_TotalLinesWritten;
    }

    RefreshScreen();
}
