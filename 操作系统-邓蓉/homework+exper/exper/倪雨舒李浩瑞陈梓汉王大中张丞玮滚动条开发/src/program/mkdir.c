#include<file.h>
#include"GetOptAndPath.h" 

void usage()
{
   printf("Usage:mkdir [options] direcroty\n");
   printf("Create the directories, if they do not exist.\n");
   printf("Options:\n");
   printf("-m or --mode:set permissio mode;\n");
   printf("-p or --parent:no error existing, make parent node as needed\n");
   printf("--verbose: printf a message for each created directory\n");
   printf("--help: show help about this command\n");
    return;
};

int convert(char string[10],int *newmode)
{
     if(strlen(string)!=3|| \
       string[0]<'0'||string[0]>'7'||\
       string[1]<'0'||string[1]>'7'||  \
        string[2]<'0'||string[1]>'7')
     {
          printf("The mode is wrong.\n");
          return 1;
     }

     else
     {//mode:0100 000x xxxx xxxx,last 9 bits indicates the permissions.
       (*newmode)=4*8*8*8*8+(string[0]-48)*64+(string[1]-48)*8+(string[2]-48)*1;
         return 0;
     }
};

void main1(int argc, char **argv)
{
  struct option options[]=
 {
   {"-m","--mode",1,'m'},
   {"-p","--parent",0,'p'},
   {"","--help",0,'h'},
   {"","--verbose",0,'v'}
 };

int flag=0;//If this flag is 1, it indicates that something is wrong.
int flag_file=0;
int flagv=0;
int flagp=0;
int flagh=0;
int flagm=0;
int flag_fail=0;
int getret;
int i;
int defaultmode=040755;//default newmode:- rwx r-w r-w
int *newmode;
char optarg[10];
char path[10][100];
int *pathi;
int *optind;
int a=0;
int b=1;

pathi=&a;
optind=&b;
newmode=&defaultmode;//newmode is default mode initially.


while((getret=GetOptAndPath(argc,argv,optind,pathi,optarg,\
              path,options,4)))
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
             printf("Certain option lacks argument.\n");
             flag=1;
             break;
             case 'm':
             flag=convert(optarg,newmode);
             flagm=1;
             break;
             case 'p':
             flagp=1;
             break;
             case 'v':
             flagv=1;
             break;
             case 'h':
             flagh=1; 
             break;
                
             
      }      


}

 //  printf("flagp:%d\n",flagp);
  
      if(flag_file==0)//If there's no file input,end,exit
      {
      printf("No path has been input!\n");
      flag=1;
      }
      if(argc==1)//If there's no options and paths input, end, exit
      {
      printf("Too few options!\n");
      flag=1;
      }


      if(flag==1)
      {
      printf("\n");
      usage();//Some options are wrong, so end, exit
      }
      else
      {   
          if(flagh==1)
           usage();
          for(i=0;i<(*pathi);i++)
          {
                 if(mknod(path[i],(*newmode),0)==-1)
                 {
                    if(flagp==1)
                     {
                        if(flagm==1)//If the option '-p' and '-m' existed,
                                 // change mode.
                         {
                          if(chmod(path[i],(*newmode))==1)
                        printf("Directory %s has changed the mode.\n",path[i]);
                         }
                     }  
                    else
                       {
                       printf("Directory %s failed to creat.\n",path[i]);
                       flag_fail=1;
                       }
                 }
                else 
                if(flagv==1)

                   printf("Directory %s has been successfully created!\n",\
                                   path[i]);
          }

               
      }
  




}







