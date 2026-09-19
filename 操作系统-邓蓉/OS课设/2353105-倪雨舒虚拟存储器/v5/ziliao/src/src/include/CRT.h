#ifndef CRT_H
#define CRT_H

#include "TTy.h"

class CRT
{
	/* Const Member */
public:
	/* 鏄剧ず鎺у埗瀵勫瓨鍣↖/O绔彛鍦板潃 */
	static const unsigned short VIDEO_ADDR_PORT = 0x3d4;	/* 鏄剧ず鎺у埗绱㈠紩瀵勫瓨鍣ㄧ鍙ｅ彿 */
	static const unsigned short VIDEO_DATA_PORT = 0x3d5;	/* 鏄剧ず鎺у埗鏁版嵁瀵勫瓨鍣ㄧ鍙ｅ彿 */
	
	/* 灞忓箷澶у皬涓�80 * 25 */
	static const unsigned int COLUMNS = 80;
	static unsigned int ROWS;
	
	static const unsigned short COLOR = 0x0F00;		/* char in white color */
	// ========== 婊氬姩缂撳啿鍖� ==========
	static unsigned short* m_ScrollBuffer;       // 鎸囧悜闈欐€佺紦鍐插尯
	static const unsigned int BUFFER_LINES = 1000;

	// ========== 琛屽彿鍙婄粺璁� ==========
	static unsigned int m_CurrentLine;           // 褰撳墠鍐欏叆鐨勮鍙凤紙缁濆鍊硷級
	static unsigned int m_TotalLinesWritten;     // 宸插啓鍏ョ殑鎬昏鏁�

	// ========== 婊氬姩鎺у埗 ==========
	static unsigned int m_ScrollOffset;          // 婊氬姩鍋忕Щ锛�0 琛ㄧず鏄剧ず鏈€鏂帮級
	static bool m_IsScrolling;                   // 鏄惁姝ｅ湪鏌ョ湅鍘嗗彶


	/* Functions */
public:
	/* 灏嗚緭鍑虹紦瀛橀槦鍒椾腑鐨勫唴瀹硅緭鍑哄埌灞忓箷涓� */
	static void CRTStart(TTy* pTTy);

	/* 鏀瑰彉鍏夋爣浣嶇疆 */
	static void MoveCursor(unsigned int x, unsigned int y);

	/* 鎹㈣澶勭悊瀛愮▼搴� */
	static void NextLine();

	/* 閫€鏍煎鐞嗗瓙绋嬪簭 */
	static void BackSpace();
	
	/* Tab澶勭悊瀛愮▼搴� */
	static void Tab();

	/* 鏄剧ず鍗曚釜瀛楃 */
	static void WriteChar(char ch);

	/* 娓呴櫎灞忓箷 */
	static void ClearScreen();

	/* Members */
	static void InitScrollBuffer();
	static void ScrollUp();
	static void ScrollDown();
	static void RefreshScreen();
	static void ReadLineFromBuffer(unsigned int absLine, unsigned int screenRow);
	static void GoToLatestLine();   // 鏂板锛氶€€鍑烘粴鍔ㄦā寮忥紝鍥炲埌搴曢儴
	
	/* 绘制滚动条 */
	static void DrawScrollBar();

public:
	static unsigned short* m_VideoMemory;
	static unsigned int m_CursorX;
	static unsigned int m_CursorY;

	/* 鎸囧悜杈撳嚭缂撳瓨闃熷垪涓綋鍓嶈杈撳嚭鐨勫瓧绗� */
	static char* m_Position;
	/* 鎸囧悜杈撳嚭缂撳瓨闃熷垪涓湭纭鐨勮緭鍑哄瓧绗︾殑寮€濮嬪锛屽嵆鍙互閫氳繃Backspace閿垹闄ょ殑鍐呭锛�
	 * 鏈€鍚庝竴涓洖杞︿箣鍓嶇殑鍐呭涓哄凡纭鍐呭锛屼笉鍙Backspace閿垹闄ゃ€�
	 */
	static char* m_BeginChar;
};

#endif
