#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

#define READ_END 0
#define WRITE_END 1

char*
getfilename(char *path)
{
  char *p;
  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;
  return p;
}

void 
find(char *path,char *ans_fname){

  char buf[512],*p;
  int fd;
  struct dirent de;
  struct stat st;
  
  //printf("debug: %s\n",path);
  if((fd = open(path, 0)) < 0){// second is flags (O_RDONLY)
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    // The fstat () function shall obtain information about 
    // an open file associated with the file descriptor fildes, 
    // and shall write it to the area pointed to by buf.
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }
  
  switch(st.type){
  case T_FILE:
    if(strcmp(getfilename(path), ans_fname) == 0) {
      printf("%s\n",path);
    }
    break;

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("find: path too long\n");
      break;
    }
    strcpy(buf, path);//path->buf
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){//fd->de
      if(de.inum == 0)
        continue;
      if(strcmp(de.name,".") == 0 || strcmp(de.name,"..") == 0){
        continue;
      }
      memmove(p, de.name, DIRSIZ);//de.name->p
      p[DIRSIZ] = 0;
      if(stat(buf, &st) < 0){//fstat( fd(path) )?
        printf("find: cannot stat %s\n", buf);
        continue;
      }
      //printf("%s",buf);
      find(buf,ans_fname);
    }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[]){
  if(argc < 3){
    fprintf(2,"error");
    exit(1);
  }
  char *path = argv[1];
  int i;
  for(i=2; i<argc; i++) {
    char *fname = argv[i];
    find(path,fname);
  }
  exit(0);
}
