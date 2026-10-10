#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define BUF_SIZE 1024

int
main(int argc, char *argv[])
{
  int fd_src, fd_dst;
  char buf[BUF_SIZE];

  if (argc != 3) {
    fprintf(2, "Usage: cp [SOURCE] [DEST]\n");
    exit(1);
  }

  if ((fd_src = open(argv[1], O_RDONLY)) < 0) {
    fprintf(2, "cp: cannot open %s\n", argv[1]);
    exit(1);
  }

  if ((fd_dst = open(argv[2], O_WRONLY | O_CREATE | O_TRUNC)) < 0) {
    fprintf(2, "cp: cannot open %s\n", argv[2]);
    close(fd_src);
    exit(1);
  }

  int n;
  while ((n = read(fd_src, buf, sizeof(buf))) > 0) {
    if (write(fd_dst, buf, n) != n) {
      fprintf(2, "cp: write error\n");
      break;
    }
  }
  if (n < 0) {
    fprintf(2, "cp: read error\n");
    exit(1);
  }

  close(fd_src);
  close(fd_dst);

  exit(0);
}
