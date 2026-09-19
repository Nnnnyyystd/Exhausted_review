#include<stdio.h>
#include<string.h>
struct option
{
char *shortopt;
char *longopt;
int has_arg;
int val;
};

int GetOptAndPath(argc,argv,optind,pathi,optarg,path,options,optionsi)
    int argc;
    char **argv;
    int *optind;
    int *pathi;
    char *optarg;
    char path[10][100];
    struct option *options;
    int optionsi;
{

int flag=0;
int i;

if((*optind)<argc)
{
   if(argv[(*optind)][0]=='-')
   {
        
        for(i=0;i<optionsi;i++)
        {
           if(!strcmp(options[i].shortopt,argv[(*optind)])||\
              !strcmp(options[i].longopt,argv[(*optind)]))
           {flag=1;break;} 
        }    
  
   
    
       if(flag==0)
       {
      
            (*optind)++; 
            return 2;//Indicate that the option is wrong
       }
       else
       {
           if(options[i].has_arg==1)
           {
              (*optind)++;
              if((*optind)<argc)
              strcpy(optarg,argv[(*optind)]);
              else
              return 3;//Indicate that there're no argument for certain option   
           }
           (*optind)++;
           return options[i].val;
       }
    
   }
 else
   { 
    strcpy(path[*pathi],argv[*optind]);
    (*optind)++;
    (*pathi)++;
    
     return 1;
   }
}
else

return 0;

}



