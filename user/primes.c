#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

//pipes are unidirectional!!!!!
#define READ_END 0
#define WRITE_END 1
void print(int x){
  printf("prime %d\n",x);
}
void calc(int p[]){
  int child_p[2];
  int base,x;
  close(p[WRITE_END]);
  if(read(p[READ_END],&base,4) != 0){
    pipe(child_p);
    print(base);
    if(fork()!=0){
      while(read(p[READ_END],&x,4) != 0){
        if(x%base != 0){
          write(child_p[WRITE_END],&x,4);
        }
      }
      close(child_p[WRITE_END]);
      close(p[READ_END]);//arcane
      wait(0);
    }
    else{
      calc(child_p);
    }
  }

  exit(0);
}

int
main(int argc, char *argv[]){
  int i,p[2],pid;
  if(argc > 1){
    fprintf(2,"error");
    exit(1);
  }
  pipe(p);
  if((pid = fork()) != 0){
    close(p[READ_END]);
    for(i=2;i<=35;i++){
      write(p[WRITE_END],&i,4);
    }
    close(p[WRITE_END]);//arcane
    wait(0);
  } else{
    calc(p);
  }
  exit(0);
}

