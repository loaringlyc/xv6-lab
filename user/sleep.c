#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

void 
main(int argc, char *argv[])
{
  int t; // time to sleep

  if (argc != 2) {
    fprintf(2, "usage: sleep seconds...\n");
    exit(1);
  }
  t = atoi(argv[1]);

  printf("sleep for %d * 0.1 seconds\n", t);
  sleep(t);

  exit(0);
}
