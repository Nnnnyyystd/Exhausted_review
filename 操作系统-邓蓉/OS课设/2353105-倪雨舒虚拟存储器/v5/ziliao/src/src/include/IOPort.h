#ifndef IOPORT_H
#define IOPORT_H


/*
	创建时间：3:09 PM 2008-8-9
	说明：定义了对IN,OUT指令的抽象
		使用了C++ Inline Assembly 封装了IN,OUT指令，
		用于对CHIPS以及外设reprogram时候端口的读写和数据传送。
*/

/*
	有2种可供选择的实现方案： 
	(1) 使用class IOPort对IN,OUT指令进行封装，IN、OUT实现为static成员函数。
	(2) 使用namespace IOPort对IN,OUT指令进行封装。
	
	上述两种方案的区别在于：  调用时语法不同。
	方案一：  class IOPort 的实现方式
	调用时：  IOPort::IN8(unsigned short port);        
	
	方案二：  namespace IOPort的实现方式
	调用时：  在没有声明using namespace IOPort; 情况下， IOPort::IN8(unsigned short port);
			声明了using namespace IOPort;之后， 直接IN8(unsigned short port);   
			
	注：	两种方案的实现代码下面都有，反正两种方案实现代码除了class和namespace关键字的区别，
		namespace实现方案函数前没有static关键字之外，没其它区别。
		目前enable了方案一， 方案二的代码被注释掉了。
*/

/*
	关于C++ Inline Assembly，参考“AT&T 汇编 (AT&T ASM) 参考 ” 
	@ http://blog.csdn.net/robin_qiu/archive/2006/03/02/613904.aspx
	不过这两天这个网页我打不开了，还好网页我存下来了。
*/

/*方案一：*/
class IOPort
{	
	//由于函数都是Inline Assembly的原因，函数定义写道.cpp文件中好像会有问题。
	//所以干脆把函数声明、定义一起写道class IOPort的声明里面算了。
	
	public:
		//从指定端口port读入8比特数据，放入data中作为返回值
		static inline unsigned char InByte(unsigned short port)
		{
			unsigned char data;
			__asm__ __volatile__("inb %%dx, %%al"
						:"=a" (data)
						:"d" (port) );
			return data;
		}
		
		//从指定端口port读入16比特数据，放入data中作为返回值
		static inline unsigned short InWord(unsigned short port)
		{
			unsigned short data;
			__asm__ __volatile__("inw %%dx, %%ax"
						:"=a" (data)
						:"d"	(port)	);
			return data;
		}
		
		//从指定端口port读入32比特数据，放入data中作为返回值
		static inline unsigned int InDWord(unsigned short port)
		{
			unsigned int data;
			__asm__ __volatile__("inl %%dx, %%eax"
						:"=a" (data)
						:"d" (port)	);
			return data;
		}
	//=====================================================================
	//以上是对IN的封装，以下是对OUT指令的封装
		
		//将8比特数据data，写入到指定端口port中
		static inline void OutByte(unsigned short port, unsigned char data)
		{
			__asm__ __volatile__("outb %%al, %%dx"
						:: "d" (port), "a"(data)	);
		}
		
		//将16比特数据data，写入到指定端口port中
		static inline void OutWord(unsigned short port, unsigned short data)
		{
			__asm__ __volatile__("outw %%ax, %%dx"
						:: "d" (port), "a"(data)	);
		}
		
		//将32比特数据data，写入到指定端口port中
		static inline void OutDWord(unsigned short port, unsigned int data)
		{
			__asm__ __volatile__("outl %%eax, %%dx"
						:: "d" (port), "a"(data)	);
		}
		
}; // end of class IOPort declearation

/*end of 方案一 */ 

/*方案二:
namespace IOPort{
		
		//从指定端口port读入8比特数据，放入data中作为返回值
		inline unsigned char in8(unsigned short port)
		{
			unsigned char data;
			__asm__ __volatile__("inb %%dx, %%al"
						:"=a" (data)
						:"d" (port) );
			return data;
		}
		
		//从指定端口port读入16比特数据，放入data中作为返回值
		inline unsigned short in16(unsigned short port)
		{
			unsigned short data;
			__asm__ __volatile__("inw %%dx, %%ax"
						:"=a" (data)
						:"d"	(port)	);
			return data;
		}
		
		//从指定端口port读入32比特数据，放入data中作为返回值
		inline unsigned int in32(unsigned short port)
		{
			unsigned int data;
			__asm__ __volatile__("inl %%dx, %%eax"
						:"=a" (data)
						:"d" (port)	);
			return data;
		}
	//=====================================================================
	//以上是对IN的封装，以下是对OUT指令的封装
		
		//将8比特数据data，写入到指定端口port中
		inline void out8(unsigned short port, unsigned char data)
		{
			__asm__ __volatile__("outb %%al, %%dx" 
						:: "d" (port), "a"(data)	);
		}
		
		//将16比特数据data，写入到指定端口port中
		inline void out16(unsigned short port, unsigned short data)
		{
			__asm__ __volatile__("outw %%ax, %%dx"
						:: "d" (port), "a"(data)	);
		}
		
		//将32比特数据data，写入到指定端口port中
		inline void out32(unsigned short port, unsigned int data)
		{
			__asm__ __volatile__("outl %%eax, %%dx"
						:: "d" (port), "a"(data)	);
		}
}//end of IOPort namespace

end of 方案二 */ 

#endif
