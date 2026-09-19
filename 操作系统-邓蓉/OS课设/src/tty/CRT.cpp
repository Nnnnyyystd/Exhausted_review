#include "CRT.h"
#include "IOPort.h"

// 鏄惧瓨鎸囬拡涓庡厜鏍囦綅缃�1锟�7
unsigned short* CRT::m_VideoMemory = (unsigned short*)(0xB8000 + 0xC0000000);
unsigned int CRT::m_CursorX = 0;
unsigned int CRT::m_CursorY = 0;
char* CRT::m_Position = 0;
char* CRT::m_BeginChar = 0;

// 灞忓箷琛屾暟
unsigned int CRT::ROWS = 15;

// ======= 婊氬姩缂撳啿鍖虹浉鍏抽潤鎬佹垚鍛樺畾涔�1锟�7 =======
unsigned short* CRT::m_ScrollBuffer = 0;
unsigned int    CRT::m_CurrentLine = 0;
unsigned int    CRT::m_TotalLinesWritten = 0;
unsigned int    CRT::m_ScrollOffset = 0;
bool            CRT::m_IsScrolling = false;

//======================================================================
// 浠�1锟�7 TTY 杈撳嚭闃熷垪鍙栧瓧绗﹀苟鏄剧ず
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
            // Canon宸茬粡鎺у埗浜哛IGHT鐨勮竟鐣岋紝杩欓噷鍙渶绉诲姩鍏夋爣
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
            // UP/DOWN閿笉搴旇鍑虹幇鍦ㄨ緭鍑洪槦鍒椾腑
            break;

        default:	/* 鍦ㄥ睆骞曚笂鍥炴樉鏅€氬瓧绗� */
            // 杩囨护鎺夌壒娈婃帶鍒跺瓧绗︼紝閬垮厤鏄剧ず涔辩爜
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
// 绉诲姩纭欢鍏夋爣
//======================================================================
void CRT::MoveCursor(unsigned int col, unsigned int row)
{
    if ((col >= CRT::COLUMNS) || (row >= CRT::ROWS))
    {
        return;
    }

    /* 璁＄畻鍏夋爣鍋忕Щ閲�1锟�7 */
    unsigned short cursorPosition = row * CRT::COLUMNS + col;

    /* 閫夋嫨瀵勫瓨鍣紝鍒嗗埆涓哄厜鏍囦綅缃殑楂�1锟�78浣嶅拰浣�1锟�78浣�1锟�7 */
    IOPort::OutByte(CRT::VIDEO_ADDR_PORT, 14);
    IOPort::OutByte(CRT::VIDEO_DATA_PORT, cursorPosition >> 8);
    IOPort::OutByte(CRT::VIDEO_ADDR_PORT, 15);
    IOPort::OutByte(CRT::VIDEO_DATA_PORT, cursorPosition & 0xFF);
}

//======================================================================
// 鎹㈣ + 灞忓箷婊氬姩 + 缂撳啿鍖鸿鍙风淮鎶�1锟�7
//======================================================================
void CRT::NextLine()
{
    m_CursorX = 0;
    ++m_CursorY;

    // 1. 缂撳啿鍖鸿鍙凤拷锟藉骞舵竻绌烘柊琛�1锟�7
    if (m_ScrollBuffer != 0)
    {
        ++m_CurrentLine;
        ++m_TotalLinesWritten;

        unsigned int newBufferLine = m_CurrentLine % BUFFER_LINES;
        unsigned int newBufferOffset = newBufferLine * COLUMNS;
        for (unsigned int col = 0; col < COLUMNS; ++col)
            m_ScrollBuffer[newBufferOffset + col] = ' ' | CRT::COLOR;
    }

    // 2. 灞忓箷婊氬姩锛堜粎鍦ㄦ湭鏌ョ湅鍘嗗彶鏃讹級
    if (m_CursorY >= ROWS)
    {
        if (m_ScrollBuffer != 0 && m_ScrollOffset == 0)
        {
            // 鍚戜笂婊氬姩灞忓箷鍐呭
            for (unsigned int row = 0; row < ROWS - 1; ++row)
            {
                for (unsigned int col = 0; col < COLUMNS; ++col)
                {
                    m_VideoMemory[row * COLUMNS + col] =
                        m_VideoMemory[(row + 1) * COLUMNS + col];
                }
            }

            // 娓呯┖鏈拷鍚庝竴琛�1锟�7
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
// 閫拷鏍�1锟�7
//======================================================================
void CRT::BackSpace()
{
    m_CursorX--;

    /* 绉诲姩鍏夋爣锛屽鏋滆鍥炲埌涓婁竴琛岀殑锟� */
    if (m_CursorX >= (unsigned int)CRT::COLUMNS) // 鐢变簬锟� unsigned锛岃闃叉涓嬫孩
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

    /* 鍦ㄥ厜鏍囨墍鍦ㄤ綅缃～涓婄┖锟� */
    m_VideoMemory[m_CursorY * COLUMNS + m_CursorX] = ' ' | CRT::COLOR;

    if (m_ScrollBuffer != 0)
    {
        unsigned int bufferLine = m_CurrentLine % BUFFER_LINES;
        unsigned int bufferOffset = bufferLine * COLUMNS + m_CursorX;
        m_ScrollBuffer[bufferOffset] = ' ' | CRT::COLOR;
    }
}

//======================================================================
// 鍒惰〃绗�1锟�7
//======================================================================
void CRT::Tab()
{
    m_CursorX &= 0xFFFFFFF8;	/* 鍚戝乏瀵归綈鍒板墠涓拷涓猅ab杈圭晫 */
    m_CursorX += 8;

    if (m_CursorX >= CRT::COLUMNS)
        NextLine();
    else
        MoveCursor(m_CursorX, m_CursorY);
}

//======================================================================
// 鍐欏瓧绗︼細鍚屾椂鍐欏叆婊氬姩缂撳啿鍖轰笌褰撳墠灞忓箷
//======================================================================
void CRT::WriteChar(char ch)
{
    // 1. 浼樺厛鍐欑紦鍐插尯锛堢‘淇濇暟鎹笉涓㈠け锛�1锟�7
    if (m_ScrollBuffer != 0)
    {
        unsigned int bufferLine = m_CurrentLine % BUFFER_LINES;
        unsigned int bufferOffset = bufferLine * COLUMNS + m_CursorX;
        m_ScrollBuffer[bufferOffset] = (unsigned char)ch | CRT::COLOR;
    }

    // 2. 鍙湁鍦ㄦ湭婊氬姩鏃舵墠鍐欏睆骞曪紙鏌ョ湅鍘嗗彶鏃朵笉鐮村潖鏄剧ず锛�1锟�7
    if (m_ScrollOffset == 0)
    {
        m_VideoMemory[m_CursorY * COLUMNS + m_CursorX] =
            (unsigned char)ch | CRT::COLOR;
    }

    // 3. 鏇存柊鍏夋爣
    ++m_CursorX;
    if (m_CursorX >= COLUMNS)
        NextLine();
    else if (m_ScrollOffset == 0)
        MoveCursor(m_CursorX, m_CursorY);
}

//======================================================================
// 娓呭睆锛堝彧娓呭綋鍓嶅睆骞曪紝涓嶅姩婊氬姩缂撳啿鍖猴級
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
// 鍒濆鍖栨粴鍔ㄧ紦鍐插尯锛堝湪绯荤粺鍚姩闃舵璋冪敤锛�1锟�7
//======================================================================
void CRT::InitScrollBuffer()
{
    static unsigned short scrollBufferMemory[BUFFER_LINES * COLUMNS];
    m_ScrollBuffer = scrollBufferMemory;

    // 1. 鍒濆鍖栦负绌烘牸+榛樿棰滆壊
    for (unsigned int i = 0; i < BUFFER_LINES * COLUMNS; ++i)
        m_ScrollBuffer[i] = ' ' | CRT::COLOR;

    // 2. 淇濆瓨褰撳墠灞忓箷鍐呭锛堢郴缁熷惎鍔ㄨ緭鍑猴級
    for (unsigned int row = 0; row < ROWS; ++row)
    {
        for (unsigned int col = 0; col < COLUMNS; ++col)
        {
            m_ScrollBuffer[row * COLUMNS + col] =
                m_VideoMemory[row * COLUMNS + col];
        }
    }

    // 3. 鏇根嵁鍏夋爣浣嶇疆鍒濆鍖栬鍙1锟7
    m_CurrentLine = m_CursorY;
    m_TotalLinesWritten = m_CursorY + 1;
    m_ScrollOffset = 0;
    m_IsScrolling = false;

    DrawScrollBar();
}

//======================================================================
// 浠庣紦鍐插尯璇讳竴琛屽埌灞忓箷鎸囧畾琛�1锟�7
//======================================================================
void CRT::ReadLineFromBuffer(unsigned int absLine, unsigned int screenRow)
{
    unsigned int physLine = absLine % BUFFER_LINES;
    unsigned int offset = physLine * COLUMNS;
    for (unsigned int col = 0; col < COLUMNS; ++col)
        m_VideoMemory[screenRow * COLUMNS + col] = m_ScrollBuffer[offset + col];
}

//======================================================================
// 鍒锋柊灞忓箷鏄剧ず锛屾牴鎹�1锟�7 m_ScrollOffset 閫夋嫨绐楀彛
//======================================================================
void CRT::RefreshScreen()
{
    if (m_ScrollBuffer == 0 || m_TotalLinesWritten == 0)
        return;

    // 1. 璁＄畻璧峰琛屽彿
    // startLine = (m_CurrentLine + 1) - ROWS - m_ScrollOffset
    unsigned int startLine;
    if (m_CurrentLine + 1 >= ROWS + m_ScrollOffset)
        startLine = (m_CurrentLine + 1) - ROWS - m_ScrollOffset;
    else
        startLine = 0;

    // 2. 浠庣紦鍐插尯璇诲彇骞舵樉绀�1锟�7
    for (unsigned int screenRow = 0; screenRow < ROWS; ++screenRow)
    {
        unsigned int bufferLine = startLine + screenRow;

        if (bufferLine > m_CurrentLine)
        {
            // 瓒呭嚭褰撳墠琛岋紝鏄剧ず绌鸿
            for (unsigned int col = 0; col < COLUMNS; ++col)
                m_VideoMemory[screenRow * COLUMNS + col] = ' ' | CRT::COLOR;
        }
        else
        {
            ReadLineFromBuffer(bufferLine, screenRow);
        }
    }

    // 3. 鏇存柊鍏夋爣
    if (m_ScrollOffset == 0)
        MoveCursor(m_CursorX, m_CursorY);      // 鏈€鏂颁綅缃
    else
        MoveCursor(COLUMNS - 1, ROWS - 1);     // 鏌ョ湅鍘嗗彶鏃讹紝鎶婂厜鏍囨斁鍦ㄥ彸涓嬭

    DrawScrollBar();
}

//======================================================================
// 涓婃粴涓拷琛岋紙PageUp锛�1锟�7
//======================================================================
void CRT::ScrollUp()
{
    if (m_ScrollBuffer == 0 || m_TotalLinesWritten <= ROWS)
        return;

    // 璁＄畻鏈拷澶у亸绉婚噺
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
// 涓嬫粴涓拷琛岋紙PageDown锛�1锟�7
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
    // 娌℃湁缂撳啿鍖虹洿鎺ヨ繑鍥�1锟�7
    if (m_ScrollBuffer == 0)
        return;

    // 宸茬粡鍦ㄦ渶鏂颁綅缃氨涓嶇敤鍔�1锟�7
    if (m_ScrollOffset == 0)
        return;

    // 閫拷鍑哄巻鍙叉ā寮�1锟�7
    m_ScrollOffset = 0;
    m_IsScrolling = false;

    // 鐢1锟7 offset = 0 鍒锋柊灞忓箷锛屾鏃1锟7 RefreshScreen 浼氭樉绀烘渶鏂1锟7 ROWS 琛1锟7
    // 骞舵妸鍏夋爣绉诲洖 m_CursorX, m_CursorY锛堜綘涔嬪墠鐨勫疄鐜板凡缁忚繖鏍峰啓浜嗭級
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
