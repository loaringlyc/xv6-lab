#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int 
main(int argc, char *argv[])
{
  int pipefd_foo[2];
  int pipefd_bar[2];
  int pid; 
  char msg = 'a';
  char buf_foo[100], buf_bar[100];

  if (pipe(pipefd_foo) == -1) {
    fprintf(2, "Creating pipe failed\n");
    exit(1);
  }

  if (pipe(pipefd_bar) == -1) {
    fprintf(2, "Creating pipe failed\n");
    exit(1);
  }

  pid = fork();
  if (pid < 0) {
    fprintf(2, "Forking failed\n");
    exit(1);
  }

  if (pid == 0){
    close(pipefd_foo[1]);
    read(pipefd_foo[0], buf_bar, 1);
    printf("%d: received ping\n", getpid());
    close(pipefd_foo[0]);
  } else {
    close(pipefd_foo[0]); // close read
    write(pipefd_foo[1], &msg, 1);
    close(pipefd_foo[1]); // close write
    wait(0);
    close(pipefd_bar[1]); 
    read(pipefd_bar[0], buf_foo, 1);
    printf("%d: received pong\n", getpid());
  }
  exit(0);
}
