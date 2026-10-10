#include "kernel/types.h"
#include "user/user.h"

__attribute__((noreturn)) void
sieve(int read_pipe[2])
{
  close(read_pipe[1]);

  int prime;

  int bytes_read = read(read_pipe[0], &prime, sizeof(prime));
  if (bytes_read < 0) {
    fprintf(2, "sieve: read failed\n");
    exit(1);
  }

  if (bytes_read == 0) {
    close(read_pipe[0]);
    exit(0);
  }

  printf("prime %d\n", prime);

  int write_pipe[2];
  if (pipe(write_pipe) < 0) {
    fprintf(2, "sieve: pipe failed\n");
    exit(1);
  }

  int pid = fork();
  if (pid < 0) {
    fprintf(2, "sieve: fork failed\n");
    exit(1);
  }

  if (pid == 0) {
    close(write_pipe[1]);
    close(read_pipe[0]);
    sieve(write_pipe);
  } else {
    close(write_pipe[0]);
    int num;
    int bytes_read;
    while ((bytes_read = read(read_pipe[0], &num, sizeof(num))) > 0) {
      if (num % prime != 0) {
        write(write_pipe[1], &num, sizeof(num));
      }
    }
    if (bytes_read < 0) {
      fprintf(2, "sieve: read failed\n");
      exit(1);
    }

    close(read_pipe[0]);
    close(write_pipe[1]);
    wait(0);
    exit(0);
  }
}

int
main(int argc, char *argv[])
{
  if (argc != 1) {
    fprintf(2, "Usage: primes\n");
    exit(1);
  }

  int p[2];
  if (pipe(p) < 0) {
    fprintf(2, "primes: pipe failed\n");
    exit(1);
  }

  int pid = fork();
  if (pid < 0) {
    fprintf(2, "primes: fork failed\n");
    exit(1);
  }

  if (pid == 0) {
    sieve(p);
  } else {
    close(p[0]);
    for (int i = 2; i <= 280; i++) {
      write(p[1], &i, sizeof(i));
    }

    close(p[1]);
    wait(0);
    exit(0);
  }
  return 0;
}
