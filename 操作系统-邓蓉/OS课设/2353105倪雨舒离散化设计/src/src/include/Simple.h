//Simple.h

#ifndef SIMPLE_H
#define	SIMPLE_H

#include "Video.h"

class Simple
{
public:
	Simple();
	~Simple();

private:
	unsigned int x;
	unsigned int y;

public:	
	/*ÏÂÃæÊÔÊÔstatic data member*/
	static int size;
	static char tst[50];
};

#endif
