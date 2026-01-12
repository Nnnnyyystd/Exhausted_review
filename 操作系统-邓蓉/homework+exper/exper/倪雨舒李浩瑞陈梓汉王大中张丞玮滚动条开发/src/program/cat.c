#include<file.h>
#include "GetOptAndPath.h"
void display(char *path)
{ 
   char string[256];
   int fd;
   int i=0;
   fd=open(path,0111);
   if(fd==-1)
   {
      printf("Wrong file %s!\n",path);
   }
   else
   {
       	int rbytes = 0;
		int wbytes = 0;
		while ( rbytes = read(fd, string, 256) )
		{
			if ( rbytes < 0 )
			{
				printf("Read src file error...\n");
				break;
			}
			else
			{
				wbytes = write(STDOUT, string, rbytes);
			}
		}
	} 
	close(fd);
	return;
}


void usage()
{
     printf("Usage:cat [options] filename1 filename2 ...\n");
     printf("Options:\n");
     printf("--help:show usage\n");
     
}

void main1(int argc, char **argv)
{
struct option options[]=
{
 {"","--help",0,'h'}
};

int flag=0;
int flag_file=0;
int getret;
char optarg[10];
char path[10][100];
int *pathi;
int *optind;
int a=0;
int b=1;
int i=0;

pathi=&a;
optind=&b;
//rm_r_flag_fail=&val;


while((getret=GetOptAndPath(argc,argv,optind,pathi,optarg,path,options,1)))
{
   switch(getret)
   {
       case 1: 
       flag_file=1;
       break;
       case 2:
       printf("The option is not right.\n");
       flag=1;
       break;
       case 3:
       printf("Certain options lacks argument\n");
       flag=1;
       break;
       case 'h':
       flag=1;
       break;
   }
}

if(flag_file==0)
{
  printf("No path has been input!\n");
  flag=1;
}

if(argc==1)
{
   printf("Too few options!\n");
   flag=1;
}


if(flag==1)
{
  printf("\n");
  usage();
  return;
}
 
     for(i=0;i<(*pathi);i++)    
     {   
            display(path[i]);
     }
 }
