#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

void primes(int []) __attribute__((noreturn)); 

void 
primes(int pipefd[])
{
  int newfd[2]; // newfd 在这里申明，就在这里close掉
  int buf[1], tmp[1];
  int pid;

  if (pipe(newfd) == -1) {
    fprintf(2, "Creating pipe failed\n");
    exit(1);
  }

  if (read(pipefd[0], tmp, 4) <= 0){ // 从前一个pipe读取第一个数
    exit(0);
  }
  printf("prime %d\n", tmp[0]); //必为质数 

  pid = fork(); // 复制了一个 关闭了写的fd[2]
  if(pid == 0) {
    close(pipefd[0]); // 关闭上一个fd的读取
    close(newfd[1]);  // 在primes中需要读取newfd，但是不需要写
    primes(newfd); // 在内部会关闭读管道
  }else {
    close(newfd[0]);

    while(1) {
      int n = read(pipefd[0], buf, 4);
      if (n <= 0){
        break;
      }
      if (buf[0] % tmp[0] != 0){ 
        write(newfd[1], buf, 4);
      }
    }
    close(pipefd[0]); // 关闭上一个fd的读取
    close(newfd[1]);  // close write 因为子进程不需要写

    wait(0);
  }

  exit(0);
}

int
main(int argc, char *argv[])
{
  int pipefd[2];
  int pid;
  if (pipe(pipefd) == -1) {
    fprintf(2, "Creating pipe failed\n");
    exit(1);
  }

  pid = fork(); // 复制了pipefd

  if(pid == 0){
    close(pipefd[1]);
    primes(pipefd);
    // close(pipefd[0]); // 在primes内部已经close了
  } else{
    for (int i = 2; i <= 280; i++){
      int n = i;
      // printf("%d", n);
      write(pipefd[1], &n, 4);
    }
    close(pipefd[1]);
    wait(0);
    close(pipefd[0]);
  }

  exit(0);
}

// primes_backup.c 
// 可以运行，但是会阻塞在写入pipe，因为没有人来读取并且长度不够长
// #include "kernel/types.h"
// #include "kernel/fcntl.h"
// #include "user/user.h"

// void primes(int []) __attribute__((noreturn)); 

// void 
// primes(int pipefd[])
// {
//   int newfd[2]; // newfd 在这里申明，就在这里close掉
//   int buf[1], tmp[1];
//   int pid;

//   if (pipe(newfd) == -1) {
//     fprintf(2, "Creating pipe failed\n");
//     exit(1);
//   }

//   if (read(pipefd[0], tmp, 4) <= 0){ // 从前一个pipe读取第一个数
//     exit(0);
//   }
//   printf("prime %d\n", tmp[0]); //必为质数 

//   while(1) {
//     int n = read(pipefd[0], buf, 4);
//     if (n <= 0){
//       break;
//     }
//     if (buf[0] % tmp[0] != 0){ 
//       write(newfd[1], buf, 4);
//     }
//   }
//   close(pipefd[0]); // 关闭上一个fd的读取
//   close(newfd[1]);  // close write 因为子进程不需要写

//   pid = fork(); // 复制了一个 关闭了写的fd[2]
//   if(pid == 0) {
//     primes(newfd); //在内部会关闭读管道
//   }else {
//     close(newfd[0]);
//     wait(0);
//   }

//   exit(0);
// }

// int
// main(int argc, char *argv[])
// {
//   int pipefd[2];
//   int pid;
//   if (pipe(pipefd) == -1) {
//     fprintf(2, "Creating pipe failed\n");
//     exit(1);
//   }
//   for (int i = 2; i <= 35; i++){ //如果大一点，就会发生阻塞（不read就write不下了），所以需要将这部分写在fork里面
//     int n = i;
//     // printf("%d", n);
//     write(pipefd[1], &n, 4);
//   }
//   close(pipefd[1]);

//   pid = fork(); // 复制了一个关闭写的fd
//   if(pid == 0){
//     primes(pipefd);
//     // close(pipefd[0]); // 在primes内部已经close了
//   } else{
//     wait(0);
//     close(pipefd[0]);
//   }

//   exit(0);
// }
