#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"

int
main(int argc, char *argv[])
{
  // your code here.  you should write the secret to fd 2 using write
  // (e.g., write(2, secret, 8)
  char secret[8];
  char *end = sbrk(PGSIZE*32);

  // for(int i = 0; i<32; i++){
  //   memcpy(secret, end + 32 + i*PGSIZE, 8);
  //   printf("page %d: %s\n", i, secret);
  // }

  end = end + 16 * PGSIZE;
  memcpy(secret, end + 32, 8);
  // printf("secret: %s\n", secret);
  write(2, secret, 8);
  exit(1);
}
