#include "Video.h"

unsigned short *Diagnose::m_VideoMemory = (unsigned short *)(0xB8000 + 0xC0000000);
unsigned int Diagnose::m_Row = 10;
unsigned int Diagnose::m_Column = 0;

unsigned int Diagnose::ROWS = 10;

bool Diagnose::trace_on = true;

Diagnose::Diagnose()
{
	// 全部都是static成员变量，所以没有什么需要在构造函数中初始化的。
}

Diagnose::~Diagnose()
{
	// this is an empty dtor
}

void Diagnose::TraceOn()
{
	Diagnose::trace_on = 1;
}

void Diagnose::TraceOff()
{
	Diagnose::trace_on = 0;
}

/*
	能够输出格式化后的字符串，目前只能识别一些%d %x  %s 和%n;
	没有检查错误功能，% 和 值匹配要自己注意。
*/
void Diagnose::Write(const char *fmt, ...)
{
	if (false == Diagnose::trace_on)
	{
		return;
	}
	// 使va_arg中存放参数fmt的 “后一个参数” 所在的内存地址
	// fmt的内容本身是字符串的首地址(这不是我们要的)，而&fmt + 1则是下一个参数的地址
	// 参考UNIX v6中的函数prf.c/printf(fmt, x1,x2,x3,x4,x5,x6,x7,x8,x9,xa,xb,xc)
	unsigned int *va_arg = (unsigned int *)&fmt + 1;
	const char *ch = fmt;

	while (1)
	{
		while (*ch != '%' && *ch != '\n')
		{
			if (*ch == '\0')
				return;
			if (*ch == '\n')
				break;
			/*注意： '\n'是一个单一字符，而不是'\\'和 ‘n'两个字符的相加，
			譬如在字符串"\nHello World!!"中如果比较 if(*ch == '\\' && *(ch+1) == '\n' ) 的话，
			会死的狠惨的！*/
			WriteChar(*ch++);
		}

		ch++; // skip the '%' or '\n'

		if (*ch == 'd' || *ch == 'x')
		{ //%d 或 %x 格式来输出，当然要添加八进制和二进制也很容易，但用处不大。
			int value = (int)(*va_arg);
			va_arg++;
			if (*ch == 'x')
				Write("0x"); // as prefix for HEX value
			PrintInt(value, *ch == 'd' ? 10 : 16);
			ch++; // skip the 'd' or 'x'
		}

		else if (*ch == 's')
		{		  //%s 格式来输出
			ch++; // skip the 's'
			char *str = (char *)(*va_arg);
			va_arg++;
			while (char tmp = *str++)
			{
				WriteChar(tmp);
			}
		}
		else /* if(*(ch-1) == '\n') */
		{
			Diagnose::NextLine();
		}
	}
}

/*
	参考UNIX v6中的函数prf.c/printn(n,b)
	此函数的功能是将一个值value以base进制的方式显示出来。
*/
void Diagnose::PrintInt(unsigned int value, int base)
{
	// 因为数字0～9 和 A~F的ASCII码之间不是连续的，所以不能简单通过
	// ASCII(i) = i + '0'直接计算得到，因此用了Digits字符数组。
	static char Digits[] = "0123456789ABCDEF";
	int i;

	if ((i = value / base) != 0)
		PrintInt(i, base);
	WriteChar(Digits[value % base]);
}

void Diagnose::NextLine()
{
	m_Row += 1;
	m_Column = 0;
}

void Diagnose::WriteChar(const char ch)
{
	if (Diagnose::m_Column >= Diagnose::COLUMNS)
	{
		NextLine();
	}

	if (Diagnose::m_Row >= Diagnose::SCREEN_ROWS)
	{
		Diagnose::ClearScreen();
	}

	Diagnose::m_VideoMemory[Diagnose::m_Row * COLUMNS + Diagnose::m_Column] = (unsigned char)ch | Diagnose::COLOR;
	Diagnose::m_Column++;
}

void Diagnose::ClearScreen()
{
	unsigned int i;

	Diagnose::m_Row = Diagnose::SCREEN_ROWS - Diagnose::ROWS;
	Diagnose::m_Column = 0;

	for (i = 0; i < (COLUMNS * ROWS); i++)
	{
		Diagnose::m_VideoMemory[i + m_Row * COLUMNS] = (unsigned char)' ' | Diagnose::COLOR;
	}
}
