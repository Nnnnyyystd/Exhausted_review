#include "CommandTree.h"
#include "ExecuteCommand.h"
#include "PreExecute.h"
#include "globe.h"
#include "sys.h"
#include "stdio.h"
#include "string.h"
#define stdin 0
#define stdout 1

char curPath[50];

void ExecuteTLST( struct commandNode* node, struct commandNode* parent, int* pipes)
{
	if ( node->left >= 0 )
	{
		//似乎这里应该判断node节点是否有pin属性，以便复制给左子节点
		ExecuteCommand( &commandNodes[node->left], node, pipes);		
	}	
	if ( node->right >= 0 )
	{
		ExecuteCommand( &commandNodes[node->right], node, pipes);
	}
}
void ExecuteTFIL( struct commandNode* node, struct commandNode* parent, int* pipes)
{
	int newpipes[2];
	int result = pipe(newpipes);	
	if ( node->left > 0 )
	{
		struct commandNode* pleft = &commandNodes[node->left];
		pleft->params |= FPOU;
		if ( ( node->params & FPIN ) )
		{			
			pleft->params |= FPIN;
			pleft->params |= FAND;
			pipes[1] = newpipes[1]; 
			ExecuteCommand( pleft, node, pipes );
		}
		else
		{
			ExecuteCommand( pleft, node, newpipes );
		}
	}
	if ( node->right > 0)
	{
		struct commandNode* pright = &commandNodes[node->right];
		pright->params |= FPIN;		
		pright->params |= FAND;
		ExecuteCommand( pright, node, newpipes );
	}	
}

void ExecuteTPAR( struct commandNode* node, struct commandNode* parent, int* pipes)
{
	if ( node->left > 0 )
	{
		struct commandNode* ppnode = &commandNodes[node->left];
		int state;
		/*printf("ExecuteTPAR() Begin fork\n");*/
		if ( fork() )
		{
			wait(&state);
			/*printf("parent: pid: %d\n", getpid());*/
		}
		else
		{
			/*printf("ExecuteTPAR()\n");*/
			/*printf("child: pid: %d\n", getpid());*/
			OutputRedirect( node, pipes );
			ExecuteCommand( ppnode, node, pipes );
			exit(0);
		}
	}
}

void ExecuteTCOM( struct commandNode* node, struct commandNode* parent, int* pipes)
{
	int state;
	if ( strcmp(node->commandName, "cd" ) == 0 )
	{
		if ( argsCnt == 1 )
		{
			printf("%s\n", curPath);
		}
		else if ( argsCnt == 2 )
		{			
        	if ( chdir((node->args[1])) == -1 )
			{            
				printf("Invalid path!\n");
			}
			getPath( curPath );
		}
		else
		{
			printf("Two many arguments for cd command!\n");
		}
		return;
	}

	int child = fork();
	int dead = -1;
	if ( child != 0 ) /* parent */
	{		
		if ( (node->params & FAND) == 0 ) /* need wait */
		{			
			while( wait(&state)!= child);
			//wait(&state);
		}
	}
	else
	{
		/* test first */
		char pathname[100];
		//printf("commandName:%s\n", node->commandName);
		OutputRedirect( node, pipes );
		
		if (-1 != execv( node->args[0], node->args) )
		{
			exit(0);
		}
		/* 搜索bin目录 */
		pathname[0] = 0;
		strcat(pathname, "/bin/");
		strcat(pathname, node->args[0]);
		//exec((unsigned int)pathname);
		if(-1 == execv( pathname, node->args) )
		{
			printf("\'%s\' is not an exist command or may not in this folder!\n", node->args[0]);
		}
		//printf("exit\n");
		exit(0);
	}
}

void ExecuteCommand( struct commandNode* node, struct commandNode* parent, int* pipes)
{
	switch( node->commandType )
	{
	case TLST:
		//printf("ExcuteCommand() case TLST\n");
		ExecuteTLST( node, parent, pipes);
		break;
	case TFIL:
		//printf("ExcuteCommand() case TFIL\n");
		ExecuteTFIL( node, parent, pipes);
		break;
	case TPAR:
		//printf("ExcuteCommand() case TPAR\n");
		ExecuteTPAR( node, parent, pipes);
		break;
	case TCOM:
		//printf("ExcuteCommand() case TCOM\n");
		ExecuteTCOM( node, parent, pipes);
		break;
	}
}

void OutputRedirect(struct commandNode* node, int* pipes)
{
	/*printf("node: fin: %d, fout: %d, params: %d\n, pipes: %d\n", node->fin, node->fout, node->params, pipes);*/
	if ( node->fin != 0 )
	{
		int fin = open( node->fin, 0 );
		if ( fin > 0 ) /* open is ok */
		{
			close(stdin);
			dup(fin);
			close(fin); /* close stdin */
		}
		else
		{
			printf("%s can't be open.\n", node->fin);
		}
	}
	if ( node->fout != 0 )
	{
		int fout = creat( node->fout, 0x1ff );
		if ( fout > 0 )
		{
			if ( ( node->params & FCAT ) )
			{
				/* move to eof */
			}
			close(stdout); /* close stdout */
			dup(fout);
			close(fout);
		}
		else
		{
			printf("%s can't be open/create.\n", node->fout);
		}
	}
	if ( ( node->params & FPIN ) > 0 )
	{
		close(stdin);
		dup(pipes[0]);
		close(pipes[0]);
		pipes[0] = -1;		
	}
	if ( ( node->params & FPOU ) > 0 )
	{
		close(stdout);
		dup(pipes[1]);
		close(pipes[1]);
		pipes[1] = -1;
	}
	if ( pipes )
	{
		if ( pipes[0] >= 0 ) close(pipes[0]);
		if ( pipes[1] >= 0 ) close(pipes[1]);
	}
	/*printf("End OutputRedirect()\n");*/
}
