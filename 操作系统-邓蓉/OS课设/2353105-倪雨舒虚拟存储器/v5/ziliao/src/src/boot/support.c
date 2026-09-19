//Support.c

/*
函数名前需加上 extern “C”，虽然本文件名support.c是.c后缀的文件，
文件内容也是pure C code，让是因为Makefile里面对support.c编译用的是
g++，而不是gcc，所以函数名前面还是要加上extern “C”，否则会报
“undefined reference to __main()”类似的错误。
*/
extern "C" void _main()
{
	extern void (*_CTOR_LIST__)();   
	/*Note： 此处使用_CTOR_LIST__，变量名prefix 一个 ’_‘   
			而link.ld中要prefix 两个 ’_‘*/
	
	void (**constructor)() = &_CTOR_LIST__;

	//the first element is the number of constructors
	int total = *(int *)constructor;
	
	//constructor++;   
		/*  (可以先看一下链接脚本：Link.ld)
		Link script中修改过后，这里的total已经不是constructor的个数了，
		_CTOR_LIST__的第一个单元开始就是global/static对象的constructor，
		所以不用 constructor++; 
		*/
	
	while(total) //total不是constructor的数量，而是用于检测是否到了_CTOR_LIST__的末尾
	{
		(*constructor)();
		//total--;
		constructor++;
		total = *(int *)constructor;
	}
}

extern "C" void _atexit()
{
	extern void (*_DTOR_LIST__)();
	
	void (**deconstructor)() = &_DTOR_LIST__;
	
	int total = *(int *)deconstructor;
	
	deconstructor++;
	
	while(total)
	{
		(*deconstructor)();
		//total--;
		deconstructor++;
		total = *(int *)deconstructor;
	}
}
