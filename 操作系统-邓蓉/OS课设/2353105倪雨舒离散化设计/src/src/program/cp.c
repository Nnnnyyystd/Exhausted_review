#include<file.h>
#include"GetOptAndPath.h"
#include<string.h>
#include<stdio.h>
void cpfile(char *source,char *dest)
{
    //char string[50];
	char buf[512];
    int count;
    int i;
    int fds,fdd;
    fds=open(source,0777);
    fdd=creat(dest,0777);
    if(fds==-1)
    {
         printf("Wrong file \'%s\'\n",source);
         return;
    }
    if(fdd==-1)
    {
         printf("Wrong file \'%s\'\n",dest);
         return;
    }
    // while(read(fds,string,50)!=0)
    // {
         
         // write(fdd,string,strlen(string));
         // for(i=0;i<50;i++)
         // string[i]='\0';
    // }
	
	int rbytes = 0;
	int wbytes = 0;
	while ( rbytes = read(fds, buf, 512) )
	{
		if ( rbytes < 0 )
		{
			printf("Read src file error...\n");
			return;
		}
		else
		{
			wbytes = write(fdd, buf, rbytes);
			if (wbytes != rbytes)
			{
				printf("Write dst file error...\n");
				return;
			}
		}
	}
    close(fds);
    close(fdd);
}

void cpdir(char * source,char * dest,char * filename)
{
    int fd;
    int i,j,count;
    char sourcebuf[100];
    char destbuf[100];
    char answer[2];
    char item[32];
    struct st_inode inode,inodet;
    
    
    if(stat(source,&inode)==-1)
    {
         printf("Inode of \'%s\' cannot be read!\n",source);
         return;
    }
         //Get real path of the new file or dir:dest/filename
         for(i=0;i<100;i++)   //clear destbuf
         destbuf[i]='\0';    
         strcpy(destbuf,dest);
         strcat(destbuf,"/");
         strcat(destbuf,filename);
    //if source file is a normal file
    if((inode.st_mode&0x4000)==0)
    {
       //if there is no file under destinition dir with the same name as
       //the source file:
         if(stat(destbuf,&inodet)!=1) 
         {   
             //creat a file with the source's name under dest dir
               creat(destbuf,0777);
               cpfile(source,destbuf);
               return;
         }
     //if there is a file under dest dir with the same name as the source file
         else
         {
               printf("Do you want to cover the file \'%s\' ?\n",destbuf);
               gets(answer);
               if(answer[0]=='Y'||answer[0]=='y')
               {     
                     unlink(destbuf);//delete the original one 
                     creat(destbuf,0777);//and creat a new one
                     cpfile(source,destbuf);
                     return;
               }
               else
                  return;         
         }
    }
   
    //if source file is a directory
    else
    {
          fd=open(source,0111);
          if(fd==-1)
          printf("Wrong file \'%s\'!\n",source);
          //if there is no dir with the same name as the source dir,creat one
          if(stat(destbuf,&inodet)!=1)
          {
               if(mknod(destbuf,040755,0)==-1)
               {
                  printf("Creating dir \'%s\' failed!\n",destbuf);
                  return;
               }
               
          }
          count=read(fd,item,32);
          while(count!=0)
          {
                     if(((int)item[0])==0)
                     {
                           for(j=0;j<32;j++)
                           item[j]='\0';
                           count=read(fd,item,32);
                     }
                     else
                     {  
                           strcpy(sourcebuf,source);
                           strcat(sourcebuf,"/");
                           strcat(sourcebuf,item+4);
                           cpdir(sourcebuf,destbuf,item+4);
                           for(j=0;j<100;j++)
                           {
                               sourcebuf[j]='\0';
                           }     
                           for(j=0;j<32;j++)
                           item[j]='\0';
                           count=read(fd,item,32);
                     }
          }
       close(fd);          
   }
    return;
}

void GetFileName(char * string,char * filename)
{
     int i;
     int flag=0;
     char *pointer;
     for(i=strlen(string)-1;i>=0;i--)
     {
           if(string[i]=='/')
           {
               flag=1;
               break;
           }
     }
     if(flag==0)
     strcpy(filename,string);
     else
     {
          pointer=&string[i+1];
          strcpy(filename,pointer);
     }
     return;
}

void usage()
{
  printf("Usage:cp [options] filename1 filename2 ... targetfile\n");
  printf("Options:\n");
  printf("-r:copy recursively all the files under a directory\n");
  printf("--help:show usage\n");
}

void main1(int argc, char ** argv)
{
       struct st_inode inode,inodet;
       struct option options[]=
       {
          {"-r","",0,'r'},
          {"","--help",0,'h'}
       };
       int flag_tndir=0;//flag to indicate whether target is dir or file
       int flag=0;
       int flag_file=0;
       int flagr=0;
       int flagcreat=0;
       int getret;
       char optarg[10];
       char path[10][100];
       int *pathi;
       int *optind;
       int a=0;
       int b=1;
       int i=0,j=0;
       char answer[2];
       char filename[20];

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
                      break;
                      case 3:
                      printf("Certain options lacks argument.\n");
                      flag=1;
                      break;
                      case 'h':
                      flag=1;
                      break;
                      case 'r':
                      flagr=1;
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
                printf("Too few options and!\n");
                flag=1;
        }
       if((*pathi)<2)
       {
                printf("Not enough files!\n");
                flag=1;
       }
       if(flag==1)
       {
               printf("\n");
               usage();
               return;
       }

       //Find out whether target file existed
       //if target file does not exist
       
       if(stat(path[(*pathi)-1],&inodet)!=1)
       {
               if((*pathi)==2)
               {
                          if(stat(path[0],&inode)==-1)
                          {
                                  printf("Wrong file \'%s\'!\n",path[0]);
                                          return;
                          }
                          else
                          {     //if there is only 1 source file
                                //and the source file is not a dir,
                                //then creat target to be a normal file.
                                 if((inode.st_mode&0x4000)==0)
                                 {    
                                          creat(path[(*pathi)-1],0777);
                                          flagcreat=1;
                                          flag_tndir=1;
                                 }
                                //if the only source file is a dir,
                                //than creat target to be a dir
                                 else
                                 {
                                          mknod(path[(*pathi)-1],040755,0);
                                          
                                 }
                          }
               }
               //if there are more than one source file,
               //then creat the target file to be a dir.  
               else
               {
                          mknod(path[(*pathi)-1],040755,0);
               }
       }
       //if target file exists
       else
       {       
               //if target file is a normal file
               if((inodet.st_mode&0x4000)==0) 
               {
                      
                     if((*pathi)==2)
                     {                  
                          if(stat(path[0],&inode)==-1)
                          {
                                  printf("Wrong file \'%s\'!\n",path[0]);
                                          return;
                          }
                          else
                          {     //if there is only 1 source file
                                //and the source file is not a dir,
                                //then it is ok
                                 if((inode.st_mode&0x4000)==0)
                                 {    
                                          flag_tndir=1;
                                 }
                                //if the only source file is a dir,
                                //then error
                                 else
                                 {
                                        printf("Source file is a directory,and destinition file is a normal file,it is not allowed!\n");
                                        return;
                                          
                                 }
                          }
                     }
                      //if there are more than one source file,
                      //then error  
                     else
                     {
                            printf("There are more than 1 source files,and destinition file is a normal file,it is not allowed!\n");
                              return;
                     }
               }
       } 
       
       //------------------------------------------------------------ 
       //if the target file is a normal file,which means there's only
       //one normal source file
       if(flag_tndir==1)
       {    
             
           
                 if(flagcreat!=1)
                 {
                     printf("Do you want to cover the file \'%s\'?\n",path[1]);
                         gets(answer);
                         if(answer[0]=='y' || answer[1]=='Y')
                         {
                                 
                              unlink(path[1]);//delete the original one 
                              creat(path[1],0777);//and creat a new one
                              cpfile(path[0],path[1]);
                         }
                         else
                              return;
                       
                }
                else
				{
					printf("Debug Info: Copy one normal file!\n");
                      cpfile(path[0],path[1]);     
				}
       }      
       //if there are more than one source files, or destinition is a dir,
       //or the only source file is a dir
       else
       { 

                      for(i=0;i<(*pathi-1);i++)
                     {      
                            for(j=0;j<20;j++)
                                  filename[j]='\0';
                            GetFileName(path[i],filename);
                           if(stat(path[i],&inode)==-1)
                           {
                                printf("Wrong file \'%s\'!",path[i]);
                           }    
                           else
                           {    //if current file is a directory 
                                if((inode.st_mode&0x4000)!=0)
                                {
                              //if option '-r' is opened
                                    if(flagr==1)
                                    { 
                                        
                                       cpdir(path[i],path[(*pathi)-1],filename);
                                    } 
                              //otherwise don't copy
                                         else
                               printf("Omitting directory \'%s\'\n",path[i]);
                                }
                                else
                                    cpdir(path[i],path[(*pathi)-1],filename);
                           }
                     }
       }  
}
