/*Link.ld*/
OUTPUT_FORMAT("pei-i386")

ENTRY(greatstart)

SECTIONS
{
	.text 0xc0100000:
	{
		code = .; _code = .; __code = .;
		*(.text)
		. = ALIGN(4096);
	}
	
	.data :
	{
		__CTOR_LIST__ = .;
			
		*(.ctors) 
		LONG(0)
		__CTOR_END__ = .;
		
		__DTOR_LIST__ = .;
		/*LONG((__DTOR_END__ - __DTOR_LIST__) /4 - 2)*/
		*(.dtors)
		LONG(0)
		__DTOR_END__ = .;
		
		data = .; _data = .; __data = .;
		*(.data)
		. = ALIGN(4096);
	}
	
	.bss :
	{
		bss = .; _bss = .; __bss = .;
		*(.bss)
		. = ALIGN(4096);
	}
	
	end = .; _end =.; __end = .;
}
