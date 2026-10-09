#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define MAXLINE 512


int
readline(int fd, char *buf, int max)
{
  int n = 0, got = 0;
  char c;

  while (read(fd, &c, 1) == 1) {
    got = 1;
    if (c == '\n')
      break;
    if (n < max - 1)
      buf[n++] = c;
  }
  buf[n] = 0;
  return got;
}

int
main(int argc, char *argv[])
{
  char *f[2];
  int nf = 0, quiet = 0;
  int fd1, fd2;
  char l1[MAXLINE], l2[MAXLINE];
  int line = 0;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-q") == 0)
      quiet = 1;
    else if (nf < 2)
      f[nf++] = argv[i];
    else
      nf++;
  }
  if (nf != 2) {
    fprintf(2, "usage: diff file1 file2 [-q]\n");
    exit(1);
  }

  if ((fd1 = open(f[0], O_RDONLY)) < 0) {
    fprintf(2, "diff: cannot open %s\n", f[0]);
    exit(1);
  }
  if ((fd2 = open(f[1], O_RDONLY)) < 0) {
    fprintf(2, "diff: cannot open %s\n", f[1]);
    close(fd1);
    exit(1);
  }

  while (1) {
    int has1 = readline(fd1, l1, MAXLINE);
    int has2 = readline(fd2, l2, MAXLINE);
    line++;

    if (!has1 && !has2)
      break;
    if (has1 && has2 && strcmp(l1, l2) == 0)
      continue;

    if (quiet) {
      printf("diff: files differ\n");
      break;
    }
    printf("%s:%d: < %s\n", f[0], line, has1 ? l1 : "EOF");
    printf("%s:%d: > %s\n", f[1], line, has2 ? l2 : "EOF");
  }

  close(fd1);
  close(fd2);
  exit(0);
}
