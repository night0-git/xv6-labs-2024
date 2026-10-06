#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if (argc != 1) {
    fprintf(2, "Usage: pingpong\n");
    exit(1);
  }

  int ptoc_pipe[2] = {0};
  if (pipe(ptoc_pipe) < 0) {
    fprintf(2, "pingpong: pipe failed\n");
    exit(1);
  }

  int ctop_pipe[2] = {0};
  if (pipe(ctop_pipe) < 0) {
    fprintf(2, "pingpong: pipe failed\n");
    exit(1);
  }

  // Create a child process.
  int child_pid = fork();
  if (child_pid < 0) {
    fprintf(2, "pingpong: fork failed\n");
    exit(1);
  }

  if (child_pid == 0) { // child process
    if (close(ptoc_pipe[1]) < 0) {
      fprintf(2, "pingpong(parent): close failed\n");
      exit(1);
    }
    if (close(ctop_pipe[0]) < 0) {
      fprintf(2, "pingpong(parent): close failed\n");
      exit(1);
    }

    char received;
    // Receive the byte from parent.
    int bytes_read = read(ptoc_pipe[0], &received, 1);
    if (bytes_read != 1) {
      fprintf(2, "pingpong(child): read failed\n");
      exit(1);
    }
    printf("%d: received ping\n", getpid());
    if (close(ptoc_pipe[0]) < 0) {
      fprintf(2, "pingpong(child): close failed\n");
      exit(1);
    }

    // Send the byte back to parent.
    if (write(ctop_pipe[1], &received, 1) != 1) {
      fprintf(2, "pingpong(child): write failed\n");
      exit(1);
    }
    if (close(ctop_pipe[1]) < 0) {
      fprintf(2, "pingpong(child): close failed\n");
      exit(1);
    }

    exit(0);
  } else { // parent process
    if (close(ptoc_pipe[0]) < 0) {
      fprintf(2, "pingpong(parent): close failed\n");
      exit(1);
    }
    if (close(ctop_pipe[1]) < 0) {
      fprintf(2, "pingpong(parent): close failed\n");
      exit(1);
    }

    // The byte to be sent back and forth between parent and child.
    char buf = 'b';

    // Send the byte to child.
    if (write(ptoc_pipe[1], &buf, 1) != 1) {
      fprintf(2, "pingpong(parent): write failed\n");
      exit(1);
    }
    if (close(ptoc_pipe[1]) < 0) {
      fprintf(2, "pingpong(parent): close failed\n");
      exit(1);
    }

    char received;
    // Receive the byte from child.
    int bytes_read = read(ctop_pipe[0], &received, 1);
    if (bytes_read != 1) {
      fprintf(2, "pingpong(parent): read failed\n");
      exit(1);
    }
    printf("%d: received pong\n", getpid());
    if (close(ctop_pipe[0]) < 0) {
      fprintf(2, "pingpong(parent): close failed\n");
    }

    exit(0);
  }
}
