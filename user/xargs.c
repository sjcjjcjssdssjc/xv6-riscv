
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"
#define DEFAULT_INDEX 1
#define EXTENDED_INDEX 3

char *
strcat(char *dest,const char *src){
	char *p;
	p = dest;
	while(*dest){
		dest++;
  }
	while(*src) {
		*dest++ = *src++;
	}
	*dest = '\0';
	return p;
}

int
main(int argc, char *argv[])
{
  //exec("/echo",...);
  char c[1];
  char buf[512];
  char *p = buf;
  char *argv_child[MAXARG]; //remember the last is NULL.
  int pid;
  if(argc <= 1){
    fprintf(2,"error: argc is less than 2\n");
    exit(0);
  }
  else if(argc >= 3 && strcmp(argv[1],"-n") == 0 
  && strcmp(argv[2],"1") == 0){
    //printf("3\n");
    while((read(0,c,1)) != 0){ //standard input
      p = buf;
      if(c[0] != '\\'&& c[0] != 'n' && c[0] != '\"' && c[0] != '\n'){
        *p++ = c[0];
        while((read(0,c,1)) != 0 && c[0] != '\\'&& c[0] != 'n'
          && c[0] != '\"' && c[0] != '\n'){
          *p++ = c[0];
        }
      } else{
        continue;
      }

      *p = '\0';
     // printf("%s debug\n",buf);
      for(int i=EXTENDED_INDEX; i<argc; i++){
        argv_child[i-EXTENDED_INDEX]      = argv[i];
      }
      argv_child[argc-EXTENDED_INDEX]   = buf;
      argv_child[argc-EXTENDED_INDEX+1] = 0;
      if((pid = fork()) == 0){//child
        exec(strcat("/",argv[EXTENDED_INDEX]),argv_child);
      } else{
        wait(0);
      }
    }
  }
  else {
    //printf("1\n");
    while((read(0,c,1)) != 0){
      p = buf;
      //printf("debug %s\n",c);
      if(c[0] != '\n'){
        *p++ = c[0];
        while((read(0,c,1)) != 0 && c[0] != '\n'){
          *p++ = c[0];
        }
      } else{
        continue;
      }
      *p = '\0';
      for(int i=DEFAULT_INDEX; i<argc; i++){
        argv_child[i-DEFAULT_INDEX]      = argv[i];
      }
      argv_child[argc-DEFAULT_INDEX]   = buf;
      argv_child[argc-DEFAULT_INDEX+1] = 0;
      //printf("debug: %s\n",buf);
      if((pid = fork()) == 0){//child
        //printf("%s\n",argv[DEFAULT_INDEX]);
        exec(strcat("/",argv[DEFAULT_INDEX]),argv_child);
      } else{
        wait(0);
      }
    }
    
  }
  
  exit(0);
}
