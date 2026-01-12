#ifndef COMMANDTREE_H
#define	COMMANDTREE_H

/* enum command type */
#define	TLST	0x01
#define	TFIL	0x02
#define TPAR	0x04
#define	TCOM	0x08

/* enmu command params */
#define FCAT	0x01
#define FPIN	0x02
#define FPOU	0x04
#define FAND	0x08
#define FPAR	0x10

struct commandNode
{
	unsigned int commandType;
	unsigned int params;
	int left;
	int right;

	/* command infos */
	char* commandName;
	char* fin;
	char* fout;
	char* args[10];
};

extern struct commandNode commandNodes[50];
extern int curNode;

void InitCommandTree();
int GetNextFreeCommandNode();

/* 
*建立不同命令节点函数
* 参数：stArg:Start pos in args[], edArg: End pos in args[], midArg: Key token pos, only for pipe(|) and & and ; Nodes 
* 返回：节点在commandNodes[] 中的位置
*
*/
int CreateSimpleNode( int stArg, int edArg, int params);
int CreateCurveNode( int stArg, int edArg, int params);
int CreatePipeNode( int stArg, int edArg, int midArg, int params);
int CreateTableNode( int stArg, int edArg, int midArg, int params);
/*
* 分析命令树
* 返回命令数根节点
*/
int AnalizeCommand( int stArg, int edArg, int params);

#endif
