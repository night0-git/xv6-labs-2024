#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/ptree.h"

#define MAX_PROCS 64

// Print process info with indentation based on depth (2 spaces per depth level)
static void
print_process(struct ptreeinfo *p, int depth)
{
  int d;

  for (d = 0; d < depth; d++) {
    printf("  ");
  }
  printf("%d %s state=%d mem=%ld\n", p->pid, p->name, p->state, p->memsize);
}

// Recursively find and print children whose ppid matches parent_pid
static void
print_tree(struct ptreeinfo *buf, int n, int parent_pid, int depth)
{
  int i;

  for (i = 0; i < n; i++) {
    if (buf[i].ppid == parent_pid && buf[i].pid != parent_pid) {
      print_process(&buf[i], depth);
      print_tree(buf, n, buf[i].pid, depth + 1);
    }
  }
}

int
main(int argc, char *argv[])
{
  struct ptreeinfo buf[MAX_PROCS];
  int n = ptree(buf, MAX_PROCS);
  int i, j;

  if (n < 0) {
    fprintf(2, "pstree: ptree failed\n");
    exit(1);
  }

  // 1. Sort array by pid ascending so siblings print in order of pid
  for (i = 0; i < n - 1; i++) {
    for (j = 0; j < n - i - 1; j++) {
      if (buf[j].pid > buf[j + 1].pid) {
        struct ptreeinfo tmp = buf[j];
        buf[j] = buf[j + 1];
        buf[j + 1] = tmp;
      }
    }
  }

  // 2. Identify root processes (pid == 1, ppid == 0, or parent not in list)
  for (i = 0; i < n; i++) {
    int is_root = 0;
    if (buf[i].pid == 1 || buf[i].ppid == 0) {
      is_root = 1;
    } else {
      int parent_exists = 0;
      for (j = 0; j < n; j++) {
        if (buf[i].ppid == buf[j].pid) {
          parent_exists = 1;
          break;
        }
      }
      if (!parent_exists) {
        is_root = 1;
      }
    }

    if (is_root) {
      print_process(&buf[i], 0);
      print_tree(buf, n, buf[i].pid, 1);
    }
  }

  exit(0);
}
