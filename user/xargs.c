#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/stat.h"
#include "user/user.h"

char whitespace[] = " \t\r\n\v";

int
readline(char *nargv[MAXARG], int currc)
{
  int n = 0;
  char *ps;
  char buf[512];

  memset(buf, 0, 512*sizeof(char));

  while(read(0, buf+n, 1) == 1 && buf[n] != '\n'){
    // printf("*buf: %s\n", (buf+n));
    if(n == 511){
      fprintf(2, "xargs: argument too long\n");
      exit(1);
    }
    n++;
  }
  buf[n] = 0;

  ps = buf;
  while(strchr(whitespace, *ps)) //跳过最开始的空格
    ps++;
  if (*ps == 0)
    return 0;

  while(*ps != 0){ // 如果碰到0，说明到达buf的最后
    nargv[currc] = malloc(strlen(ps) + 1); // currc = 当前arg个数
    strcpy(nargv[currc], ps); // 一行为一个参数
    // nargv[currc] = ps; 
    currc++;
    while(!strchr(whitespace, *ps)) //跳过第一个参数
      ps++;

    while(strchr(whitespace, *ps)) //跳过末尾的空格
      ps++;
  }
  return currc;
}

int
main(int argc, char *argv[])
{
  int n, pid; 
  char *nargv[MAXARG];

  if(argc < 2){
    fprintf(2, "usage: xargs ...\n");
    exit(1);
  }

  for(int i = 1; i < argc; i++){ // nargv[0 ~ argc-2] 共 argc-1 个
    // nargv[i-1] = argv[i];
    nargv[i - 1] = malloc(strlen(argv[i]) + 1); // 给nargv分配内存
    strcpy(nargv[i-1], argv[i]); 
  }
  while((n = readline(nargv, argc - 1)) != 0){
    pid = fork();
    nargv[n] = 0;
    if(pid == 0){
      // for(int i = 0; i < n; i++)
      //   printf("%s\n", nargv[i]);
      exec(argv[1], nargv);
      fprintf(2, "xargs: exec failed\n");
			exit(1);
    }
    wait(0);
  }

  exit(0);
}
