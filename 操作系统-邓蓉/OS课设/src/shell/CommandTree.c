#include "CommandTree.h"
#include "PreExecute.h"
#include "globe.h"
#include "string.h"

struct commandNode commandNodes[50];
int curNode;
char* getFilename( char* pathname)
{
	int len = strlen(pathname);
	char* cp = &pathname[len - 1];
	while( cp >= pathname && *cp != '/' ) cp--;
	return ++cp;
}
void InitCommandTree()
{
	int i = 0;
	curNode = 0;
	for ( i = 0; i < 50; i++ )
	{
		commandNodes[i].commandName = 0;
		commandNodes[i].commandType = 0;
		commandNodes[i].fin = 0;
		commandNodes[i].fout = 0;
		commandNodes[i].left = -1;
		commandNodes[i].right = -1;
		commandNodes[i].params = 0;
	}
}
int GetNextFreeCommandNode()
{
	return curNode++;
}

int CreateSimpleNode( int stArg, int edArg, int params )
{
	int nodeNumber = GetNextFreeCommandNode();
	struct commandNode* pnode = &commandNodes[nodeNumber];

	int argCnt = 0;

	pnode->fin = 0;
	pnode->fout = 0;
	pnode->commandType = TCOM;
	if ( (params & FPAR) > 0 ) 
	{
		pnode->params |= FPAR;
	}
	if ( (params & FAND ) > 0 )
	{
		pnode->params |= FAND;
	}
	pnode->commandName = args[stArg++];
	pnode->args[argCnt++] = getFilename(pnode->commandName);

	while ( stArg <= edArg )
	{
		char ch = args[stArg][0];
		if ( args[stArg][1] == 0 )
		{
			switch ( ch )
			{
				case '<':
					if ( stArg + 1 <= edArg && !pnode->fin)
					{
						pnode->fin = args[stArg+1];
						stArg++;
					}
					break;
				case '>':
					if ( stArg + 2 <= edArg && !pnode->fout )
					{
						if ( args[stArg + 1][0] == '>' && args[stArg + 1][1] == 0	)
						{
							pnode->fout = args[stArg + 2] ;
							pnode->params |= FCAT;
							stArg += 2;
						}
					}
					else if ( stArg + 1 <= edArg && !pnode->fout )
					{
						pnode->fout = args[stArg + 1];
						stArg++;
					}
					break;
				default:
					pnode->args[argCnt++] = args[stArg];
			}
		}
		else
		{
			pnode->args[argCnt++] = args[stArg];
		}
		stArg++;
	}
	pnode->args[ argCnt ] = 0;
	return nodeNumber;
}
int CreateCurveNode( int stArg, int edArg, int params )
{
	int nodeNumber = GetNextFreeCommandNode();
	struct commandNode* pnode = &commandNodes[nodeNumber];
	int lastArg = edArg;
	pnode->commandType = TPAR;
	pnode->fin = 0;
	pnode->fout = 0;
	while ( !(args[edArg][0] == ')' && args[edArg][1] == 0 ) ) edArg--;
	if ( edArg < lastArg ) /* has some output redirect params */
	{
		int firstArg = edArg + 1;
		while( firstArg <= lastArg )
		{
			char ch = args[firstArg][0];
			if ( args[firstArg][1] == 0 )
			{
				switch ( ch )
				{
				case '<':
					if ( firstArg + 1 <= lastArg )
					{
						pnode->fin = pnode->fin == 0 ? args[firstArg + 1] : pnode->fin;
					}
					break;
				case '>':
					if ( firstArg + 2 <= lastArg )
					{
						if ( args[firstArg + 1][0] == '>' && args[firstArg + 1][1] == 0	)
						{
							pnode->fout = pnode->fout == 0 ? args[firstArg + 2] : pnode->fout;
							pnode->params |= FCAT;
						}
					}
					else if ( firstArg + 1 <= lastArg )
					{
						pnode->fout = pnode->fout == 0 ? args[firstArg + 1] : pnode->fout;
					}
					break;
				}
			}
			firstArg++;
		}		
	}
	pnode->left = AnalizeCommand(stArg + 1, edArg - 1, FPAR );
	return nodeNumber;
}

int CreatePipeNode( int stArg, int edArg, int midArg, int params )
{
	int nodeNumber = GetNextFreeCommandNode();
	struct commandNode* pnode = &commandNodes[nodeNumber];
	pnode->commandType = TFIL;
	pnode->left = AnalizeCommand( stArg, midArg - 1, 0 );
	pnode->right = AnalizeCommand( midArg + 1, edArg, params );
	return nodeNumber;
}

int CreateTableNode( int stArg, int edArg, int midArg, int params)
{
	int nodeNumber = GetNextFreeCommandNode();
	struct commandNode* pnode = &commandNodes[nodeNumber];
	pnode->commandType = TLST;
	if ( args[midArg][0] == '&' )
	{
		params |= FAND;
	}
	pnode->left = AnalizeCommand( stArg, midArg - 1, params );
	pnode->right = AnalizeCommand( midArg + 1, edArg, params & (~FAND) );
	return nodeNumber;
}

/* 返回-1表示失败或者不能建立 */
/* 所以在执行命令树时需要检查节点是否为-1 */
int AnalizeCommand( int stArg, int edArg, int params)
{
	int curArg;
	int curveCnt = 0;
	int leftCurve = edArg + 1;
	int rightCurve = edArg + 1;
	int firstPipe = edArg + 1;
	if ( stArg > edArg ) return -1; /* error */
	while( stArg <= edArg && args[stArg][1] == 0 && ( args[stArg][0] == '&' || args[stArg][0] == ';' || args[stArg][0] == '|' ) ) stArg++; /* remove them */
	curArg = stArg;
	
	/* find ; & */
	while ( curArg <= edArg )
	{
		char ch = args[curArg][0];
		if ( args[curArg][1] == 0 )
		{
			switch (ch)
			{
			case '&':
			case ';':
				if ( curveCnt == 0 )  return CreateTableNode( stArg, edArg, curArg, params );
				break;
			case '|':
				if ( curveCnt == 0 && curArg < firstPipe  ) firstPipe = curArg;
				break;
			case '(':
			case ')':
				if ( ch == '(' ) 
					curveCnt++;
				else 
					curveCnt--;
				break;
			}
		}
		curArg++;
	}

	if ( firstPipe <= edArg )
		return CreatePipeNode( stArg, edArg, firstPipe, params );
	if ( args[stArg][0] == '(' && curveCnt == 0 ) 
		return CreateCurveNode( stArg, edArg, params );
	return CreateSimpleNode( stArg, edArg, params );
}
