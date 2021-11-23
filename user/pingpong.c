#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#define READ_END 0
#define WRITE_END 1

int
main(int argc, char *argv[]){
  if(argc > 1){
    fprintf(2,"error");
    exit(1);
  }
  int p1[2];//read_port(0) and write_port(1)
  int p2[2];

  pipe(p1);
  pipe(p2);
  
  int pid;
  char buf[1];
  

  if(fork() == 0){
    // child reads from p2 and writes p1
    close(p2[WRITE_END]);
    read(p2[READ_END],buf,1);
    pid=getpid();
    fprintf(1,"%d: received ping\n",pid);
    write(p1[WRITE_END],buf,1);
  } else {
    // father reads from p1 and writes p2
    close(p2[READ_END]);
    write(p2[WRITE_END],buf,1);
    read(p1[READ_END],buf,1);
    pid=getpid();
    fprintf(1,"%d: received pong\n",pid);
  }
  exit(0);
}
