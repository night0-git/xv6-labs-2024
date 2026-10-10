#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if (argc != 1) {
    fprintf(2, "Usage: pingpong\n");
    exit(1);
  }

  int ptoc_pipe[2] = {0};
  int ctop_pipe[2] = {0};

  if (pipe(ptoc_pipe) < 0) {
    fprintf(2, "pingpong: pipe failed\n");
    exit(1);
  }
  if (pipe(ctop_pipe) < 0) {
    fprintf(2, "pingpong: pipe failed\n");
    exit(1);
  }

  int pid = fork();
  if (pid < 0) {
    fprintf(2, "pingpong: fork failed\n");
    exit(1);
  }

  if (pid == 0) {
    close(ptoc_pipe[1]);
    close(ctop_pipe[0]);

    char received;
    // Receive the byte from parent.
    int bytes_read = read(ptoc_pipe[0], &received, 1);
    if (bytes_read != 1) {
      fprintf(2, "pingpong(child): read failed\n");
      exit(1);
    }
    printf("%d: received ping\n", getpid());
    close(ptoc_pipe[0]);

    // Send the byte back to parent.
    if (write(ctop_pipe[1], &received, 1) != 1) {
      fprintf(2, "pingpong(child): write failed\n");
      exit(1);
    }
    close(ctop_pipe[1]);

    exit(0);
  } else {
    close(ptoc_pipe[0]);
    close(ctop_pipe[1]);

    // The byte to be sent back and forth between parent and child.
    char buf = 'b';

    // Send the byte to child.
    if (write(ptoc_pipe[1], &buf, 1) != 1) {
      fprintf(2, "pingpong(parent): write failed\n");
      exit(1);
    }
    close(ptoc_pipe[1]);

    char received;
    // Receive the byte from child.
    int bytes_read = read(ctop_pipe[0], &received, 1);
    if (bytes_read != 1) {
      fprintf(2, "pingpong(parent): read failed\n");
      exit(1);
    }
    printf("%d: received pong\n", getpid());
    close(ctop_pipe[0]);

    wait(0);
    exit(0);
  }
}
