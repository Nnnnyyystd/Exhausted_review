#include "CRT.h"
#include "IOPort.h"

// 显存指针与光标位置
unsigned short* CRT::m_VideoMemory = (unsigned short*)(0xB8000 + 0xC0000000);
unsigned int CRT::m_CursorX = 0;
unsigned int CRT::m_CursorY = 0;
char* CRT::m_Position = 0;
char* CRT::m_BeginChar = 0;

// 屏幕行数
unsigned int CRT::ROWS = 15;

// ======= 滚动缓冲区相关静态成员定义 =======
unsigned short* CRT::m_ScrollBuffer = 0;
unsigned int    CRT::m_CurrentLine = 0;
unsigned int    CRT::m_TotalLinesWritten = 0;
unsigned int    CRT::m_ScrollOffset = 0;
bool            CRT::m_IsScrolling = false;

//======================================================================
// 从 TTY 输出队列取字符并显示
//======================================================================
void CRT::CRTStart(TTy* pTTy)
{
    char ch;
    if (0 == CRT::m_BeginChar)
    {
        m_BeginChar = pTTy->t_outq.CurrentChar();
    }
    if (0 == m_Position)
    {
        m_Position = m_BeginChar;
    }

    while ((ch = pTTy->t_outq.GetChar()) != TTy::GET_ERROR)
    {
        switch (ch)
        {
        case '\n':
            NextLine();
            CRT::m_BeginChar = pTTy->t_outq.CurrentChar();
            m_Position = CRT::m_BeginChar;
            break;

        case 0x15:
            // del_line();
            break;

        case '\b':
            if (m_Position != CRT::m_BeginChar)
            {
                BackSpace();
                m_Position--;
            }
            break;

        case '\t':
            Tab();
            m_Position++;
            break;

        case TTy::KEY_LEFT:
            if (m_Position > m_BeginChar)
            {
                if (m_CursorX == 0) {
                    if (m_CursorY > 0) {
                        m_CursorY--;
                        m_CursorX = CRT::COLUMNS - 1;
                    }
                } else {
                    m_CursorX--;
                }
                MoveCursor(m_CursorX, m_CursorY);
                m_Position--;
            }
            break;

        case TTy::KEY_RIGHT:
            // Canon已经控制了RIGHT的边界，这里只需移动光标
            m_CursorX++;
            if (m_CursorX >= CRT::COLUMNS) {
                m_CursorX = 0;
                m_CursorY++;
                if (m_CursorY >= CRT::ROWS) {
                     m_CursorX = CRT::COLUMNS - 1; 
                     m_CursorY = CRT::ROWS - 1;
                }
            }
            MoveCursor(m_CursorX, m_CursorY);
            m_Position++;
            break;
        
        case TTy::KEY_UP:
        case TTy::KEY_DOWN:
            // UP/DOWN键不应该出现在输出队列中
            break;

        default:	/* 在屏幕上回显普通字符 */
            // 过滤掉特殊控制字符，避免显示乱码
            if ((unsigned char)ch >= 0x20 || ch == '\t')
            {
                WriteChar(ch);
                m_Position++;
            }
            break;
        }
    }
}

//======================================================================
// 移动硬件光标
//======================================================================
void CRT::MoveCursor(unsigned int col, unsigned int row)
{
    if ((col >= CRT::COLUMNS) || (row >= CRT::ROWS))
    {
        return;
    }

    /* 计算光标偏移量 */
    unsigned short cursorPosition = row * CRT::COLUMNS + col;

    /* 选择寄存器，分别为光标位置的高8位和低8位 */
    IOPort::OutByte(CRT::VIDEO_ADDR_PORT, 14);
    IOPort::OutByte(CRT::VIDEO_DATA_PORT, cursorPosition >> 8);
    IOPort::OutByte(CRT::VIDEO_ADDR_PORT, 15);
    IOPort::OutByte(CRT::VIDEO_DATA_PORT, cursorPosition & 0xFF);
}

//======================================================================
// 换行 + 屏幕滚动 + 缓冲区行号维护
//======================================================================
void CRT::NextLine()
{
    m_CursorX = 0;
    ++m_CursorY;

    // 1. 缓冲区行号递增并清空新行
    if (m_ScrollBuffer != 0)
    {
        ++m_CurrentLine;
        ++m_TotalLinesWritten;

        unsigned int newBufferLine = m_CurrentLine % BUFFER_LINES;
        unsigned int newBufferOffset = newBufferLine * COLUMNS;
        for (unsigned int col = 0; col < COLUMNS; ++col)
            m_ScrollBuffer[newBufferOffset + col] = ' ' | CRT::COLOR;
    }

    // 2. 屏幕滚动（仅在未查看历史时）
    if (m_CursorY >= ROWS)
    {
        if (m_ScrollBuffer != 0 && m_ScrollOffset == 0)
        {
            // 向上滚动屏幕内容
            for (unsigned int row = 0; row < ROWS - 1; ++row)
            {
                for (unsigned int col = 0; col < COLUMNS; ++col)
                {
                    m_VideoMemory[row * COLUMNS + col] =
                        m_VideoMemory[(row + 1) * COLUMNS + col];
                }
            }

            // 清空最后一行
            for (unsigned int col = 0; col < COLUMNS; ++col)
                m_VideoMemory[(ROWS - 1) * COLUMNS + col] = ' ' | CRT::COLOR;
        }
        m_CursorY = ROWS - 1;
    }

    if (m_ScrollOffset == 0)
        MoveCursor(m_CursorX, m_CursorY);

    DrawScrollBar();
}

//======================================================================
// 退格
//======================================================================
void CRT::BackSpace()
{
    m_CursorX--;

    /* 移动光标，如果要回到上一行的话 */
    if (m_CursorX >= (unsigned int)CRT::COLUMNS) // 由于是 unsigned，要防止下溢
    {
        m_CursorX = CRT::COLUMNS - 1;
        if (m_CursorY > 0)
        {
            --m_CursorY;
            if (m_ScrollBuffer != 0 && m_CurrentLine > 0)
            {
                --m_CurrentLine;
                --m_TotalLinesWritten;
            }
        }
        else
        {
            m_CursorY = 0;
        }
    }
    MoveCursor(m_CursorX, m_CursorY);

    /* 在光标所在位置填上空格 */
    m_VideoMemory[m_CursorY * COLUMNS + m_CursorX] = ' ' | CRT::COLOR;

    if (m_ScrollBuffer != 0)
    {
        unsigned int bufferLine = m_CurrentLine % BUFFER_LINES;
        unsigned int bufferOffset = bufferLine * COLUMNS + m_CursorX;
        m_ScrollBuffer[bufferOffset] = ' ' | CRT::COLOR;
    }
}

//======================================================================
// 制表符
//======================================================================
void CRT::Tab()
{
    m_CursorX &= 0xFFFFFFF8;	/* 向左对齐到前一个Tab边界 */
    m_CursorX += 8;

    if (m_CursorX >= CRT::COLUMNS)
        NextLine();
    else
        MoveCursor(m_CursorX, m_CursorY);
}

//======================================================================
// 写字符：同时写入滚动缓冲区与当前屏幕
//======================================================================
void CRT::WriteChar(char ch)
{
    // 1. 优先写缓冲区（确保数据不丢失）
    if (m_ScrollBuffer != 0)
    {
        unsigned int bufferLine = m_CurrentLine % BUFFER_LINES;
        unsigned int bufferOffset = bufferLine * COLUMNS + m_CursorX;
        m_ScrollBuffer[bufferOffset] = (unsigned char)ch | CRT::COLOR;
    }

    // 2. 只有在未滚动时才写屏幕（查看历史时不破坏显示）
    if (m_ScrollOffset == 0)
    {
        m_VideoMemory[m_CursorY * COLUMNS + m_CursorX] =
            (unsigned char)ch | CRT::COLOR;
    }

    // 3. 更新光标
    ++m_CursorX;
    if (m_CursorX >= COLUMNS)
        NextLine();
    else if (m_ScrollOffset == 0)
        MoveCursor(m_CursorX, m_CursorY);
}

//======================================================================
// 清屏（只清当前屏幕，不动滚动缓冲区）
//======================================================================
void CRT::ClearScreen()
{
    unsigned int i;
    for (i = 0; i < COLUMNS * ROWS; ++i)
        m_VideoMemory[i] = (unsigned short)' ' | CRT::COLOR;

    m_CursorX = 0;
    m_CursorY = 0;
    MoveCursor(0, 0);
}

//======================================================================
// 初始化滚动缓冲区（在系统启动阶段调用）
//======================================================================
void CRT::InitScrollBuffer()
{
    static unsigned short scrollBufferMemory[BUFFER_LINES * COLUMNS];
    m_ScrollBuffer = scrollBufferMemory;

    // 1. 初始化为空格+默认颜色
    for (unsigned int i = 0; i < BUFFER_LINES * COLUMNS; ++i)
        m_ScrollBuffer[i] = ' ' | CRT::COLOR;

    // 2. 保存当前屏幕内容（系统启动输出）
    for (unsigned int row = 0; row < ROWS; ++row)
    {
        for (unsigned int col = 0; col < COLUMNS; ++col)
        {
            m_ScrollBuffer[row * COLUMNS + col] =
                m_VideoMemory[row * COLUMNS + col];
        }
    }

    // 3. 根据光标位置初始化行号
    m_CurrentLine = m_CursorY;
    m_TotalLinesWritten = m_CursorY + 1;
    m_ScrollOffset = 0;
    m_IsScrolling = false;

    DrawScrollBar();
}

//======================================================================
// 从缓冲区读一行到屏幕指定行
//======================================================================
void CRT::ReadLineFromBuffer(unsigned int absLine, unsigned int screenRow)
{
    unsigned int physLine = absLine % BUFFER_LINES;
    unsigned int offset = physLine * COLUMNS;
    for (unsigned int col = 0; col < COLUMNS; ++col)
        m_VideoMemory[screenRow * COLUMNS + col] = m_ScrollBuffer[offset + col];
}

//======================================================================
// 刷新屏幕显示，根据 m_ScrollOffset 选择窗口
//======================================================================
void CRT::RefreshScreen()
{
    if (m_ScrollBuffer == 0 || m_TotalLinesWritten == 0)
        return;

    // 1. 计算起始行号
    // startLine = (m_CurrentLine + 1) - ROWS - m_ScrollOffset
    unsigned int startLine;
    if (m_CurrentLine + 1 >= ROWS + m_ScrollOffset)
        startLine = (m_CurrentLine + 1) - ROWS - m_ScrollOffset;
    else
        startLine = 0;

    // 2. 从缓冲区读取并显示
    for (unsigned int screenRow = 0; screenRow < ROWS; ++screenRow)
    {
        unsigned int bufferLine = startLine + screenRow;

        if (bufferLine > m_CurrentLine)
        {
            // 超出当前行，显示空行
            for (unsigned int col = 0; col < COLUMNS; ++col)
                m_VideoMemory[screenRow * COLUMNS + col] = ' ' | CRT::COLOR;
        }
        else
        {
            ReadLineFromBuffer(bufferLine, screenRow);
        }
    }

    // 3. 更新光标
    if (m_ScrollOffset == 0)
        MoveCursor(m_CursorX, m_CursorY);      // 最新位置
    else
        MoveCursor(COLUMNS - 1, ROWS - 1);     // 查看历史时，把光标放在右下角

    DrawScrollBar();
}

//======================================================================
// 上滚一行（PageUp）
//======================================================================
void CRT::ScrollUp()
{
    if (m_ScrollBuffer == 0 || m_TotalLinesWritten <= ROWS)
        return;

    // 计算最大偏移量
    unsigned int maxOffset =
        (m_TotalLinesWritten < BUFFER_LINES)
        ? (m_TotalLinesWritten - ROWS)
        : (BUFFER_LINES - ROWS);

    if (m_ScrollOffset < maxOffset)
        ++m_ScrollOffset;

    m_IsScrolling = true;
    RefreshScreen();
}

//======================================================================
// 下滚一行（PageDown）
//======================================================================
void CRT::ScrollDown()
{
    if (m_ScrollBuffer == 0 || m_ScrollOffset == 0)
        return;

    --m_ScrollOffset;
    if (m_ScrollOffset == 0)
        m_IsScrolling = false;

    RefreshScreen();
}
void CRT::GoToLatestLine()
{
    // 没有缓冲区直接返回
    if (m_ScrollBuffer == 0)
        return;

    // 已经在最新位置就不用动
    if (m_ScrollOffset == 0)
        return;

    // 退出历史模式
    m_ScrollOffset = 0;
    m_IsScrolling = false;

    // 用 offset = 0 刷新屏幕，此时 RefreshScreen 会显示最新 ROWS 行
    // 并把光标移回 m_CursorX, m_CursorY（你之前的实现已经这样写了）
    RefreshScreen();
}

//======================================================================
// 缁樺埗婊氬姩鏉
//======================================================================
void CRT::DrawScrollBar()
{
    if (m_ScrollBuffer == 0) return;

    // 璁＄畻鏈夋晥鎬昏鏁帮紙涓嶈秴杩囩紦鍐插尯澶у皬锛
    unsigned int effectiveTotal = (m_TotalLinesWritten < BUFFER_LINES) ? m_TotalLinesWritten : BUFFER_LINES;
    
    // 榛樿婊戝潡浣嶇疆鍦ㄦ渶搴曢儴
    unsigned int thumbPos = ROWS - 1; 

    if (effectiveTotal > ROWS)
    {
        unsigned int maxOffset = effectiveTotal - ROWS;
        // 纭繚 offset 涓嶈秺鐣
        unsigned int currentOffset = (m_ScrollOffset > maxOffset) ? maxOffset : m_ScrollOffset;
        
        // 璁＄畻婊戝潡浣嶇疆锛
        // m_ScrollOffset = 0 (鏈€搴曠) -> thumbPos = ROWS - 1
        // m_ScrollOffset = maxOffset (鏈€椤剁) -> thumbPos = 0
        // 浣跨敤 (ROWS - 1) 浣滀负鏄犲皠鑼冨洿
        
        if (maxOffset > 0)
        {
            thumbPos = (ROWS - 1) - (currentOffset * (ROWS - 1) / maxOffset);
        }
    }

    // 缁樺埗婊氬姩鏉
    for (unsigned int i = 0; i < ROWS; ++i)
    {
        unsigned int offset = i * COLUMNS + (COLUMNS - 1); // 鏈€鍚庝竴鍒
        
        if (i == thumbPos)
        {
            // 婊戝潡锛氱櫧鑹插疄蹇冩柟鍧 (0xDB)
            m_VideoMemory[offset] = 0xDB | 0x0F00; 
        }
        else
        {
            // 杞ㄩ亾锛氱伆鑹查槾褰 (0xB0)锛屼娇鐢ㄦ祬鐏板墠鏅
            // 0x0700 鏄 Light Grey on Black
            m_VideoMemory[offset] = 0xB0 | 0x0700; 
        }
    }
}
