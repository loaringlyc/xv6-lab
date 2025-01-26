#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"

int
find(char *path, char *name)
{
  int fd; 
  struct dirent de;
  struct stat st;
  char buf[512];
  char *p;

  if((fd = open(path, O_RDONLY)) < 0){
    fprintf(2, "find: cannot open %s\n", path);
    exit(1);
  }
  // printf("path: %s\n", path);
  // printf("fd: %d\n", fd);

  if(fstat(fd, &st) < 0){
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    exit(1);
  }

  switch (st.type){
  case T_FILE:
    for(p=path+strlen(path); p>=path && *p != '/'; p--) 
      ;
    p++; // 得到名字起始位置
    // printf("p: %s\n", p);
    if(strcmp(p, name) == 0){
      printf("%s\n", path);
    }
    break;
  case T_DEVICE:
  default:
    break;
  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("find: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf + strlen(buf); //未考虑长度超出
    *p++ = '/';        // 先赋值再加一
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0) // 未使用或已删除
        continue;
      memmove(p, de.name, DIRSIZ);
      if(strcmp(de.name, ".") != 0 && strcmp(de.name, "..") != 0){
        // printf("buf: %s\n", buf);
        find(buf, name); // 碰到.和..不递归，否则死循环或者find错误
      }
    }
    break;
  }
  close(fd);
  return 0; //注意，这里不能用exit返回，这个递归和primes不同，并不是fork出新的进程
}

int
main(int argc, char *argv[])
{
  char *path, *name;

  if(argc != 3){
    fprintf(2, "usage: find path file\n");
    exit(1);
  }
  path = argv[1];
  name = argv[2];

  find(path, name);

  exit(0);
}
