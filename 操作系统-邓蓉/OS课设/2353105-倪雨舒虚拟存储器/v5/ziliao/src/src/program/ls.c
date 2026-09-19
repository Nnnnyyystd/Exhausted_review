#include<file.h>
#include<sys.h>
#include "GetOptAndPath.h"
void permissions(unsigned short mode)
{  //Directory or not: 
   if((mode&0x4000)==0)
      printf("-");
   else
      printf("d");

   // permissions of file owner
   if((mode&0x0100)==0)
      printf("-");
   else
      printf("r");

   if((mode&0x0080)==0)
      printf("-");
   else
      printf("w");
   if((mode&0x0040)==0)
      printf("-");
   else
      printf("x");

   //permissions of users in the same group
   if((mode&0x0020)==0)
      printf("-");
   else
      printf("r");
   if((mode&0x0010)==0)
      printf("-");
   else
      printf("w");
   if((mode&0x0008)==0)
      printf("-");
   else
      printf("x");

   //permissions of other users
   if((mode&0x0004)==0)
      printf("-");
   else
      printf("r");
   if((mode&0x0002)==0)
      printf("-");
   else
      printf("w");
   if((mode&0x0001)==0)
      printf("-");
   else
      printf("x");
}

void usage()
{
     printf("Usage:ls [options] filename1 filename2 ...\n");
     printf("Options:\n");
     printf("-l:List details of the dir\n");
     printf("--help:show usage\n");
     
}

void main1(int argc, char **argv)
{
 int fd;
 struct st_inode inode;//Initially "struct st_inode *inode" is wrong!
 char item[32];//store directory infos. 
 char pathbuf[100];
 int i=0;
 int j=0;
 int count;
 int ret;

 struct option options[]=
{
 {"-l","",0,'l'},
 {"","--help",0,'h'}
};

int flag=0;
int flag_file=0;
int flagl=0;
int getret;
char optarg[10];
char path[10][100];
int *pathi;
int *optind;
int a=0;
int b=1;

pathi=&a;
optind=&b;

while((getret=GetOptAndPath(argc,argv,optind,pathi,optarg,path,options,2)))
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
       case 'l':
       flagl=1;
       break; 
   }
}

if(flag_file==0)
{
  *pathi = 1;
  getPath(path[0]);
  flag=0;
}

if(flag==1)
{
  printf("\n");
  usage();
  return;
}
 
     for(i=0;i<(*pathi);i++)    
     {
          
         fd=open(path[i],0111);
        
         if(fd==-1)
         {
           printf("Directory \'%s\' cannot be opened! Maybe it does not exist! \
                                    \n",path[i]);
              continue;
         }
   
         printf("Directory \'%s\':\n",path[i]);
     
         if(flagl!=1)
          {
               
               //Get directory infos:
               
                 
                   count=read(fd,item,32);
                   while(count!=0)                     
                         {  
                           if(((int) item[0])==0)//This is very important!
                          //When some dir has been deleted, their dir name may
                  //still exit in the item. The fact that the inode number
                  //of an directory is "0" indicates that the dir is invalid.
                            {
                                  
                             for(j=0;j<32;j++)
                             item[j]='\0';      
                             count=read(fd,item,32);
                            }    
                           else
                           {
                           printf("%s\t" ,item+4);
                     
                           for(j=0;j<32;j++)
                           item[j]='\0';      
                           count=read(fd,item,32);
                           }

                         }
                   printf("\n");
              
          }
          else
          {
                  printf("permission\tnlink\towner\tgroup\tsize\tname\n");
                  count=read(fd,item,32);
                  while(count!=0)
                  {    
                       if(((int)item[0])==0)
                       {

                           for(j=0;j<32;j++)
                           item[j]='\0';      
                           count=read(fd,item,32);
                        }          
                        //get current path:
                       else
                       {
                       strcpy(pathbuf,path[i]);//Combine parent path 
                                         //and current item path
                       strcat(pathbuf,"/");
                       strcat(pathbuf,item+4);//to get real path
                       
                      //get inode infos:
                       ret=stat(pathbuf,&inode);
                       if(ret==-1)
                       {
                                  printf("Cannot read inode!\n");
                                  return;
                        }
 
                       permissions(inode.st_mode);
                       printf("\t");
                       printf("%d\t",inode.st_nlink);
                       printf("root\t");
                       printf("root\t");
                       printf("%d\t",inode.st_size);
                       printf("%s\n",item+4);
                       
 

                       for(j=0;j<32;j++)
                           item[j]='\0';
                       count=read(fd,item,32);
                     }
                 }//while
             }
     
       close(fd);

      }//for

 
}
