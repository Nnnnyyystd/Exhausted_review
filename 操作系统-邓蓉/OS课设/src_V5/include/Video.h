// Video.h
#ifndef DIAGNOSE_H
#define DIAGNOSE_H

class Diagnose
{
public:
	static unsigned int ROWS;

	/* static const member */
	static const unsigned int COLUMNS = 80;
	static const unsigned short COLOR = 0x0B00; /* char in bright CYAN */
	static const unsigned int SCREEN_ROWS = 25; /* full screen rows */

public:
	Diagnose();
	~Diagnose();

	static void TraceOn();
	static void TraceOff();

	static void Write(const char *fmt, ...);
	static void ClearScreen();

private:
	static void PrintInt(unsigned int value, int base);
	static void NextLine();
	static void WriteChar(const char ch);

public:
	static unsigned int m_Row;
	static unsigned int m_Column;

private:
	static unsigned short *m_VideoMemory;
	/* DebugÊä³ö¿ª¹Ø */
	static bool trace_on;
};

#endif
