[BITS 32]
[extern _main0]

[extern __main]  	;"_main()"定义在support.c中
[extern __atexit]	; "_atexit()"定义在support.c中

global greatstart
greatstart:
	mov eax,1
	mov eax,2
	mov eax,3

;Makefile中g++选项 -nostartfiles禁止了g++去链接startup code,
;startup code即是在进入我们用C++编写的main0()函数之前，以及main0()
;退出时执行的代码，其执行的工作是初始化(/销毁)global/static对象。
	call __main		;call our own startup code
	jmp _main0
	call __atexit  	;call our own startup code
